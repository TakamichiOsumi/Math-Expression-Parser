#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include "ExportedParser.h"
#include "MexprEnums.h"

/*
 * The production rules for math expression that avoid left recursion :
 *
 * <Arithmetic Operations>, <Supported Math Functions> and <Inequality Operators>.
 *
 *  E  ->  T E'
 *  E' ->  + T E' | - T E' | $
 *  T  ->  F T'
 *  T' ->  * F T' | / F T' | $
 *  F  ->  INT | DOUBLE | VAR | ( E ) | G ( E , E ) | P ( E )
 *  Q  ->  E I E
 *  I  ->  = | != | < | > | <= | >=
 *  P  ->  SIN | COS | SQR | SQRT
 *  G  ->  MAX | MIN | POW
 *
 * <Logical Operators>
 *
 *  S  ->  J S'
 *  S' ->  OR J S' | $
 *  J  ->  K J'
 *  J' ->  AND K J' | $
 *  K  ->  ( S ) K' | D K' | Q L K K'
 *  K' ->  L Q K' | $
 *  D  ->  Q L Q
 *  L  ->  AND | OR
 *
 * Dollar sign means empty.
 *
 * With regard to the rule which contains dollar sign, the function for
 * the non-terminal symbol must never return error. It should always
 * return success. The way to implement dollar sign is to rewind the
 * parse stack and parse position and then return success after all
 * other grammer rules.
 */
bool E(void);
static bool E_dash(void);
static bool T(void);
static bool T_dash(void);
static bool F(void);
static bool I(void);
static bool P(void);
bool Q(void);
static bool G(void);
bool S(void);
static bool S_dash(void);
static bool J(void);
static bool J_dash(void);
static bool K(void);
static bool K_dash(void);
static bool D(void);
static bool L(void);

/* E  -> T E' */
bool
E(void){
    int CKP;

    CHECKPOINT(CKP);

    do {
	if (T() == false)
	    break;

	if (E_dash() == false)
	    break;

	return true;

    } while(0);

    RESTORE_CHECKPOINT(CKP);

    return false;
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

    /*
     * This production rule contains dollar.
     *
     * Restore the parse position of lex buffer and
     * the stack contents that were moved or created
     * since this function started, to their original
     * states. Then, return true.
     *
     * Let the caller apply other grammer rule.
     */
    RESTORE_CHECKPOINT(CKP);

    return true;
}

/* T  -> F T' */
static bool
T(void){
    int CKP;

    CHECKPOINT(CKP);

    do {
	if (F() == false)
	    break;

	if (T_dash() == false)
	    break;

	return true;

    } while(0);

    RESTORE_CHECKPOINT(CKP);

    return false;
}

/* T' -> * F T' | / F T' | % F T' | $ */
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

    /* T' -> % F T' */
    do {
	if ((token_code = cyylex()) != MOD)
	    break;

	if (F() == false)
	    break;

	if (T_dash() == false)
	    break;

	return true;

    } while(0);

    RESTORE_CHECKPOINT(CKP);

    /*
     * T' -> $.
     *
     * See E_dash() for more detail.
     */

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
	    break;

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

    do {
	if (E() == false)
	    break;

	if (I() == false)
	    break;

	if (E() == false)
	    break;

	return true;

    } while(0);

    RESTORE_CHECKPOINT(CKP);

    return false;
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
	default:
	    break;
    }

    RESTORE_CHECKPOINT(CKP);

    return false;
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

/* S -> J S' */
bool
S(void){
    int CKP;

    CHECKPOINT(CKP);

    do {
	if (J() == false)
	    break;

	if (S_dash() == false)
	    break;

	return true;

    } while(0);

    RESTORE_CHECKPOINT(CKP);

    return false;
}

/* S' -> OR J S' | $ */
static bool
S_dash(void){
    int token_code, CKP;

    CHECKPOINT(CKP);

    do {
	if ((token_code = cyylex()) != OR)
	    break;

	if (J() == false)
	    break;

	if (S_dash() == false)
	    break;

	return true;

    } while(0);

    RESTORE_CHECKPOINT(CKP);

    /* $ sign */

    return true;
}

/* J -> K J' */
static bool
J(void){
    int CKP;

    CHECKPOINT(CKP);

    do {
	if (K() == false)
	    break;

	if (J_dash() == false)
	    break;

	return true;

    } while(0);

    RESTORE_CHECKPOINT(CKP);

    return false;
}

/* J' ->  AND K J' | $ */
static bool
J_dash(void){
    int token_code, CKP;

    CHECKPOINT(CKP);

    do {
	if ((token_code = cyylex()) != AND)
	    break;

	if (K() == false)
	    break;

	if (J_dash() == false)
	    break;

	return true;

    } while(0);

    RESTORE_CHECKPOINT(CKP);

    /* $ sign */

    return true;
}

static bool
K(void){
    int token_code, CKP;

    CHECKPOINT(CKP);

    /* K -> ( S ) K' */
    do {
	if ((token_code = cyylex()) != BRACKET_START)
	    break;

	if (S() == false)
	    break;

	if ((token_code = cyylex()) != BRACKET_END)
	    break;

	if (K_dash() == false)
	    break;

	return true;

    } while(0);

    RESTORE_CHECKPOINT(CKP);

    /* K -> D K' */
    do {

	if (D() == false)
	    break;

	if (K_dash() == false)
	    break;

	return true;

    } while(0);

    RESTORE_CHECKPOINT(CKP);

    /* K -> Q L K K' */
    do {

	if (Q() == false)
	    break;

	if (L() == false)
	    break;

	if (K() == false)
	    break;

	if (K_dash() == false)
	    break;

	return true;

    } while(0);

    RESTORE_CHECKPOINT(CKP);

    return false;
}

/* K' -> L Q K' | $ */
static bool
K_dash(void){
    int CKP;

    CHECKPOINT(CKP);

    do {

	if (L() == false)
	    break;

	if (Q() == false)
	    break;

	if (K_dash() == false)
	    break;

	return true;

    } while(0);

    RESTORE_CHECKPOINT(CKP);

    /* $ sign */

    return true;
}

/* D  ->  Q L Q */
static bool
D(void){
    int CKP;

    CHECKPOINT(CKP);

    do {

	if (Q() == false)
	    break;

	if (L() == false)
	    break;

	if (Q() == false)
	    break;

	return true;

    } while(0);

    RESTORE_CHECKPOINT(CKP);

    return false;
}

/* L -> AND | OR */
static bool
L(void){
    int token_code, CKP;

    CHECKPOINT(CKP);

    do {
	if ((token_code = cyylex()) != AND)
	    break;

	return true;

    } while(0);

    RESTORE_CHECKPOINT(CKP);

    do {
	if ((token_code = cyylex()) != OR)
	    break;

	return true;

    } while(0);

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
	return false;
    }else{
	if (!parse_result){
	    return false;
	}else{
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
	return false;
    }else{
	if (!parse_result){
	    return false;
	}else{
	    return true;
	}
    }
}

/*
 * The caller of S()
 */
bool
start_logical_mathexpr_parse(){
    bool parse_result;
    int token_code;

    parse_result = S();

    if ((token_code = cyylex()) != PARSER_EOF){
	return false;
    }else{
	if (!parse_result){
	    return false;
	}else{
	    return true;
	}
    }
}
