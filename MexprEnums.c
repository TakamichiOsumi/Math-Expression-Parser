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
	case MATH_BRACKET_START:
	    return "MATH_BRACKET_START";
	case MATH_BRACKET_END:
	    return "MATH_BRACKET_END";
	case MATH_COMMA:
	    return "MATH_COMMA";
	case MATH_PLUS:
	    return "MATH_PLUS";
	case MATH_MINUS:
	    return "MATH_MINUS";
	case MATH_DIVIDE:
	    return "MATH_DIVIDE";
	case MATH_REMAINDER:
	    return "MATH_REMAINDER";
	case MATH_EQ:
	    return "MATH_EQ";
	case MATH_MIN:
	    return "MATH_MIN";
	case MATH_MAX:
	    return "MATH_MAX";
	case MATH_SIN:
	    return "MATH_SIN";
	case MATH_COS:
	    return "MATH_COS";
	case MATH_POW:
	    return "MATH_POW";
	case WHITE_SPACE:
	    return "WHITE_SPACE";
	case DTYPE_END:
	    return "DTYPE_END";
	case PARSER_EOF:
	    return "PARSER_EOF";
	default:
	    return "(NA)";
    }
}
