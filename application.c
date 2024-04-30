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
 * Convert 'target' into postfix notation and compare
 * it with the 'answer'.
 */
static void
app_converter_test(bool (*parser)(void), char *target,
		   char **answer, int answer_length){
    int i;
    linked_list *postfix;
    lex_data *curr;

    init_buffer(target);

    assert(parser() == true);

    postfix = convert_infix_to_postfix(lstack.main_data,
				       lstack.stack_pointer);

    assert(ll_get_length(postfix) == answer_length);

    for (i = 0; i < answer_length; i++){
	curr = (lex_data *) ll_get_first_node(postfix);

	if (strncmp(curr->token_val, answer[i], strlen(answer[i])) != 0){
	    printf("index = %d : expected the postfix string = '%s', but it was '%s'\n",
		   i, answer[i], curr->token_val);
	    exit(-1);
	}
    }
}

static void
app_evaluation_test(bool (*parser)(void), char *target,
		    int expected_type, node_value expected_value){
    linked_list *postfix;
    tr_node top;
    tree *t;

    /* Prepare the test */
    init_buffer(target);

    /* Parse the string */
    assert(parser() == true);
    postfix = convert_infix_to_postfix(lstack.main_data,
				       lex_stack_pointer());
    /* Convert the postfix array to tree and evaluate it */
    t = convert_postfix_to_tree(postfix);
    evaluate_tree(t, &top);

    /* Compare the result with expected value */
    assert(top.node_id == expected_type);
    switch(top.node_id){
	case INT:
	    assert(top.unv.ival == expected_value.ival);
	    break;
	case DOUBLE:
	    assert(top.unv.dval == expected_value.dval);
	    break;
	case VARIABLE:
	    assert(0);
	    break;
	case BOOLEAN:
	    assert(top.unv.bval == expected_value.bval);
	    break;
	default:
	    break;
    }
}

