
%{

#include <assert.h>
#include <stdbool.h>
#include "ExportedParser.h"
#include "Stack/stack.h"
#include "MexprTree.h"

lex_stack lstack = { INVALID, {{ 0, 0, NULL}} };
static char lex_buffer[BUFFER_LEN];
static char *next_parse_pos = lex_buffer;

/* functions required for parse processing */
void
lex_set_scan_buffer(const char *buffer){
    yy_scan_string(buffer);
}

/*
 * function for expression.
 *
 * Close access to the lstack from other files.
 */
int
lex_stack_pointer(void){
    return lstack.stack_pointer;
}

static void
lex_push(lex_data data){
    assert(lstack.stack_pointer >= 0);

    if (lstack.stack_pointer > MAX_STACK_INDEX - 1){
	printf("The stack pointer is bigger than the upper limit = %d\n",
	       lstack.stack_pointer);
	return;
    }

    lstack.main_data[lstack.stack_pointer++] = data;
}

static void
process_white_space_or_tab(int type, int n){
    lex_data ldata;

    next_parse_pos += n;
    ldata.token_code = type;
    ldata.token_len = n;

    /* Create the copy of the whitespaces/tabs */
    ldata.token_val = (char *) malloc(yyleng + 1);
    if (ldata.token_val == NULL){
	perror("malloc");
	exit(-1);
    }
    memset(ldata.token_val, '\0', yyleng + 1);
    strncpy(ldata.token_val, yytext, yyleng + 1);

    lex_push(ldata);
}

static void
parser_stack_reset(void){
    int i, removed_string_len = 0;
    lex_data *ldata;

    for (i = 0; i < lstack.stack_pointer; i++){
	ldata = &lstack.main_data[i];
	ldata->token_code = INVALID;
	removed_string_len += ldata->token_len;
	ldata->token_len = 0;
	if (ldata->token_val != NULL){
	    free(ldata->token_val);
	    ldata->token_val = NULL;
	}
    }

    lstack.stack_pointer = 0;
    next_parse_pos -= removed_string_len;
    lex_set_scan_buffer(next_parse_pos);
}

%}

%%

\n               { return PARSER_EOF; }

"TRUE"           { return BOOLEAN;  }

"FALSE"          { return BOOLEAN;  }

"or"             { return OR;    }

"and"            { return AND;   }

"min"            { return MIN;   }

"max"            { return MAX;   }

"pow"            { return POW;   }

"sin"            { return SIN;   }

"cos"            { return COS;   }

"sqrt"           { return SQRT;  }

"sqr"            { return SQR;   }

"("              { return BRACKET_START; }

")"              { return BRACKET_END;   }

"+"              { return PLUS;      }

"-"              { return MINUS;     }

"*"              { return MULTIPLY;  }

"/"              { return DIVIDE;    }

"%"              { return REMAINDER; }

","              { return COMMA;     }

">="             { return GREATER_THAN_OR_EQUAL_TO; }

"<="             { return LESS_THAN_OR_EQUAL_TO;    }

"<"              { return LESS_THAN;    }

">"              { return GREATER_THAN; }

"!="             { return NEQ;       }

"="              { return EQ;        }

0|-?[1-9][0-9]*  { return INT;       }

-?[0-9]*\.[0-9]+ { return DOUBLE;    }

[a-zA-Z1-9_]+    { return VARIABLE;  }

[ ]+             { process_white_space_or_tab(WHITE_SPACE, yyleng); }

[\t]+            { process_white_space_or_tab(TAB, yyleng);         }

.                { printf("detected invalid input '%s'\n", yytext); }

%%

/*
 * Parsed string set to lex buffer must end with a new line '\n'.
 */
bool
parsed_format_validation(char *s){
    char last_char;

    if (s == NULL){
	printf("input string is null. test failure\n");
	exit(-1);
    }

    if (strlen(s) >= 2){
	last_char = s[strlen(s) - 1];
	if (last_char != '\n'){
	    printf("invalid string format : '%s'\n", s);
	    exit(-1);
	}
    }else{
	printf("input string is too short : '%s\n", s);
	exit(-1);
    }

    return true;
}

/*
 * Clean up all the resouces and set target to the buffer.
 */
