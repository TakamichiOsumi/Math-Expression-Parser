
%{

#include <assert.h>
#include <stdbool.h>
#include "ExportedParser.h"
#include "PostfixConverter.h"
#include "MexprTree.h"

lex_stack lstack = { INVALID, { 0, 0, NULL} };
char lex_buffer[BUFFER_LEN];
char *next_parse_pos = lex_buffer;

extern void
lex_set_scan_buffer(const char *buffer){
    yy_scan_string(buffer);
}

static void
print_lex_data_in_stack(lex_data data, char *prefix, int index){
    if (STRING_IS_NEWLINE(data.token_val)){
	printf("%s : token = '\n' whose code is '%s' at index = %d\n",
	       prefix,
	       get_string_token(data.token_code),
	       index);
    }else{
	printf("%s : token = '%s' whose code is '%s' at index = %d\n",
	       prefix,
	       data.token_val,
	       get_string_token(data.token_code),
	       index);
    }
}

/* Stack operations */
static void
lex_push(lex_data data){
    assert(lstack.stack_pointer >= 0);

    if (lstack.stack_pointer > MAX_STACK_INDEX - 1){
	printf("The stack pointer is bigger than the upper limit = %d\n",
	       lstack.stack_pointer);
	return;
    }

    /* print_lex_data_in_stack(data, "\tPush", lstack.stack_pointer - 1); */

    lstack.main_data[lstack.stack_pointer++] = data;
}

static lex_data
lex_pop(){
    lex_data ldata = { INVALID, 0, NULL };
    lex_data *ldp;

    assert(lstack.stack_pointer >= 0);

    /*
    print_lex_data_in_stack(lstack.main_data[lstack.stack_pointer - 1],
                            "\tPop ",
                            lstack.stack_pointer - 1); */

    lstack.stack_pointer--;

    return lstack.main_data[lstack.stack_pointer];
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

/* Dump any useful information for debugging */
static void
debug_print(void){
    int i;

    printf("-----\n");
    for (i = 0; i < lstack.stack_pointer; i++){
	print_lex_data_in_stack(lstack.main_data[i], "debug",
				lstack.stack_pointer - 1);
    }
    printf("-----\n");
}

extern void
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

"!="             { return NEQ;       }

">="             { return GREATER_THAN_OR_EQUAL_TO; }

"<="             { return LESS_THAN_OR_EQUAL_TO;    }

"<"              { return LESS_THAN;    }

">"              { return GREATER_THAN; }

"="              { return EQ;        }

0|-?[1-9][0-9]*  { return INT;       }

-?[0-9]*\.[0-9]+ { return DOUBLE;    }

[a-zA-Z1-9_]+    { return VARIABLE;  }

[ ]+             { process_white_space_or_tab(WHITE_SPACE, yyleng); }

[\t]+            { process_white_space_or_tab(TAB, yyleng);         }

.                { printf("detected invalid input '%s'\n", yytext); }

%%

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
 * Input string must end with a new line '\n'.
 */
static void
test_format_check(char *s){
    char last_char;

    if (strlen(s) > 2){
	last_char = s[strlen(s) - 1];
	if (last_char != '\n'){
	    printf("invalid string format : '%s'\n", s);
	    exit(-1);
	}
    }
}

extern bool start_mathexpr_parse();
extern bool start_ineq_mathexpr_parse();
tree* convert_postfix_to_tree(linked_list *postfix_array);
typedef bool (*parser_func)(void);

/*
 * Input strings as 'tested' whose last string is null.
 *
 * Iterate each string and pass it to the parser. Then,
 * check whether the parser returned the expected boolean
 * result or not.
 *
 * Note : if one string of 'tested' ended without a new line,
 * then, quit the test and exit for quick notification.
 */
static void
parser_test(parser_func parser,
	    char **tested, bool expected_result){
    char last_char;
    int iter = 0;

    while (tested[iter]){
	/* Clean up */
	parser_stack_reset();
	memset(lex_buffer, '\0', BUFFER_LEN);

	/* Pretest format check */
	test_format_check(tested[iter]);

	/* Copy the test string to the buffer */
	strncpy(lex_buffer, tested[iter], strlen(tested[iter]));
	lex_set_scan_buffer(lex_buffer);

	if (parser() == expected_result){
	    printf("expected parsing '%s' to return %s and it did\n",
		   tested[iter],
		   expected_result ? "true" : "false");
	}else{
	    printf("'%s' was not parsed expectedly, it was not '%s'\n",
		   tested[iter],
		   expected_result ? "Accepted" : "Rejected");

	    exit(-1);
	}
	/* debug_print(); */

	/* Move to the next string */
	iter++;
    }
}

static void
converter_test(parser_func parser, char *tested, char **answer, int answer_length){
    int i;
    linked_list *postfix_array;
    node *lln;
    lex_data *curr;

    /* Clean up */
    parser_stack_reset();

    /* Pretest format check */
    test_format_check(tested);

    /* Copy the test string to the buffer */
    memset(lex_buffer, '\0', BUFFER_LEN);
    strncpy(lex_buffer, tested, strlen(tested));
    lex_set_scan_buffer(lex_buffer);

    assert(parser() == true);

    postfix_array = convert_infix_to_postfix(lstack.main_data,
					     lstack.stack_pointer);
    assert(ll_get_length(postfix_array) == answer_length);

    for (i = 0; i < answer_length; i++){
	lln = ll_get_first_node(postfix_array);
	curr = (lex_data *) lln->data;
	if (strncmp(curr->token_val, answer[i], strlen(answer[i])) != 0){
	    printf("index = %d : expected the postfix string = %s, but it was = %s\n",
		   i, answer[i], curr->token_val);
	    exit(-1);
	}
    }
    /* t = convert_postfix_to_tree(postfix_array); */
}

extern void evaluate_tree(tree *t);

static void
evaluation_test(parser_func parser, char *tested){
    linked_list *postfix_array;
    node *lln;
    lex_data *curr;
    tree *t;

    /* Clean up */
    parser_stack_reset();

    /* Pretest format check */
    test_format_check(tested);

    /* Copy the test string to the buffer */
    memset(lex_buffer, '\0', BUFFER_LEN);
    strncpy(lex_buffer, tested, strlen(tested));
    lex_set_scan_buffer(lex_buffer);

    /* Parse the string */
    assert(parser() == true);
    postfix_array = convert_infix_to_postfix(lstack.main_data,
					     lstack.stack_pointer);

    /* Convert the postfix array to tree and evaluate it */
    t = convert_postfix_to_tree(postfix_array);
    evaluate_tree(t);
}

extern linked_list *convert_infix_to_postfix(lex_data *infix,
					     int size_in);

int
main(int argc, char **argv){
    char *success[BUFFER_LEN] = {
	/* single token */
	"1\n",
	"-2\n",
	"a\n",
	/* plus/minus */
	"1 + 2\n",
	"-1 - -2\n",
	"1 + 2 + 3\n",
	"a + 2 + 3\n",
	"a - 2 - 3\n",
	/* multiply/divide */
	"1 * 2\n",
	"1 * 2 * 3\n",
	"a * 2 * 3\n",
	"a / 2 / 3\n",
	/* brachet */
	"( 1 + 2 ) \n",
	"( 1 + 2 ) * (3 * 4) \n",
	"( 1 - 2 ) + (3 * 4) \n",
	"( 3 ) + ( -10 )\n",
	"((1 + 2) - 3) * 4 / (5 / 6) + (7 - 8) \n",
	"( 1 + 2 * (3 - 4 ) ) / 5 - 6\n",
	/* miscellaneous */
	"      a   -    b      \n",
	"max(a, b) \n",
	"a + sqrt(b) * sqrt(c) + pow(d, e)\n",
	"((-1 * b) + sqrt(sqr(b) - 4 * a * c)) / (2 * a)\n",
	NULL,
    };

    char *failure[BUFFER_LEN] = {
	/* plus/minus */
	"+\n",
	"- 1\n",
	"1 + 2 + \n",
	"a - 2 - \n",
	/* multiply/divide */
	"*\n",
	"/ 1\n",
	"1 * 2 *\n",
	"1 * 2 /\n",
	"a * 2 /\n",
	"* a / 2 /\n",
	/* brachet */
	"(\n",
	"( )\n",
	"( 1 + 2\n",
	"( 1 + 2 * (3 * 4) \n",
	"3 * 4 / (5 / 6) + (7 - 8) )\n",
	/* miscellaneous */
	"1 1\n",
	"+ *\n",
	NULL,
    };

    char *ineq_success[BUFFER_LEN] = {
	"1 < 2\n",
	"1 > 2\n",
	"a > (1 + 10)\n",
	"(a - b) <= (c + d + e + (f / g))\n",
	"(a - (b * c)) >= (d + e)\n",
	"a - b = c - d\n",
	"( 5 ) >= ( 10 )\n",
	NULL,
    };

    char *ineq_failure[BUFFER_LEN] = {
	">\n"
	">=\n",
	"< 1\n",
	"() < ( )\n",
	"( + ) < ()\n",
	"( * ) < ( - )\n",
	"< <\n",
	"1 < 2 3\n",
	"1 < < 2\n",
	"1 < < 2 3\n",
	"* < 1\n",
	"* 1 < 2\n",
	NULL,
    };

    char
	*conversion_test1 = "max(a, b)\n",
	*conversion_test2 = "a + sqr(b) * sqrt(c) + pow(d, e)\n",
	*conversion_test3 = "((-1 * b) + sqrt(sqr(b) - 4 * a * c)) / (2 * a)\n";

    char
	*answer1[BUFFER_LEN] = { "a", "b", "max" },
	*answer2[BUFFER_LEN] = { "a", "b", "sqr", "c", "sqrt",
				 "*", "+", "d", "e", "pow", "+" },
	*answer3[BUFFER_LEN] = { "-1", "b", "*", "b", "sqr", "4", "a",
				 "*", "c", "*", "-", "sqrt", "+", "2", "a", "*", "/" };
    char
	*eval_test1 = "max(1, 2)\n",
	*eval_test2 = "5.0 + 10\n",
	*eval_test3 = "10 + 5.0\n",
	*eval_test4 = "1 - 2\n",
	*eval_test5 = "10 - 2.0\n",
	*eval_test6 = "min(-10, 1)\n",
	*eval_test7 = "min(1, 10)\n",
	*eval_test8 = "-5\n",
	*eval_test9 = "sqrt(16)\n",
	*eval_test10 = "sqr(10)\n";

    parser_test(start_mathexpr_parse, success, true);
    parser_test(start_mathexpr_parse, failure, false);
    parser_test(start_ineq_mathexpr_parse, ineq_success, true);
    parser_test(start_ineq_mathexpr_parse, ineq_failure, false);

    converter_test(start_mathexpr_parse, conversion_test1, answer1, 3);
    converter_test(start_mathexpr_parse, conversion_test2, answer2, 11);
    converter_test(start_mathexpr_parse, conversion_test3, answer3, 17);

    evaluation_test(start_mathexpr_parse, eval_test1);
    evaluation_test(start_mathexpr_parse, eval_test2);
    evaluation_test(start_mathexpr_parse, eval_test3);
    evaluation_test(start_mathexpr_parse, eval_test4);
    evaluation_test(start_mathexpr_parse, eval_test5);
    evaluation_test(start_mathexpr_parse, eval_test6);
    evaluation_test(start_mathexpr_parse, eval_test7);
    evaluation_test(start_mathexpr_parse, eval_test8);
    evaluation_test(start_mathexpr_parse, eval_test9);
    evaluation_test(start_mathexpr_parse, eval_test10);

    printf("All tests are done gracefully\n");

    return 0;
}
