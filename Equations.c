#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include "ExportedParser.h"
#include "MexprEnums.h"

/*
 * Return true, if user inputs pow(x, 2) + pow(y, 2) = <CONSTANT>.
 *
 * Otherwise, return false.
 */
static bool
circle_eqn(){
    int token_code;

    CHECKPOINT(); /* preparation for failure */

    token_code = cyylex();
    if (token_code != MATH_POW){
	RESTORE_CHECKPOINT();
	return false;
    }

    token_code = cyylex();
    if (token_code != MATH_BRACKET_START){
	RESTORE_CHECKPOINT();
	return false;
    }

    token_code = cyylex();
    if (token_code != VARIABLE){
	RESTORE_CHECKPOINT();
	return false;
    }

    if (strcmp(STACK_TOPMOST_ELEM.token_val, "x") != 0){
	RESTORE_CHECKPOINT();
	return false;
    }

    token_code = cyylex();
    if (token_code != MATH_COMMA){
	RESTORE_CHECKPOINT();
	return false;
    }

    token_code = cyylex();
    if (token_code != INT){
	RESTORE_CHECKPOINT();
	return false;
    }

    if (strcmp(STACK_TOPMOST_ELEM.token_val, "2") != 0){
	RESTORE_CHECKPOINT();
	return false;
    }

    /* pow(x, 2) will by done by this call */
    token_code = cyylex();
    if (token_code != MATH_BRACKET_END){
	RESTORE_CHECKPOINT();
	return false;
    }

    /* + */
    token_code = cyylex();
    if (token_code != MATH_PLUS){
	RESTORE_CHECKPOINT();
	return false;
    }

    token_code = cyylex();
    if (token_code != MATH_POW){
	RESTORE_CHECKPOINT();
	return false;
    }

    token_code = cyylex();
    if (token_code != MATH_BRACKET_START){
	RESTORE_CHECKPOINT();
	return false;
    }

    token_code = cyylex();
    if (token_code != VARIABLE){
	RESTORE_CHECKPOINT();
	return false;
    }

    if (strcmp(STACK_TOPMOST_ELEM.token_val, "y") != 0){
	RESTORE_CHECKPOINT();
	return false;
    }

    token_code = cyylex();
    if (token_code != MATH_COMMA){
	RESTORE_CHECKPOINT();
	return false;
    }

    token_code = cyylex();
    if (token_code != INT){
	RESTORE_CHECKPOINT();
	return false;
    }

    if (strcmp(STACK_TOPMOST_ELEM.token_val, "2") != 0){
	RESTORE_CHECKPOINT();
	return false;
    }

    token_code = cyylex();
    if (token_code != MATH_BRACKET_END){
	RESTORE_CHECKPOINT();
	return false;
    }

    token_code = cyylex();
    if (token_code != MATH_EQ){
	RESTORE_CHECKPOINT();
	return false;
    }

    token_code = cyylex();
    if (token_code != INT && token_code != DOUBLE){
	RESTORE_CHECKPOINT();
	return false;
    }

    return true;
}

void
parse_equations(){
    bool rc;

    if ((rc = circle_eqn()) == true){
	printf("Equation of Circle\n");
    }else{
	printf("Not equation of Circle\n");
    }
}
