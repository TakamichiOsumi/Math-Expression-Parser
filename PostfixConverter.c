#include <stdio.h>
#include <stdlib.h>
#include "ExportedParser.h"
#include "Stack/stack.h"

#define SKIP_TOKEN(token_code) \
    (token_code == WHITE_SPACE ||		\
     token_code == TAB || token_code == PARSER_EOF)

lex_data *
convert_infix_to_postfix(lex_data *infix, int size_in,
			 int *size_out){
    stack *postfix;
    int iter;

    postfix = stack_init(size_in);
    for (iter = 0; iter < size_in; iter++){
	if (SKIP_TOKEN(infix[iter].token_code))
	    continue;
    }

    return NULL;
}
