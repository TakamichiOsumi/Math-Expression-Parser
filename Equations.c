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

    token_code = cyylex();
    if (token_code != MATH_POW){
	yyrewind(1);
	return false;
    }

    token_code = cyylex();
    if (token_code != MATH_BRACKET_START){
	yyrewind(2);
	return false;
    }

    token_code = cyylex();
    if (token_code != VARIABLE){
	yyrewind(3);
	return false;
    }

    if (strcmp(STACK_TOPMOST_ELEM.token_val, "x") != 0){
	yyrewind(3);
	return false;
    }

    token_code = cyylex();
    if (token_code != MATH_COMMA){
	yyrewind(4);
	return false;
    }

    token_code = cyylex();
    if (token_code != INT){
	yyrewind(5);
	return false;
    }

    if (strcmp(STACK_TOPMOST_ELEM.token_val, "2") != 0){
	yyrewind(5);
	return false;
    }

    /* pow(x, 2) will by done by this call */
    token_code = cyylex();
    if (token_code != MATH_BRACKET_END){
	yyrewind(6);
	return false;
    }

    /* + */
    token_code = cyylex();
    if (token_code != MATH_PLUS){
	yyrewind(7);
	return false;
    }

    token_code = cyylex();
    if (token_code != MATH_POW){
	yyrewind(8);
	return false;
    }

    token_code = cyylex();
    if (token_code != MATH_BRACKET_START){
	yyrewind(9);
	return false;
    }

    token_code = cyylex();
    if (token_code != VARIABLE){
	yyrewind(10);
	return false;
    }

    if (strcmp(STACK_TOPMOST_ELEM.token_val, "y") != 0){
	yyrewind(10);
	return false;
    }

    token_code = cyylex();
    if (token_code != MATH_COMMA){
	yyrewind(11);
	return false;
    }

    token_code = cyylex();
    if (token_code != INT){
	yyrewind(12);
	return false;
    }

    if (strcmp(STACK_TOPMOST_ELEM.token_val, "2") != 0){
	yyrewind(12);
	return false;
    }

    token_code = cyylex();
    if (token_code != MATH_BRACKET_END){
	yyrewind(13);
	return false;
    }

    token_code = cyylex();
    if (token_code != MATH_EQ){
	yyrewind(14);
	return false;
    }

    token_code = cyylex();
    if (token_code != INT && token_code != DOUBLE){
	yyrewind(15);
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
