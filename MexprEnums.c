#include <assert.h>
#include <stdbool.h>
#include "MexprEnums.h"

int
Mexpr_operator_precedence(int token_code){

    switch(token_code){
	case MAX:
	case MIN:
	case POW:
	    return 7;
	case MULTIPLY:
	case DIVIDE:
	case REMAINDER:
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
	    /* Not yet implemented */
	    /* case AND: return 2; */
	    /* case OR: return 1; */
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
Mexpr_get_string_token(int token_code){
    switch(token_code){
	case INVALID:
	    return "INVALID";
	case INT:
	    return "INT";
	case DOUBLE:
	    return "DOUBLE";
	case VARIABLE:
	    return "VARIABLE";
	case BRACKET_START:
	    return "BRACKET_START";
	case BRACKET_END:
	    return "BRACKET_END";
	case COMMA:
	    return "COMMA";
	case PLUS:
	    return "PLUS";
	case MINUS:
	    return "MINUS";
	case MULTIPLY:
	    return "MULTIPLY";
	case DIVIDE:
	    return "DIVIDE";
	case REMAINDER:
	    return "REMAINDER";
	case MIN:
	    return "MIN";
	case MAX:
	    return "MAX";
	case SQR:
	    return "SQR";
	case SQRT:
	    return "SQRT";
	case SIN:
	    return "SIN";
	case COS:
	    return "COS";
	case POW:
	    return "POW";
	case GREATER_THAN_OR_EQUAL_TO:
	    return "GREATER_THAN_OR_EQUAL_TO";
	case GREATER_THAN:
	    return "GREATER_THAN";
	case LESS_THAN_OR_EQUAL_TO:
	    return "LESS_THAN_OR_EQUAL_TO";
	case LESS_THAN:
	    return "LESS_THAN";
	case NEQ:
	    return "NEQ";
	case EQ:
	    return "MATH_EQ";
	case WHITE_SPACE:
	    return "SPACE";
	case TAB:
	    return "TAB";
	case PARSER_EOF:
	    return "PARSER_EOF";
	default:
	    break;
    }

    assert(0);
}
