#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include "ExportedParser.h"
#include "MexprEnums.h"

static bool
compare_variable_id(char *expected){
    lex_data *top_data = &lstack.main_data[lstack.stack_pointer - 1];

    if (top_data->token_code != VARIABLE){
	return false;
    }

    if (strncmp(top_data->token_val,
		expected, strlen(expected)) != 0){
	printf("expected the token to be '%s', but it was '%s'\n",
	       expected, top_data->token_val);
	return false;
    }

    return true;
}

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
    if (compare_variable_id("select") == false){
	RESTORE_CHECKPOINT(CKP);
	return false;
    }

    /* cols */
    if (cols() == false)
	return false;

    /* from */
    token_code = cyylex();
    if (compare_variable_id("from") == false){
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
	printf("*valid*\n");
    }else{
	printf("*invalid*\n");
    }
}
