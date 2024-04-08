#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include "ExportedParser.h"
#include "MexprEnums.h"

/*
 * The syntax:
 *
 * Q     -> select COLS from TABLE
 * COLS  -> COL | COL, COLS
 * COL   -> VARIABLE
 * TABLE -> VARIABLE
 */
static bool
cols(void){
    int token_code, CKP;
    bool result;

    CHECKPOINT(CKP);

    /* Rule : 'COL, COLS' */
    do {
	if ((token_code = cyylex()) != VARIABLE){
	    RESTORE_CHECKPOINT(CKP);
	    break;
	}

	if ((token_code = cyylex()) != COMMA){
	    RESTORE_CHECKPOINT(CKP);
	    break;
	}

	/* Recursive call */
	if ((result = cols()) == false){
	    RESTORE_CHECKPOINT(CKP);
	    break;
	}

	return true;

    } while(0);

    /* Rule : 'COL' */
    if ((token_code = cyylex()) != VARIABLE){
	RESTORE_CHECKPOINT(CKP);
	return false;
    }

    return true;
}

static bool
parse_sql_select(){
    int token_code, CKP;

    /* Preparation for failure */
    CHECKPOINT(CKP);

    /* select */
    token_code = cyylex();
    if (token_code != SELECT){
	RESTORE_CHECKPOINT(CKP);
	return false;
    }

    /* cols */
    if (cols() == false)
	return false;

    /* from */
    token_code = cyylex();
    if (token_code != FROM){
	RESTORE_CHECKPOINT(CKP);
	return false;
    }

    /* table */
    token_code = cyylex();
    if (token_code != VARIABLE){
	RESTORE_CHECKPOINT(CKP);
	return false;
    }

    /* Check if the input has the PARSER_EOF at the end */
    token_code = cyylex();
    if (token_code != PARSER_EOF){
	RESTORE_CHECKPOINT(CKP);
	return false;
    }

    return true;
}

void
start_sql_parse(){
    bool rc;

    if ((rc = parse_sql_select()) == true){
	printf("*valid* SQL SELECT statements\n");
    }else{
	printf("Invalid SQL SELECT statements\n");
    }
}
