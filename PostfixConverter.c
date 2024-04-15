#include <stdio.h>
#include <stdlib.h>
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

static bool
is_skipped_token(token_code){
    return (token_code == WHITE_SPACE || token_code == TAB ||
	    token_code == PARSER_EOF);
}

static bool
is_operand(token_code){
    return (token_code == VARIABLE || token_code == INT ||
	    token_code == DOUBLE);
}

static bool
is_operator(token_code){
    switch(token_code){
	case PLUS:
	case MINUS:
	case MULTIPLY:
	case DIVIDE:
	case REMAINDER:
	case MAX:
	case MIN:
	case POW:
	case SIN:
	case COS:
	case SQR:
	case SQRT:
	    return true;
	default:
	    return false;
    }
}

static bool
is_unary_operator(token_code){
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

static void
handle_operator(stack *s, lex_data *current){
    lex_data *top;

    while(!stack_is_empty(s) &&
	  !is_unary_operator(current->token_code) &&
	  (Mexpr_operator_precedence(current->token_code) <=
	   Mexpr_operator_precedence(stack_top_token_code(s)))){
	top = (lex_data *) stack_pop(s);
	printf("%s ", top->token_val);
    }

    stack_push(s, current);
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
	if (is_skipped_token(curr->token_code))
	    continue;
	printf("%s ", curr->token_val);
    }
    printf("\n");

    printf("---- <Postfix> ----\n");
    for (iter = 0; iter < size_in; iter++){
	curr = &infix[iter];
	if (is_skipped_token(curr->token_code))
	    continue;

	if (is_operand(curr->token_code)){
	    printf("%s ", curr->token_val);
	}else if (curr->token_code == BRACKET_START){
	    stack_push(s, curr);
	}else if (is_operator(curr->token_code)){
	    handle_operator(s, curr);
	}else if (curr->token_code == BRACKET_END){
	    lex_data *top;

	    while(!stack_is_empty(s) &&
		  stack_top_token_code(s) != BRACKET_START){
		top = (lex_data *) stack_pop(s);
		printf("%s ", top->token_val);
	    }

	    /* The next token must be BRACKET_START */
	    top = stack_pop(s);
	    assert(top->token_code == BRACKET_START);

	    while(!stack_is_empty(s)){
		if (is_unary_operator(stack_top_token_code(s))){
		    top = stack_pop(s);
		    printf("%s ", top->token_val);
		    continue;
		}
		break;
	    }
	}else if (curr->token_code == COMMA){
	    lex_data *top;

	    while(!stack_is_empty(s) &&
		  stack_top_token_code(s) != BRACKET_START){
		top = stack_pop(s);
		printf("%s ", top->token_val);
	    }
	}
    }

    while(!stack_is_empty(s)){
	curr = (lex_data *) stack_pop(s);
	printf("%s ", curr->token_val);
    }

    printf("\n-------------------\n");

    return NULL;
}
