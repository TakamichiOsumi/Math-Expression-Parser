
%{

#include <assert.h>
#include <stdbool.h>
#include "ExportedParser.h"
#include "PostfixConverter.h"
#include "MexprTree.h"

lex_stack lstack = { INVALID, {{ 0, 0, NULL}} };
char lex_buffer[BUFFER_LEN];
char *next_parse_pos = lex_buffer;

/* functions required for parse processing */
static void
lex_set_scan_buffer(const char *buffer){
    yy_scan_string(buffer);
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

/* Parse functions */
extern bool start_mathexpr_parse();
extern bool start_ineq_mathexpr_parse();

/* Convert parse array into postfix notation */
extern linked_list *convert_infix_to_postfix(lex_data *infix,
					     int size_in);
/* Convert the postfix notation into tree */
extern tree* convert_postfix_to_tree(linked_list *postfix_array);

/* Resolve variable if any */
void resolve_variable(tree *t, void *app_data_src,
		      tr_node *(* app_access_cb)(struct variable *, void *));
/* Evaluate the tree */
extern void evaluate_tree(tree *t);

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
 * Input string must end with a new line '\n'.
 */
static void
test_format_check(char *s){
    char last_char;

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
}

/*
 * Input strings as 'targets' whose last string should be NULL.
 *
 * Iterate each string and pass it to the parser. Then,
 * check whether the parser returned the expected boolean
 * result or not.
 *
 * Note : if one string of 'targets' ended without a new line,
 * then quit the test and exit for quick notification.
 */
static void
parser_test(bool (*parser)(void),
	    char **targets, bool expected_result){
    int iter = 0;

    while (targets[iter]){
	/* Clean up */
	parser_stack_reset();
	memset(lex_buffer, '\0', BUFFER_LEN);

	/* Pretest format check */
	test_format_check(targets[iter]);

	/* Copy the test string to the buffer */
	strncpy(lex_buffer, targets[iter], strlen(targets[iter]));
	lex_set_scan_buffer(lex_buffer);

	/* Parse the string */
	assert(parser() == expected_result);

	/* Move to the next string */
	iter++;
    }
}

static void
converter_test(bool (*parser)(void),
	       char *target, char **answer, int answer_length){
    int i;
    linked_list *postfix_array;
    node *lln;
    lex_data *curr;

    /* Clean up */
    parser_stack_reset();

    /* Pretest format check */
    test_format_check(target);

    /* Copy the test string to the buffer */
    memset(lex_buffer, '\0', BUFFER_LEN);
    strncpy(lex_buffer, target, strlen(target));
    lex_set_scan_buffer(lex_buffer);

    assert(parser() == true);

    postfix_array = convert_infix_to_postfix(lstack.main_data,
					     lstack.stack_pointer);
    assert(ll_get_length(postfix_array) == answer_length);

    for (i = 0; i < answer_length; i++){
	lln = ll_get_first_node(postfix_array);
	curr = (lex_data *) lln->data;
	if (strncmp(curr->token_val, answer[i], strlen(answer[i])) != 0){
	    printf("index = %d : expected the postfix string = '%s', but it was '%s'\n",
		   i, answer[i], curr->token_val);
	    exit(-1);
	}
    }
}

static void
evaluation_test(bool (*parser)(void), char **targets){
    linked_list *postfix_array;
    tree *t;
    int iter = 0;

    while(targets[iter]){
	/* Clean up */
	parser_stack_reset();

	/* Pretest format check */
	test_format_check(targets[iter]);

	/* Copy the test string to the buffer */
	memset(lex_buffer, '\0', BUFFER_LEN);
	strncpy(lex_buffer, targets[iter], strlen(targets[iter]));
	lex_set_scan_buffer(lex_buffer);

	/* Parse the string */
	assert(parser() == true);
	postfix_array = convert_infix_to_postfix(lstack.main_data,
						 lstack.stack_pointer);
	/* Convert the postfix array to tree and evaluate it */
	t = convert_postfix_to_tree(postfix_array);
	evaluate_tree(t);

	/* Move to the next string */
	iter++;
    }
}

/* application data definition */
typedef struct app_data {
    char *name;
    char *val;
} app_data;

app_data app_array[] = {
    { .name = "a", .val = "1" },
    { .name = "b", .val = "3.0" },
    { .name = "c", .val = "5" },
    { .name = "d", .val = "-1" },
};

static tr_node *
app_fetch_data(variable *v, void *data){
    app_data *ary = (app_data *) data;
    tr_node *trn = gen_null_tr_node();

    if (strncmp(v->vname, "a", strlen("a")) == 0){
	trn->node_id = INT;
	trn->unv.ival = strtol(ary[0].val, (char **) NULL, 10);
	v->is_resolved = true;
	v->vdata = trn;
    }else if (strncmp(v->vname, "b", strlen("b")) == 0){
	trn->node_id = DOUBLE;
	trn->unv.dval = strtod(ary[1].val, (char **) NULL);
	v->is_resolved = true;
	v->vdata = trn;
    }else if (strncmp(v->vname, "c", strlen("c")) == 0){
	trn->node_id = INT;
	trn->unv.ival = strtol(ary[2].val, (char **) NULL, 10);
	v->is_resolved = true;
	v->vdata = trn;
    }else if (strncmp(v->vname, "d", strlen("d")) == 0){
	trn->node_id = INT;
	trn->unv.ival = strtol(ary[3].val, (char **) NULL, 10);
	v->is_resolved = true;
	v->vdata = trn;
    }else{
	/* Application should not pass any other name of data */
	assert(0);
    }

    return trn;
}

static void
resolve_and_evaluate_test(bool (*parser)(void), char *target){
    tree *t;
    linked_list *postfix;

    parser_stack_reset();

    test_format_check(target);
    memset(lex_buffer, '\0', BUFFER_LEN);
    strncpy(lex_buffer, target, strlen(target));
    lex_set_scan_buffer(lex_buffer);

    assert(parser() == true);

    /* Convert parse array into postfix notation */
    postfix = convert_infix_to_postfix(lstack.main_data,
				       lstack.stack_pointer);

    /* Convert the postfix notation into tree */
    t = convert_postfix_to_tree(postfix);

    /* Resolve variable if any */
    resolve_variable(t, app_array, app_fetch_data);

    /* Evaluate the tree */
    evaluate_tree(t);
}

static void
math_parser_tests(void){
    char *success[] = {
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

    char *failure[] = {
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

    char *conversion_test1 = "max(a, b)\n",
	*conversion_test2 = "a + sqr(b) * sqrt(c) + pow(d, e)\n",
	*conversion_test3 = "((-1 * b) + sqrt(sqr(b) - 4 * a * c)) / (2 * a)\n";

    char *answer1[] = { "a", "b", "max" },
	*answer2[] = { "a", "b", "sqr", "c", "sqrt",
		       "*", "+", "d", "e", "pow", "+" },
	*answer3[] = { "-1", "b", "*", "b", "sqr", "4", "a",
		       "*", "c", "*", "-", "sqrt", "+", "2", "a", "*", "/" };

    char *eval_tests[] = {
	"max(1, 2)\n",
	"5.0 + 10\n",
	"10 + 5.0\n",
	"1 - 2\n",
	"10 - 2.0\n",
	"min(-10, 1)\n",
	"min(1, 10)\n",
	"-5\n",
	"sqrt(16)\n",
	"sqr(10)\n",
	"sqr(3) + min(10, 0) + sqrt(25.0)\n",
	"25 % 7\n",
	"100 / 2\n",
	NULL,
    };

    parser_test(start_mathexpr_parse, success, true);
    parser_test(start_mathexpr_parse, failure, false);

    converter_test(start_mathexpr_parse, conversion_test1,
		   answer1, 3);
    converter_test(start_mathexpr_parse, conversion_test2,
		   answer2, 11);
    converter_test(start_mathexpr_parse, conversion_test3,
		   answer3, 17);
    evaluation_test(start_mathexpr_parse, eval_tests);

    resolve_and_evaluate_test(start_mathexpr_parse, "1 + a\n");
    resolve_and_evaluate_test(start_mathexpr_parse, "b * c * d\n");
    resolve_and_evaluate_test(start_mathexpr_parse, "d\n");
    resolve_and_evaluate_test(start_mathexpr_parse, "min(b, d)\n");
    resolve_and_evaluate_test(start_mathexpr_parse, "max(b, d)\n");
}

static void
ineq_parser_tests(void){
    char *ineq_success[] = {
	"1 < 2\n",
	"1 > 2\n",
	"a > (1 + 10)\n",
	"(a - b) <= (c + d + e + (f / g))\n",
	"(a - (b * c)) >= (d + e)\n",
	"a - b = c - d\n",
	"( 5 ) >= ( 10 )\n",
	NULL,
    };

    char *ineq_failure[] = {
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

    parser_test(start_ineq_mathexpr_parse, ineq_success, true);
    parser_test(start_ineq_mathexpr_parse, ineq_failure, false);
}

/* TODO */
static void
logical_parser_tests(void){}

static void
execute_full_parser_tests(){
    /* Math expression */
    math_parser_tests();
    /* Inequality expression */
    ineq_parser_tests();
    /* Logical expression */
    logical_parser_tests();
}

int
main(int argc, char **argv){

    execute_full_parser_tests();

    printf("All tests are done gracefully\n");

    return 0;
}