void
app_resolve_and_evaluate_test(bool (*parser)(void), char *target,
			      void *app_data_src,
			      tr_node *(*app_access_cb)(char *, void *),
			      int expected_type, node_value expected_value){
    tree *t;
    linked_list *postfix;
    bool parse_ret = false;
    tr_node top;

    /* Prepare the test */
    init_buffer(target);

    parse_ret = parser();
    if (parse_ret != true){
	printf("'%s' was not correctly parsed\n", target);
	exit(-1);
    }

    /* Convert parse array into postfix notation */
    postfix = convert_infix_to_postfix(lstack.main_data,
				       lex_stack_pointer());

    /* Convert the postfix notation into tree */
    t = convert_postfix_to_tree(postfix);

    /* Resolve variable if any */
    resolve_variable(t, app_data_src, app_access_cb);

    /* Evaluate the tree */
    evaluate_tree(t, &top);

    /* Compare the result with expected value */
    if (top.node_id != expected_type){
	printf("target = '%s' didn't return the expected type of value\n", target);
	assert(0);
    }

    switch(top.node_id){
	case INT:
	    assert(top.unv.ival == expected_value.ival);
	    break;
	case DOUBLE:
	    assert(top.unv.dval == expected_value.dval);
	    break;
	case VARIABLE:
	    assert(0);
	    break;
	case BOOLEAN:
	    assert(top.unv.bval == expected_value.bval);
	    break;
	default:
	    break;
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

    node_value expected_val;

    app_parser_test(start_mathexpr_parse, success, true);
    app_parser_test(start_mathexpr_parse, failure, false);

    app_converter_test(start_mathexpr_parse, conversion_test1,
		   answer1, 3);
    app_converter_test(start_mathexpr_parse, conversion_test2,
		   answer2, 11);
    app_converter_test(start_mathexpr_parse, conversion_test3,
		   answer3, 17);

    /* single value */
    expected_val.ival = -5;
    app_evaluation_test(start_mathexpr_parse, "-5\n", INT, expected_val);
    expected_val.dval = 1.0;
    app_evaluation_test(start_mathexpr_parse, "1.0\n", DOUBLE, expected_val);

    /* unary operator */
    expected_val.dval = 0.0;
    app_evaluation_test(start_mathexpr_parse, "sin(0)\n", DOUBLE, expected_val);
    expected_val.dval = 1.0;
    app_evaluation_test(start_mathexpr_parse, "cos(0)\n", DOUBLE, expected_val);
    expected_val.dval = 4.0;
    app_evaluation_test(start_mathexpr_parse, "sqrt(16)\n", DOUBLE, expected_val);
    expected_val.ival = 100;
    app_evaluation_test(start_mathexpr_parse, "sqr(10)\n", INT, expected_val);

    /* binary operator */
    expected_val.dval = 15.0;
    app_evaluation_test(start_mathexpr_parse, "5.0 + 10\n", DOUBLE, expected_val);
    expected_val.ival = 2;
    app_evaluation_test(start_mathexpr_parse, "max(1, 2)\n", INT, expected_val);
    expected_val.ival = -1;
    app_evaluation_test(start_mathexpr_parse, "1 - 2\n", INT, expected_val);
    expected_val.ival = -10;
    app_evaluation_test(start_mathexpr_parse, "min(-10, 1)\n", INT, expected_val);
    expected_val.ival = 1;
    app_evaluation_test(start_mathexpr_parse, "min(1, 10)\n", INT, expected_val);
    expected_val.ival = 50;
    app_evaluation_test(start_mathexpr_parse, "100 / 2\n", INT, expected_val);
    expected_val.ival = -6;
    app_evaluation_test(start_mathexpr_parse, "min(1 * 2 * 3, 1 * 2 * 3 * -1)\n", INT, expected_val);
    expected_val.dval = 8.0;
    app_evaluation_test(start_mathexpr_parse, "pow(2, 3)\n", DOUBLE, expected_val);
    expected_val.dval = 2;
    app_evaluation_test(start_mathexpr_parse, "11.0 % 3\n", DOUBLE, expected_val);

    /* miscellaneous */
    expected_val.dval = 5.0;
    app_evaluation_test(start_mathexpr_parse, "min(1, 0) + sqrt(25)\n", DOUBLE, expected_val);
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
    node_value expected_val;
    char *logical_success[] = {
	"1 <= 2\n",
	"3.0 <= 5.0\n",
	"1 <= 2 and 3.0 <= 5.0\n",
	"2 = 2\n",
    };

    expected_val.bval = true;
    app_resolve_and_evaluate_test(start_ineq_mathexpr_parse,
				  logical_success[0], NULL, NULL,
				  BOOLEAN, expected_val);
    app_resolve_and_evaluate_test(start_ineq_mathexpr_parse,
				  logical_success[1], NULL, NULL,
				  BOOLEAN, expected_val);
    app_resolve_and_evaluate_test(start_logical_mathexpr_parse,
				  logical_success[2], NULL, NULL,
				  BOOLEAN, expected_val);
    app_resolve_and_evaluate_test(start_ineq_mathexpr_parse,
				  logical_success[3], NULL, NULL,
				  BOOLEAN, expected_val);
}

/* Application data definition */
typedef struct app_data {
    char *name;
    char *val;
} app_data;

/* Application data source */
app_data app_array[] = {
    { .name = "a", .val = "1" },
    { .name = "b", .val = "3.0" },
    { .name = "c", .val = "5" },
    { .name = "d", .val = "-1" },
};

/* Application callback */
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

void
app_var_resolve_tests(){
    node_value expected_val;

    expected_val.ival = -1;
    app_resolve_and_evaluate_test(start_mathexpr_parse,
				  "((7 - 8))\n",
				  NULL, NULL, INT, expected_val);
    expected_val.dval = 14.0;
    app_resolve_and_evaluate_test(start_mathexpr_parse,
				  "sqr(3) + min(10, 0) + sqrt(25.0)\n",
				  NULL, NULL, DOUBLE, expected_val);
    expected_val.ival = 1;
    app_resolve_and_evaluate_test(start_mathexpr_parse,
				  "a\n", app_array, app_fetch_data, INT, expected_val);
    expected_val.dval = 3.0;
    app_resolve_and_evaluate_test(start_mathexpr_parse,
				  "b\n", app_array, app_fetch_data, DOUBLE, expected_val);
    expected_val.ival = 5;
    app_resolve_and_evaluate_test(start_mathexpr_parse,
				  "c\n", app_array, app_fetch_data, INT, expected_val);
    expected_val.ival = -1;
    app_resolve_and_evaluate_test(start_mathexpr_parse,
				  "d\n", app_array, app_fetch_data, INT, expected_val);
    expected_val.bval = true;
    app_resolve_and_evaluate_test(start_ineq_mathexpr_parse,
				  "a <= 100\n", app_array, app_fetch_data,
				  BOOLEAN, expected_val);
    expected_val.bval = true;
    app_resolve_and_evaluate_test(start_ineq_mathexpr_parse,
				  "b <= c\n", app_array, app_fetch_data,
				  BOOLEAN, expected_val);
}

int
main(int argc, char **argv){

    /* Math expression */
    app_math_parser_tests();
    /* Inequality expression */
    app_ineq_parser_tests();
    /* Logical expression (Partly supported) */
    app_logical_parser_tests();
    /* Variable resolution */
    app_var_resolve_tests();

    return 0;
}
