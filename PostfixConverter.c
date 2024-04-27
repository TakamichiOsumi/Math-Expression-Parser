#include <stdio.h>
#include <stdlib.h>
#include "Linked-List/linked_list.h"
#include "ExportedParser.h"
#include "Stack/stack.h"

/* Prefixed like it's one part of stack library */
static int
stack_top_token_code(stack *s){
    lex_data *top;

    assert(!stack_is_empty(s));

    top = (lex_data *) stack_top(s);

    return top->token_code;
}

/* for debug */
/*
static void
print_infix_lex_data(lex_data *infix, int size_in){
    lex_data *curr;
    int iter;

    printf("---- <Infix> ------\n");
    for (iter = 0; iter < size_in; iter++){
	curr = &infix[iter];
	if (is_skipped_token(curr->token_code))
	    continue;
	printf("%s ", curr->token_val);
    }
    printf("\n");
}
*/

/* for debug */
static void
print_postfix_list(linked_list *postfix){
    lex_data *curr;
    node *n;

    printf("---- <Postfix> ----\n");
    ll_begin_iter(postfix);
    while((n = ll_get_iter_node(postfix)) != NULL){
	curr = (lex_data *) n->data;
	printf("%s ", curr->token_val);
    }
    ll_end_iter(postfix);
    printf("\n");
}

linked_list *
convert_infix_to_postfix(lex_data *infix, int size_in){
    linked_list *postfix_array;
    lex_data *curr;
    stack *s;
    int iter;

    s = stack_init(size_in);
    postfix_array = ll_init(NULL, NULL);

    /* print_infix_lex_data(infix, size_in); */

    for (iter = 0; iter < size_in; iter++){
	curr = &infix[iter];

	if (is_skipped_token(curr->token_code))
	    continue;

	if (is_operand(curr->token_code)){
	    ll_tail_insert(postfix_array, (void *) curr);
	}else if (curr->token_code == BRACKET_START){
	    stack_push(s, curr);
	}else if (is_operator(curr->token_code)){
	    while(!stack_is_empty(s) &&
		  !is_unary_operator(curr->token_code) &&
		  (operator_precedence(curr->token_code) <=
		   operator_precedence(stack_top_token_code(s))))
		ll_tail_insert(postfix_array, stack_pop(s));

	    stack_push(s, curr);
	}else if (curr->token_code == BRACKET_END){
	    while(!stack_is_empty(s) &&
		  stack_top_token_code(s) != BRACKET_START)
		ll_tail_insert(postfix_array, stack_pop(s));

	    (void) stack_pop(s);

	    while(!stack_is_empty(s)){
		if (is_unary_operator(stack_top_token_code(s))){
		    ll_tail_insert(postfix_array, (void *) stack_pop(s));
		    continue;
		}
		break;
	    }
	}else if (curr->token_code == COMMA){
	    while(!stack_is_empty(s) &&
		  stack_top_token_code(s) != BRACKET_START)
		ll_tail_insert(postfix_array, (void *) stack_pop(s));
	}
    }

    while(!stack_is_empty(s))
	ll_tail_insert(postfix_array, (void *) stack_pop(s));

    print_postfix_list(postfix_array);

    return postfix_array;
}
