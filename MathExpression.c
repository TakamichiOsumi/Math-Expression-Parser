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
 * But, the first two rules contain left recursion,
 * which can't be written as it is. Therefore,
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
 * Also, define (in)equality operators as grammer:
 *
 * Q    -> E INEQ E
 * INEQ -> = | != | < | > | <= | >=
 *
 */
static bool E_dash(void);
static bool T(void);
static bool T_dash(void);
static bool F(void);
static bool INEQ(void);

/*
 * With regard to any rule which contains dollar sign, the function for
 * the non-terminal symbol must never return error. It should always
 * return success. The way to implement dollar sign is simply return success.
 */

/* 1. E  -> T E' */
static bool
E(void){
    int CKP;

    CHECKPOINT(CKP);

    if (T() == false){
	RESTORE_CHECKPOINT(CKP);
	return false;
    }

    if (E_dash() == false){
	RESTORE_CHECKPOINT(CKP);
	return false;
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
	if ((token_code = cyylex()) != PLUS)
	    break;

	if (T() == false)
	    break;

	if (E_dash() == false)
	    break;

	return true;

    } while(0);

    RESTORE_CHECKPOINT(CKP);

    /* E' -> - T E' */
    do {
	if ((token_code = cyylex()) != MINUS)
	    break;

	if (T() == false)
	    break;

	if (E_dash() == false)
	    break;

	return true;

    } while(0);

    RESTORE_CHECKPOINT(CKP);

    return true;
}

/* 3. T  -> F T' */
static bool
T(void){
    int CKP;

    CHECKPOINT(CKP);

    if (F() == false){
	RESTORE_CHECKPOINT(CKP);
	return false;
    }

    if (T_dash() == false){
	RESTORE_CHECKPOINT(CKP);
	return false;
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

	if (F() == false)
	    break;

	if (T_dash() == false)
	    break;

	return true;

    } while(0);

    RESTORE_CHECKPOINT(CKP);

    /* T' -> / F T' */
    do {
	if ((token_code = cyylex()) != DIVIDE)
	    break;

	if (F() == false)
	    break;

	if (T_dash() == false)
	    break;

	return true;

    } while(0);

    RESTORE_CHECKPOINT(CKP);

    /* T' -> $ */

    return true;
}

/* 5. F -> INTEGER | DOUBLE | VAR | '(' E ')' */
static bool
F(void){
    int token_code, CKP;

    CHECKPOINT(CKP);

    /* F -> '(' E ')' */
    do {
	if ((token_code = cyylex()) != BRACKET_START)
	    break;

	if (E() == false)
	    return false;

	if ((token_code = cyylex()) != BRACKET_END)
	    break;

	return true;

    } while(0);

    RESTORE_CHECKPOINT(CKP);

    /* F -> INTEGER | DOUBLE | VAR */
    do {
	token_code = cyylex();
	switch(token_code){
	    case INT:
	    case DOUBLE:
	    case VARIABLE:
		return true;
	    default:
		return false;
	}
    } while(0);

    RESTORE_CHECKPOINT(CKP);

    return false;
}

bool
Q(void){
    int CKP;

    CHECKPOINT(CKP);

    if (E() == false){
	RESTORE_CHECKPOINT(CKP);
	return false;
    }

    if (INEQ() == false){
	RESTORE_CHECKPOINT(CKP);
	return false;
    }

    if (E() == false){
	RESTORE_CHECKPOINT(CKP);
	return false;
    }

    return true;
}

static bool
INEQ(void){
    int token_code, CKP;

    CHECKPOINT(CKP);

    token_code = cyylex();
    switch(token_code){
	case NEQ:
	case GREATER_THAN_OR_EQUAL_TO:
	case GREATER_THAN:
	case LESS_THAN_OR_EQUAL_TO:
	case LESS_THAN:
	case EQ:
	    return true;
	default:{
	    RESTORE_CHECKPOINT(CKP);
	    return false;
	}
    }
}

/*
 * The caller of E()
 */
bool
start_mathexpr_parse(){
    bool parse_result;
    int token_code;

    parse_result = E();

    if ((token_code = cyylex()) != PARSER_EOF){
	printf("* Rejected\n");
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

/*
 * The caller of Q()
 */
bool
start_ineq_mathexpr_parse(){
    bool parse_result;
    int token_code;

    parse_result = Q();

    if ((token_code = cyylex()) != PARSER_EOF){
	printf("* Rejected\n");
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