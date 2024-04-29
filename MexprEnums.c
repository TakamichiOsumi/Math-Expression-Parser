#include <assert.h>
#include <stdbool.h>
#include "MexprEnums.h"

int
operator_precedence(int token_code){

    switch(token_code){
	case MAX:
	case MIN:
	case POW:
	    return 7;
	case MULTIPLY:
	case DIVIDE:
	case MOD:
	    return 6;
	case PLUS:
	case MINUS:
	    return 5;
	case SIN:
	case COS:
	case SQR:
	case SQRT:
	    return 4;
	case GREATER_THAN_OR_EQUAL_TO:
	case GREATER_THAN:
	case LESS_THAN_OR_EQUAL_TO:
	case LESS_THAN:
	case NEQ:
	case EQ:
	    return 3;
	case AND:
	    return 2;
	case OR:
	    return 1;
	case BRACKET_START:
	case BRACKET_END:
	    return 0;
	default:
	    break;
    }

    assert(0);

    return 0;
}

char *
get_string_token(int token_code){
    switch(token_code){
	case INVALID:
	    return "INVALID";

	    /* Whitespace symbol */
	case WHITE_SPACE:
	    return "SPACE";
	case TAB:
	    return "TAB";
	case PARSER_EOF:
	    return "PARSER_EOF";

	    /* Punctuation marker */
	case BRACKET_START:
	    return "BRACKET_START";
	case BRACKET_END:
	    return "BRACKET_END";
	case COMMA:
	    return "COMMA";

	    /* Binary operator */
	case PLUS:
	    return "PLUS";
	case MINUS:
	    return "MINUS";
	case MULTIPLY:
	    return "MULTIPLY";
	case DIVIDE:
	    return "DIVIDE";
	case MOD:
	    return "MOD";
	case MIN:
	    return "MIN";
	case MAX:
	    return "MAX";
	case POW:
	    return "POW";

	    /* Unary operator */
	case SQR:
	    return "SQR";
	case SQRT:
	    return "SQRT";
	case SIN:
	    return "SIN";
	case COS:
	    return "COS";

	    /* Inequality operator */
	case GREATER_THAN_OR_EQUAL_TO:
	    return "GREATER_THAN_OR_EQUAL_TO";
	case LESS_THAN_OR_EQUAL_TO:
	    return "LESS_THAN_OR_EQUAL_TO";
	case GREATER_THAN:
	    return "GREATER_THAN";
	case LESS_THAN:
	    return "LESS_THAN";
	case NEQ:
	    return "NEQ";
	case EQ:
	    return "EQ";

	    /* Logical operator */
	case OR:
	    return "OR";
	case AND:
	    return "AND";

	    /* Data type */
	case INT:
	    return "INT";
	case DOUBLE:
	    return "DOUBLE";
	case VARIABLE:
	    return "VARIABLE";
	case BOOLEAN:
	    return "BOOLEAN";

	default:
	    break;
    }

    assert(0);
}

bool
is_skipped_token(int token_code){
    return (token_code == WHITE_SPACE || token_code == TAB ||
	    token_code == PARSER_EOF);
}

bool
is_operand(int token_code){
    return (token_code == VARIABLE || token_code == INT ||
	    token_code == DOUBLE || token_code == BOOLEAN);
}

bool
is_operator(int token_code){
    switch(token_code){
	/* Binary operator */
	case PLUS:
	case MINUS:
	case MULTIPLY:
	case DIVIDE:
	case MOD:
	case MIN:
	case MAX:
	case POW:
	/* Unary operator */
	case SIN:
	case COS:
	case SQR:
	case SQRT:
	/* Inequality operator */
	case GREATER_THAN_OR_EQUAL_TO:
	case LESS_THAN_OR_EQUAL_TO:
	case GREATER_THAN:
	case LESS_THAN:
	case NEQ:
	case EQ:
	/* Logical operator */
	case OR:
	case AND:
	    return true;
	default:
	    return false;
    }
}

bool
is_unary_operator(int token_code){
    switch(token_code){
	case SIN:
	case COS:
	case SQR:
	case SQRT:
	    return true;
	default:
	    return false;
    }
}

bool
is_binary_operator(int token_code){
    switch(token_code){
	case PLUS:
	case MINUS:
	case MULTIPLY:
	case DIVIDE:
	case MOD:
	case MIN:
	case MAX:
	case POW:
	case GREATER_THAN_OR_EQUAL_TO:
	case LESS_THAN_OR_EQUAL_TO:
	case GREATER_THAN:
	case LESS_THAN:
	case NEQ:
	case EQ:
	case OR:
	case AND:
	    return true;
	default:
	    return false;
    }
}
