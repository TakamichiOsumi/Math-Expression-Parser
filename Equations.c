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
    int token_code, CKP;

    CHECKPOINT(CKP); /* preparation for failure */

    /* Parse 'pow(x, 2)' part of the equation */
    token_code = cyylex();
    if (token_code != POW){
	RESTORE_CHECKPOINT(CKP);
	return false;
    }

    token_code = cyylex();
    if (token_code != BRACKET_START){
	RESTORE_CHECKPOINT(CKP);
	return false;
    }

    token_code = cyylex();
    if (token_code != VARIABLE){
	RESTORE_CHECKPOINT(CKP);
	return false;
    }

    if (strcmp(STACK_TOPMOST_ELEM.token_val, "x") != 0){
	RESTORE_CHECKPOINT(CKP);
	return false;
    }

    token_code = cyylex();
    if (token_code != COMMA){
	RESTORE_CHECKPOINT(CKP);
	return false;
    }

    token_code = cyylex();
    if (token_code != INT){
	RESTORE_CHECKPOINT(CKP);
	return false;
    }

    if (strcmp(STACK_TOPMOST_ELEM.token_val, "2") != 0){
	RESTORE_CHECKPOINT(CKP);
	return false;
    }

    token_code = cyylex();
    if (token_code != BRACKET_END){
	RESTORE_CHECKPOINT(CKP);
	return false;
    }

    /* + */
    token_code = cyylex();
    if (token_code != PLUS){
	RESTORE_CHECKPOINT(CKP);
	return false;
    }

    /* Parse 'pow(y, 2)' part of the equation */
    token_code = cyylex();
    if (token_code != POW){
	RESTORE_CHECKPOINT(CKP);
	return false;
    }

    token_code = cyylex();
    if (token_code != BRACKET_START){
	RESTORE_CHECKPOINT(CKP);
	return false;
    }

    token_code = cyylex();
    if (token_code != VARIABLE){
	RESTORE_CHECKPOINT(CKP);
	return false;
    }

    if (strcmp(STACK_TOPMOST_ELEM.token_val, "y") != 0){
	RESTORE_CHECKPOINT(CKP);
	return false;
    }

    token_code = cyylex();
    if (token_code != COMMA){
	RESTORE_CHECKPOINT(CKP);
	return false;
    }

    token_code = cyylex();
    if (token_code != INT){
	RESTORE_CHECKPOINT(CKP);
	return false;
    }

    if (strcmp(STACK_TOPMOST_ELEM.token_val, "2") != 0){
	RESTORE_CHECKPOINT(CKP);
	return false;
    }

    token_code = cyylex();
    if (token_code != BRACKET_END){
	RESTORE_CHECKPOINT(CKP);
	return false;
    }

    token_code = cyylex();
    if (token_code != EQ){
	RESTORE_CHECKPOINT(CKP);
	return false;
    }

    token_code = cyylex();
    if (token_code != INT && token_code != DOUBLE){
	RESTORE_CHECKPOINT(CKP);
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
