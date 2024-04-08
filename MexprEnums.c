#include "MexprEnums.h"

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
	case DIVIDE:
	    return "DIVIDE";
	case REMAINDER:
	    return "REMAINDER";
	case EQ:
	    return "MATH_EQ";
	case MIN:
	    return "MIN";
	case MAX:
	    return "MAX";
	case SIN:
	    return "SIN";
	case COS:
	    return "COS";
	case POW:
	    return "POW";
	case SELECT:
	    return "SELECT";
	case FROM:
	    return "FROM";
	case WHITE_SPACE:
	    return "SPACE";
	case TAB:
	    return "TAB";
	case PARSER_EOF:
	    return "PARSER_EOF";
	default:
	    return "NA";
    }
}
