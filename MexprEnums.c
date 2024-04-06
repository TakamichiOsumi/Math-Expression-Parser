#include "MexprEnums.h"

char *
Mexpr_get_string_token(int token_code){
    switch(token_code){
	case INT:
	    return "INT";
	case DOUBLE:
	    return "DOUBLE";
	case VARIABLE:
	    return "VARIABLE";
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