void
init_buffer(char *target){
    /* Format check */
    parsed_format_validation(target);

    /* Clean up the stack */
    parser_stack_reset();

    /* Copy the string to the lex buffer */
    memset(lex_buffer, '\0', BUFFER_LEN);
    strncpy(lex_buffer, target, strlen(target));

    /* Let the parser know which buffer to parse */
    lex_set_scan_buffer(lex_buffer);
}

static lex_data
lex_pop(){
    assert(lstack.stack_pointer >= 0);

    lstack.stack_pointer--;

    return lstack.main_data[lstack.stack_pointer];
}

int
cyylex(){
    int token_code;
    lex_data ldata;

    token_code = yylex();

    ldata.token_code = token_code;
    ldata.token_len = yyleng;
    next_parse_pos += yyleng;

    /* Create the copy of the parsed text */
    ldata.token_val = (char *) malloc(yyleng + 1);
    if (ldata.token_val == NULL){
	perror("malloc");
	exit(-1);
    }

    memset(ldata.token_val, '\0', yyleng + 1);
    strncpy(ldata.token_val, yytext, yyleng + 1);

    /* Save the info into the stack */
    lex_push(ldata);

    return token_code;
}

/* 'n' : the number to pop up the stack */
void
yyrewind(int n){
    int removed_token_len = 0;
    lex_data ldata;

    /*
     * The caller, mainly grammer parser, stores the position
     * of stack pointer as checkpoint, before it starts to parse
     * and call this function to restore the stack pointer for
     * retrial of parsing.
     *
     * In that usage, we'll never get 'n' smaller than the
     * current stack pointer.
     */
    assert(n >= 0);
    assert(lstack.stack_pointer - n >= 0);

    if (next_parse_pos == lex_buffer)
	return;

    while(n){
	ldata = lex_pop();
	removed_token_len += ldata.token_len;
	n--;

	ldata.token_code = 0;
	ldata.token_len = 0;
    }

    next_parse_pos -= removed_token_len;

    /*
     * Notify the lexical parser of the moved starting position
     * to parse.
     */
    yy_scan_string(next_parse_pos);
}

/*
 * Prefixed like it's one part of stack library, since
 * this makes convert_infix_to_postfix() easier to read.
 */
static int
stack_top_token_code(stack *s){
    lex_data *top;

    assert(!stack_is_empty(s));

    top = (lex_data *) stack_top(s);

    return top->token_code;
}

/*
static void
print_postfix_list(linked_list *postfix){
    lex_data *curr;

    printf("---- <Postfix> ----\n");
    ll_begin_iter(postfix);
    while((curr = (lex_data *) ll_get_iter_node(postfix)) != NULL){
	printf("%s ", curr->token_val);
    }
    ll_end_iter(postfix);
    printf("\n");
}
*/

linked_list *
convert_infix_to_postfix(lex_data *infix, int size_in){
    linked_list *postfix;
    lex_data *curr;
    stack *s;
    int iter;

    s = stack_init(size_in);
    postfix = ll_init(NULL, NULL);

    for (iter = 0; iter < size_in; iter++){
	curr = &infix[iter];

	if (is_skipped_token(curr->token_code))
	    continue;

	if (is_operand(curr->token_code)){
	    ll_tail_insert(postfix, curr);
	}else if (curr->token_code == BRACKET_START){
	    stack_push(s, curr);
	}else if (is_operator(curr->token_code)){
	    while(!stack_is_empty(s) &&
		  !is_unary_operator(curr->token_code) &&
		  (operator_precedence(curr->token_code) <=
		   operator_precedence(stack_top_token_code(s))))
		ll_tail_insert(postfix, stack_pop(s));

	    stack_push(s, curr);
	}else if (curr->token_code == BRACKET_END){
	    while(!stack_is_empty(s) &&
		  stack_top_token_code(s) != BRACKET_START)
		ll_tail_insert(postfix, stack_pop(s));

	    (void) stack_pop(s);

	    while(!stack_is_empty(s)){
		if (is_unary_operator(stack_top_token_code(s))){
		    ll_tail_insert(postfix, stack_pop(s));
		    continue;
		}
		break;
	    }
	}else if (curr->token_code == COMMA){
	    while(!stack_is_empty(s) &&
		  stack_top_token_code(s) != BRACKET_START)
		ll_tail_insert(postfix, stack_pop(s));
	}
    }

    while(!stack_is_empty(s))
	ll_tail_insert(postfix, stack_pop(s));

    /* print_postfix_list(postfix); */

    return postfix;
}
