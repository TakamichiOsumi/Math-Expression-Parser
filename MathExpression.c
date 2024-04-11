#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include "ExportedParser.h"
#include "MexprEnums.h"

/*
 * The grammer to parse mathmatical expression:
 *
 * E -> E + T | E - T | T
 * T -> T * F | T / F | F
 * F -> INTEGER | DOUBLE | VAR | '(' E ')'
 *
 * But the first two rules contain left recursion,
 * which can't be written as it is. Thefore,
 * redefine them with some technique to avoid
 * the coding problem as below.
 *
 * 1. E  -> T E'
 * 2. E' -> + T E' | - T E' | $
 * 3. T  -> F T'
 * 4. T' -> * F T' | / F T' | $
 * 5. F  -> INTEGER | DOUBLE | VAR | ( E )
 * where $ means empty.
 *
 */
static bool E_dash(void);
static bool T(void);
static bool T_dash(void);
static bool F(void);

#define DEBUG_RESTORE(CKP) { printf("Restore : %s will restore from line = %d\n", \
				    __FUNCTION__, __LINE__); RESTORE_CHECKPOINT(CKP); }
#define DEBUG_PRINT { printf("Break : %s will break from line = %d\n", \
			     __FUNCTION__, __LINE__); }
#define RETURN_FALSE { printf("Return : %s will return false from line = %d\n", \
			      __FUNCTION__, __LINE__); return false; }

/*
 * Any rule which contains dollar sign, the function for the non-terminal symbol
 * must never return error. It should always return success. The way to implement
 * dollar sign is simply return success.
 */

/* 1. E  -> T E' */
static bool
E(void){
    int CKP;

    CHECKPOINT(CKP);

    if (T() == false){
	DEBUG_RESTORE(CKP);
	RETURN_FALSE;
    }

    if (E_dash() == false){
	DEBUG_RESTORE(CKP);
	RETURN_FALSE;
    }

    return true;
}

/* 2. E' -> + T E' | - T E' | $ */
static bool
E_dash(void){
    int token_code, CKP;

    CHECKPOINT(CKP);

    /* E' -> + T E' */
    do {
	if ((token_code = cyylex()) != PLUS){
	    DEBUG_PRINT;
	    break;
	}

	if (T() == false){
	    DEBUG_PRINT;
	    break;
	}

	if (E_dash() == false){
	    DEBUG_PRINT;
	    break;
	}

	return true;

    } while(0);

    DEBUG_RESTORE(CKP);

    /* E' -> - T E' */
    do {
	if ((token_code = cyylex()) != MINUS)
	    break;

	if (T() == false){
	    DEBUG_PRINT;
	    break;
	}

	if (E_dash() == false){
	    DEBUG_PRINT;
	    break;
	}

	return true;

    } while(0);

    DEBUG_RESTORE(CKP);

    return true;
}

/* 3. T  -> F T' */
static bool
T(void){
    int CKP;

    CHECKPOINT(CKP);

    if (F() == false){
	DEBUG_RESTORE(CKP);
	RETURN_FALSE;
    }

    if (T_dash() == false){
	DEBUG_RESTORE(CKP);
	RETURN_FALSE;
    }

    return true;
}

/* 4. T' -> * F T' | / F T' | $ */
static bool
T_dash(void){
    int token_code, CKP;

    CHECKPOINT(CKP);

    /* T' -> * F T' */
    do {
	if ((token_code = cyylex()) != MULTIPLY)
	    break;

	if (F() == false){
	    DEBUG_PRINT;
	    break;
	}

	if (T_dash() == false){
	    DEBUG_PRINT;
	    break;
	}

	return true;

    } while(0);

    DEBUG_RESTORE(CKP);

    /* T' -> / F T' */
    do {
	if ((token_code = cyylex()) != DIVIDE)
	    break;

	if (F() == false){
	    DEBUG_PRINT;
	    break;
	}

	if (T_dash() == false){
	    DEBUG_PRINT;
	    break;
	}

	return true;

    } while(0);

    DEBUG_RESTORE(CKP);

    /* T' -> $ */

    return true;
}

/* 5. F -> INTEGER | DOUBLE | VAR | ( E ) */
static bool
F(void){
    int token_code, CKP;

    CHECKPOINT(CKP);

    /* F -> '(' E ')' */
    do {
	if ((token_code = cyylex()) != BRACKET_START){
	    DEBUG_PRINT;
	    break;
	}

	if (E() == false){
	    RETURN_FALSE;
	}

	if ((token_code = cyylex()) != BRACKET_END){
	    DEBUG_PRINT;
	    break;
	}

	return true;

    } while(0);

    DEBUG_RESTORE(CKP);

    /* F -> INTEGER | DOUBLE | VAR */
    do {
	token_code = cyylex();
	switch(token_code){
	    case INT:
	    case DOUBLE:
	    case VARIABLE:
		return true;
	    default:
		RETURN_FALSE;
	}
    } while(0);

    DEBUG_RESTORE(CKP);

    RETURN_FALSE;
}

bool
start_mathexpr_parse(){
    bool parse_result;
    int token_code;

    parse_result = E();

    if ((token_code = cyylex()) != PARSER_EOF){
	printf("* Rejected : the parse has left string after the syntax E\n");
	return false;
    }else{
	if (!parse_result){
	    printf("* Rejected\n");
	    return false;
	}else{
	    printf("* Accepcted\n");
	    return true;
	}
    }
}
