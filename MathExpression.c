#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include "ExportedParser.h"
#include "MexprEnums.h"

/*
 * A grammer to parse mathmatical expression:
 *
 * E -> E + T | E - T | T
 * T -> T * F | T / F | F
 * F -> INTEGER | DECIMAL | VAR | ( E )
 *
 * But the first two rules contain left recursion and
 * can't be written as it is.
 * Below is the technique to avoid the coding problem.
 *
 * E  -> T E'
 * E' -> + T E' | - T E' | $
 *
 * where $ represents empty.
 *
 * Similarly,
 * T  -> F T'
 * T' -> * F T' | / F T' | $
 *
 */
static bool
E(void){
    int token_code, CKP;

    return true;
}

static bool
E_dash(void){
    int token_code, CKP;

    return true;
}

static bool
T(void){
    int token_code, CKP;

    return true;
}

static bool
T_dash(void){
    int token_code, CKP;

    return true;
}

static bool
F(void){
    int token_code, CKP;

    return true;
}

void
start_mathexpr_parse(){
    bool rc;

    if ((rc = E()) == true){
	printf("*valid*\n");
    }else{
	printf("*invalid*\n");
    }
}
