#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include "ExportedParser.h"
#include "MexprEnums.h"

/*
 * The grammer to parse mathmatical expression:
 *
 * a. E -> E + T | E - T | T
 * b. T -> T * F | T / F | F
 * c. F -> INTEGER | DOUBLE | VAR | '(' E ')'
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
 *
 * where $ means empty.
 */

static bool E_dash(void);
static bool T(void);
static bool T_dash(void);
static bool F(void);

#define RETURN_FALSE \
    { printf("%s will return false from line = %d\n",	\
	     __FUNCTION__, __LINE__); return false; }

/*
 * Any rule which contains dollar sign, the function for the non-terminal symbol
 * must never return error. It should always return success. The way to implement
 * dollar sign is simply return success.
 */

/* 1. E  -> T E' */
static bool
E(void){
    int token_code, CKP;

    CHECKPOINT(CKP);

    if (T() == false){
	RESTORE_CHECKPOINT(CKP);
	RETURN_FALSE;
    }

    if (E_dash() == false){
	RESTORE_CHECKPOINT(CKP);
	RETURN_FALSE;
    }

    /*
     * Retrun false if we didn't use all the tokens
     * in the lex buffer.
     *
     * Without this condition, writing up a list of
     * multiple 'F's returns true (like '1 1 1').
     */
    if ((token_code = cyylex()) != PARSER_EOF)
	RETURN_FALSE;

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
	RETURN_FALSE;
    }

    if (T_dash() == false){
	RESTORE_CHECKPOINT(CKP);
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

/* 5. F -> INTEGER | DOUBLE | VAR | ( E ) */
static bool
F(void){
    int token_code, CKP;

    CHECKPOINT(CKP);

    /* F -> '(' E ')' */
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
		RESTORE_CHECKPOINT(CKP);
		RETURN_FALSE;
	}
    } while(0);

    RESTORE_CHECKPOINT(CKP);
    RETURN_FALSE;
}

bool
start_mathexpr_parse(){
    bool rc;

    if ((rc = E()) == true){
	printf("*valid*\n");

	return true;
    }else{
	printf("*invalid*\n");

	return false;
    }
}
