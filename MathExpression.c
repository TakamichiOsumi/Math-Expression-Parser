#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include "ExportedParser.h"
#include "MexprEnums.h"

/*
 * The production rules for math expression that avoid left recursion :
 *
 * E  ->  T E'
 * E' ->  + T E' | - T E' | $
 * T  ->  F T'
 * T' ->  * F T' | / F T' | $
 * F  ->  INT | DOUBLE | VAR | ( E ) | G ( E , E ) | P ( E )
 * Q  ->  E I E
 * I  ->  = | != | < | > | <= | >=
 * P  ->  SIN | COS | SQRT
 * G  ->  MAX | MIN | POW
 *
 * Dollar sign means empty.
 *
 * With regard to the rule which contains dollar sign, the function for
 * the non-terminal symbol must never return error. It should always
 * return success. The way to implement dollar sign is simply return success.
 */
static bool E_dash(void);
static bool T(void);
static bool T_dash(void);
static bool F(void);
static bool I(void);
static bool P(void);
static bool G(void);

/* E  -> T E' */
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

/* E' -> + T E' | - T E' | $ */
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

/* T  -> F T' */
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

/* T' -> * F T' | / F T' | $ */
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

/* F -> INT | DOUBLE | VAR | ( E ) | G ( E , E ) | P ( E ) */
static bool
F(void){
    int token_code, CKP;

    CHECKPOINT(CKP);

    /* F -> ( E ) */
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
		break;
	}
    } while(0);

    RESTORE_CHECKPOINT(CKP);

    /* F -> P ( E ) */
    do {
	if (P() == false)
	    break;

	if ((token_code = cyylex()) != BRACKET_START)
	    break;

	if (E() == false)
	    break;

	if ((token_code = cyylex()) != BRACKET_END)
	    break;

	return true;

    } while(0);

    RESTORE_CHECKPOINT(CKP);

    /* F -> G ( E , E ) */
    do {
	if (G() == false)
	    break;

	if ((token_code = cyylex()) != BRACKET_START)
	    break;

	if (E() == false)
	    break;

	if ((token_code = cyylex()) != COMMA)
	    break;

	if (E() == false)
	    break;

	if ((token_code = cyylex()) != BRACKET_END)
	    break;

	return true;

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

    if (I() == false){
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
I(void){
    int token_code, CKP;

    CHECKPOINT(CKP);

    token_code = cyylex();
    switch(token_code){
	case GREATER_THAN_OR_EQUAL_TO:
	case GREATER_THAN:
	case LESS_THAN_OR_EQUAL_TO:
	case LESS_THAN:
	case NEQ:
	case EQ:
	    return true;
	default:{
	    RESTORE_CHECKPOINT(CKP);
	    return false;
	}
    }
}

static bool
P(void){
    int token_code, CKP;

    CHECKPOINT(CKP);

    token_code = cyylex();
    switch(token_code){
	case SIN:
	case COS:
	case SQR:
	case SQRT:
	    return true;
	default:
	    break;
    }

    RESTORE_CHECKPOINT(CKP);

    return false;
}

static bool
G(void){
    int token_code, CKP;

    CHECKPOINT(CKP);

    token_code = cyylex();
    switch(token_code){
	case MAX:
	case MIN:
	case POW:
	    return true;
	default:
	    break;
    }

    RESTORE_CHECKPOINT(CKP);

    return false;
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
