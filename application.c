#include <stdio.h>
#include <stdlib.h>
#include "MexprTree.h"
#include "ExportedParser.h"

/*
 * The array of test strings must end with NULL.
 *
 * Iterate each string and pass it to the parser. Then,
 * check whether the parser returned the expected boolean
 * result or not.
 */
static void
app_parser_test(bool (*parser)(void), char **targets, bool expected_result){
    int iter = 0;

    while (targets[iter]){

	/* Prepare the test */
	init_buffer(targets[iter]);

	/* Parse the string */
	assert(parser() == expected_result);

	/* Move to the next string */
	iter++;
    }
}

/*
 * Convert 'target' into postfix notation and
 * compare it with the 'answer'.
 */
static void
app_converter_test(bool (*parser)(void), char *target,
		   char **answer, int answer_length){
    int i;
    linked_list *postfix_array;
    node *lln;
    lex_data *curr;

    init_buffer(target);

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
app_evaluation_test(bool (*parser)(void), char **targets){
    linked_list *postfix_array;
    tree *t;
    int iter = 0;

    while(targets[iter]){
	/* Prepare the test */
	init_buffer(targets[iter]);

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

void
app_resolve_and_evaluate_test(bool (*parser)(void), char *target,
			      void *app_data_src, tr_node *(*app_access_cb)(char *, void *)){
    tree *t;
    linked_list *postfix;
    bool parse_ret = false, resolve_failed = false;

    /* Prepare the test */
    init_buffer(target);

    parse_ret = parser();
    if (parse_ret != true){
	printf("'%s' was not correctly parsed\n", target);
	exit(-1);
    }

    /* Convert parse array into postfix notation */
    postfix = convert_infix_to_postfix(lstack.main_data,
				       lstack.stack_pointer);

    /* Convert the postfix notation into tree */
    t = convert_postfix_to_tree(postfix);

    /* Resolve variable if any */
    resolve_variable(t, app_data_src, app_access_cb, &resolve_failed);

    /* Evaluate the tree */
    if (!resolve_failed)
	evaluate_tree(t);
    else{
	printf("detected invalid tree node from application callback\n");
	exit(-1);
    }
}

static void
app_math_parser_tests(void){
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

    /* Postfix notation */
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

    app_parser_test(start_mathexpr_parse, success, true);
    app_parser_test(start_mathexpr_parse, failure, false);

    app_converter_test(start_mathexpr_parse, conversion_test1,
		   answer1, 3);
    app_converter_test(start_mathexpr_parse, conversion_test2,
		   answer2, 11);
    app_converter_test(start_mathexpr_parse, conversion_test3,
		   answer3, 17);

    app_evaluation_test(start_mathexpr_parse, eval_tests);
}

static void
app_ineq_parser_tests(void){
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

    app_parser_test(start_ineq_mathexpr_parse, ineq_success, true);
    app_parser_test(start_ineq_mathexpr_parse, ineq_failure, false);
}

static void
app_logical_parser_tests(void){
    char *logical_success[] = {
	"1 <= 2\n",
	"3.0 <= 5.0\n",
	"1 <= 2 and 3.0 <= 5.0\n",
	"2 = 2\n",
    };

    app_resolve_and_evaluate_test(start_ineq_mathexpr_parse,
				  logical_success[0], NULL, NULL);
    app_resolve_and_evaluate_test(start_ineq_mathexpr_parse,
				  logical_success[1], NULL, NULL);
    app_resolve_and_evaluate_test(start_logical_mathexpr_parse,
				  logical_success[2], NULL, NULL);
    app_resolve_and_evaluate_test(start_ineq_mathexpr_parse,
				  logical_success[3], NULL, NULL);
}

/* Application data definition */
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
app_fetch_data(char *s, void *data){
    app_data *ary = (app_data *) data;
    tr_node *trn = gen_null_tr_node();

    if (strncmp(s, "a", strlen("a")) == 0){
	trn->node_id = INT;
	trn->unv.ival = strtol(ary[0].val, (char **) NULL, 10);
    }else if (strncmp(s, "b", strlen("b")) == 0){
	trn->node_id = DOUBLE;
	trn->unv.dval = strtod(ary[1].val, (char **) NULL);
    }else if (strncmp(s, "c", strlen("c")) == 0){
	trn->node_id = INT;
	trn->unv.ival = strtol(ary[2].val, (char **) NULL, 10);
    }else if (strncmp(s, "d", strlen("d")) == 0){
	trn->node_id = INT;
	trn->unv.ival = strtol(ary[3].val, (char **) NULL, 10);
    }else{
	/* Application should not pass any other name of data */
	assert(0);
    }

    return trn;
}

int
main(int argc, char **argv){
    /* Math expression */
    app_math_parser_tests();
    /* Inequality expression */
    app_ineq_parser_tests();
    /* Logical expression (Partly supported) */
    app_logical_parser_tests();

    app_resolve_and_evaluate_test(start_mathexpr_parse,
				  "((1 + 2) - 3) * 4 / (5 / 6) + (7 - 8) \n",
				  NULL, NULL);
    app_resolve_and_evaluate_test(start_mathexpr_parse,
				  "sqr(3) + min(10, 0) + sqrt(25.0)\n",
				  NULL, NULL);
    app_resolve_and_evaluate_test(start_mathexpr_parse,
				  "a\n", app_array, app_fetch_data);
    app_resolve_and_evaluate_test(start_mathexpr_parse,
				  "b\n", app_array, app_fetch_data);
    app_resolve_and_evaluate_test(start_mathexpr_parse,
				  "c\n", app_array, app_fetch_data);
    app_resolve_and_evaluate_test(start_mathexpr_parse,
				  "d\n", app_array, app_fetch_data);
    app_resolve_and_evaluate_test(start_ineq_mathexpr_parse,
				  "a <= 100\n", app_array, app_fetch_data);
    app_resolve_and_evaluate_test(start_ineq_mathexpr_parse,
				  "b <= c\n", app_array, app_fetch_data);

    return 0;
}
