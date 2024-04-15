#include <stdio.h>
#include <stdlib.h>
#include "ExportedParser.h"
#include "Stack/stack.h"

#define SKIP_TOKEN(token_code) \
    (token_code == WHITE_SPACE ||		\
     token_code == TAB || token_code == PARSER_EOF)

#define IS_OPERAND(token_code)			\
    (token_code == VARIABLE ||			\
     token_code == INT || token_code == DOUBLE)

#define IS_OPERATOR(token_code)			  \
    (token_code == PLUS || token_code == MINUS || \
     token_code == MULTIPLY || token_code == DIVIDE ||	\
     token_code == REMAINDER)

static void
handle_operator(stack *s, lex_data *current){
    lex_data *top;

    do {
	/* Now, do we have at least one element in the stack ? */
	if (stack_is_empty(s)){
	    stack_push(s, current);
	    break;
	}

	/* top() returns its reference only */
	top = (lex_data *) stack_top(s);

	if (top->token_code == BRACKET_START){
	    stack_push(s, current);
	    break;
	}

	if (Mexpr_operator_precedence(top->token_code)
	    < Mexpr_operator_precedence(current->token_code)){
	    stack_push(s, current);
	    break;
	}

	/* pop() deletes an element from the top */
	top = (lex_data *) stack_pop(s);
	printf("%s ", top->token_val);

    } while(1);

}

lex_data *
convert_infix_to_postfix(lex_data *infix, int size_in,
			 int *size_out){
    stack *s;
    int iter;
    lex_data *curr;

    s = stack_init(size_in);

    printf("---- <Infix> ------\n");
    for (iter = 0; iter < size_in; iter++){
	curr = &infix[iter];
	if (SKIP_TOKEN(curr->token_code))
	    continue;
	printf("%s ", curr->token_val);
    }
    printf("\n");

    printf("---- <Postfix> ----\n");
    for (iter = 0; iter < size_in; iter++){
	curr = &infix[iter];

	if (SKIP_TOKEN(curr->token_code))
	    continue;

	/* printf("iter= '%02d', val = %s\n", iter, curr->token_val); */

	if (IS_OPERAND(curr->token_code))
	    printf("%s ", curr->token_val);
	else if (curr->token_code == BRACKET_START)
	    stack_push(s, curr);
	else if (IS_OPERATOR(curr->token_code))
	    handle_operator(s, curr);
	else if (curr->token_code == BRACKET_END){
	    lex_data *top;

	    top = (lex_data *) stack_pop(s);
	    while(top->token_code != BRACKET_START){
		printf("%s ", top->token_val);
		top = (lex_data *) stack_pop(s);
	    }
	}
    } /* for */

    while(!stack_is_empty(s)){
	curr = (lex_data *) stack_pop(s);
	printf("%s ", curr->token_val);
    }

    printf("\n-------------------\n");

    return NULL;
}
