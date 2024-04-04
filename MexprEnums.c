#include "MexprEnums.h"

char *
Mexpr_get_string_token(int token_code){
    switch(token_code){
	case MATH_INT:
	    return "MATH_INT";
	case MATH_DOUBLE:
	    return "MATH_DOUBLE";
	case MATH_VARIABLE:
	    return "MATH_VARIABLE";
	case MATH_DTYPE_END:
	    return "MATH_DTYPE_END";
	case PARSER_EOF:
	    return "PARSER_EOF";
	default:
	    return "(NA)";
    }
}
