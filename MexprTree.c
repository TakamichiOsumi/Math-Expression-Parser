#include <assert.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include "MexprEnums.h"
#include "Linked-List/linked_list.h"
#include "Stack/stack.h"
#include "ExportedParser.h"
#include "MexprTree.h"

/*
 * To free all memory allocated during computation of tree,
 * keep the tree as it is, and connect all the temporary
 * one-shot computation results in a linked list.
 *
 * After the root node returns the final computation result,
 * free the memory with free callback.
 */
static linked_list *ll_tmp_calc;

tr_node *evaluate_node(tr_node *self, tree *t);

/* free callback */
static void
free_dynamic_calculated_node(void *p){
    tr_node *n = (tr_node *) p;

    switch(n->node_id){
	case INT:
	    break;
	case DOUBLE:
	    break;
	case VARIABLE:
	    break;
	default:
	    assert(0);
    }
}

static tree*
gen_tree(void){
    tree *t;

    t = (tree *) malloc(sizeof(tree));
    if (t == NULL){
	perror("malloc");
	exit(-1);
    }

    t->root = t->list_head = NULL;
    t->require_resolution = t->resolved = false;

    return t;
}

/*
 * Exported so that the application data can create tr_node * value easily.
 */
tr_node*
gen_null_tr_node(void){
    tr_node *n;

    if ((n = (tr_node *) malloc(sizeof(tr_node))) == NULL){
	perror("malloc");
	exit(-1);
    }

    n->parent = n->left = n->right
	= n->list_left = n->list_right = NULL;

    return n;
}

/*
 * Expect the caller passes each token processed by postfix converter.
 *
 * Therefore, some of token types must be filtered. Raise an assertion
 * failure if input of this function hits any type of them.
 */
static tr_node*
gen_tr_node_from_lex_data(lex_data *ld){
    tr_node *n = gen_null_tr_node();

    switch (ld->token_code){

	/* Whitespace symbol */
	case INVALID:
	case WHITE_SPACE:
	case TAB:
	case PARSER_EOF:
	    assert(0);
	    break;

	/* Punctuation marker */
	case BRACKET_START:
	case BRACKET_END:
	case COMMA:
	    assert(0);
	    break;

	/* Binary operator */
	case PLUS:
	case MINUS:
	case MULTIPLY:
	case DIVIDE:
	case MOD:
	case MIN:
	case MAX:
	case POW:
	    n->node_id = ld->token_code;
	    n->unv.operator = ld->token_val;
	    break;

	/* Inequality operator */
	case GREATER_THAN_OR_EQUAL_TO:
	case LESS_THAN_OR_EQUAL_TO:
	case GREATER_THAN:
	case LESS_THAN:
	case NEQ:
	case EQ:
	case AND:
	case OR:
	    n->node_id = ld->token_code;
	    n->unv.operator = ld->token_val;
	    break;

	/* Logical operator */
	case BOOLEAN:
	    n->node_id = ld->token_code;
	    n->unv.bval = (strncmp(ld->token_val, "TRUE",
				   strlen("TRUE")) == 0) ? true : false;
	    break;

	/* Unary operator */
	case SIN:
	case COS:
	case SQR:
	case SQRT:
	    n->node_id = ld->token_code;
	    n->unv.operator = ld->token_val;
	    break;

	/*
	 * Data Type
	 *
	 * All data is string. See cyylex().
	 */
	case INT:
	    assert(ld->token_val != NULL);
	    n->node_id = ld->token_code;
	    n->unv.ival = strtol(ld->token_val, (char **) NULL, 10);
	    break;
	case DOUBLE:
	    assert(ld->token_val != NULL);
	    n->node_id = ld->token_code;
	    n->unv.dval = strtod(ld->token_val, (char **) NULL);
	    break;
	case VARIABLE:
	    assert(ld->token_val != NULL);
	    /*
	     * We are making a tree from postfix notation now.
	     * So, initializing the tr_node by below values is fine.
	     * The resolution will be done against the tree after
	     * we build it.
	     */
	    n->node_id = ld->token_code;
	    n->unv.vval.vname = ld->token_val;
	    n->unv.vval.is_resolved = false;
	    n->unv.vval.vdata = NULL;
	    break;
	default:
	    assert(0);
	    break;
    }

    return n;
}

/*
 * For variable resolution, create a doubly linked list
 * with tree's 'list_left' and 'list_right' variables.
 */
tree*
convert_postfix_to_tree(linked_list *postfix){
    stack *node_stack = stack_init(ll_get_length(postfix));
    lex_data *curr;
    tr_node *trn, *prev;
    tree *t = gen_tree();

    ll_begin_iter(postfix);

    while((curr = (lex_data *) ll_get_iter_node(postfix)) != NULL){
	trn = gen_tr_node_from_lex_data(curr);

	if (is_operand(curr->token_code)){
	    stack_push(node_stack, trn);

	    /* Construct the dll by leaf nodes */
	    if (t->list_head == NULL){
		prev = t->list_head = trn;
	    }else{
		assert(prev != NULL);
		trn->list_left = prev;
		prev->list_right = trn;
		prev = trn;
	    }

	    /* Lastly, does this tree need the resolution ? */
	    if (curr->token_code == VARIABLE)
		t->require_resolution = true;

	}else if (is_unary_operator(curr->token_code)){

	    trn->left = stack_pop(node_stack);
	    trn->left->parent = trn;
	    stack_push(node_stack, trn);

	}else if (is_binary_operator(curr->token_code)){

	    trn->right = stack_pop(node_stack);
	    trn->left = stack_pop(node_stack);
	    trn->right->parent = trn;
	    trn->left->parent = trn;
	    stack_push(node_stack, trn);

	}
    }

    ll_end_iter(postfix);

    assert((t->root = stack_pop(node_stack)) != NULL);
    assert(t->root->parent == NULL);

    stack_destroy(node_stack);

    return t;
}

/*
 * Copy the calculation result (without pointers) to 'top'
 * argument.
 *
 * API user can decide dynamic allocated memory or just local
 * variable, etc for the computation result. All of calculated
 * 'tr_node's get freed internally without user intervention.
 *
 * Caller needs to check the tree's 'computation_failed' flag
 * before it accesses to the 'top' variable.
 */
void
evaluate_tree(tree *t, tr_node *top){
    tr_node *result;

    if (t->require_resolution && !t->resolved){
	printf("variable included in expression but not resolved\n");
	return;
    }

    ll_tmp_calc = ll_init(NULL,
			  free_dynamic_calculated_node);

    top->parent = top->left = top->right = top->list_left
	= top->list_right = NULL;

    result = evaluate_node(t->root, t);

    /* Calculation failed. Just return */
    if (t->computation_failed == true){
	printf("calculation failure\n");
	return;
    }

    switch(result->node_id){
	case INT:
	    top->node_id = INT;
	    top->unv.ival = result->unv.ival;
	    break;
	case DOUBLE:
	    top->node_id = DOUBLE;
	    top->unv.dval = result->unv.dval;
	    break;
	case VARIABLE:
	    printf("the return value of tree computation is invalid type\n");
	    assert(0);
	    break;
	case BOOLEAN:
	    top->node_id = BOOLEAN;
	    top->unv.bval = result->unv.bval;
	    break;
	default:
	    assert(0);
	    break;
    }

    /*
     * Free all computation results created during the
     * evaluate_node().
     */
    ll_destroy(ll_tmp_calc);
}

/*
 * There are needs to handle exit case, like zero division.
 *
 * As for the paths to access the VARIABLE node, there are
 * assert() statements. This is because the recursive call
 * of evaluate_node() returns concrete value types such as
 * INT and DOUBLE.
 *
 * Set 'true' to the original tree's 'computation_failed'
 * if calculation is not possible or failure.
 */
tr_node *
evaluate_node(tr_node *self, tree *t){
    tr_node *result, *left, *right;

    assert(self != NULL);
    assert(t != NULL);

    /* Reach the leaf node ? */
    if (self->left == NULL && self->right == NULL){
	if (self->node_id != VARIABLE)
	    return self;
	else{
	    /* VARIABLE */
	    variable *v;

	    v = &self->unv.vval;
	    assert(v != NULL);

	    return v->vdata;
	}
    }

    /* If not, execute the operator */
    if (is_unary_operator(self->node_id)){

	assert(self->left != NULL);
	assert(self->right == NULL);

	/* Create node and add it to the list */
	result = gen_null_tr_node();
	ll_insert(ll_tmp_calc, (void *) result);

	/* Get the result of left node */
	left = evaluate_node(self->left, t);

	if (t->computation_failed)
	    return result;

	assert(left != NULL);

	switch(left->node_id){
	    case INT:
		switch(self->node_id){
		    case SIN:
			result->node_id = DOUBLE;
			result->unv.dval = sin(left->unv.ival);
			break;
		    case COS:
			result->node_id = DOUBLE;
			result->unv.dval = cos(left->unv.ival);
			break;
		    case SQR:
			result->node_id = INT;
			result->unv.ival = left->unv.ival * left->unv.ival;
			break;
		    case SQRT:
			result->node_id = DOUBLE;
			result->unv.dval = sqrt(left->unv.ival);
			break;
		    default:
			assert(0);
			return NULL;
		}
		break;
	    case DOUBLE:
		switch(self->node_id){
		    case SIN:
			result->node_id = DOUBLE;
			result->unv.dval = sin(left->unv.dval);
			break;
		    case COS:
			result->node_id = DOUBLE;
			result->unv.dval = cos(left->unv.dval);
			break;
		    case SQR:
			result->node_id = DOUBLE;
			result->unv.dval = left->unv.dval * left->unv.dval;
			break;
		    case SQRT:
			result->node_id = DOUBLE;
			result->unv.dval = sqrt(left->unv.dval);
			break;
		    default:
			assert(0);
			return NULL;
		}
		break;
	    case VARIABLE:
		assert(0);
		break;
	    case BOOLEAN:
		/* Evaluation failure. Report an error */
		t->computation_failed = true;
		break;
	    default:
		assert(0);
		return NULL;
	}
    }else if (is_binary_operator(self->node_id)){
	assert(self->left != NULL);
	assert(self->right != NULL);

	result = gen_null_tr_node();
	left = evaluate_node(self->left, t);
	if (t->computation_failed)
	    return result;

	right = evaluate_node(self->right, t);
	if (t->computation_failed)
	    return result;

	assert(left != NULL);
	assert(right != NULL);

	switch(self->node_id){
	    case PLUS:
		switch(left->node_id){
		    case INT:
			/* INT + ? */
			switch(right->node_id){
			    case INT:
				result->node_id = INT;
				result->unv.ival = left->unv.ival + right->unv.ival;
				break;
			    case DOUBLE:
				result->node_id = DOUBLE;
				result->unv.dval = left->unv.ival + right->unv.dval;
				break;
			    case VARIABLE:
				assert(0);
				break;
			    case BOOLEAN:
				/* Evaluation failure */
				t->computation_failed = true;
				break;
			    default:
				assert(0);
				break;
			}
			break;
		    case DOUBLE:
			/* DOUBLE + ? */
			switch(right->node_id){
			    case INT:
				result->node_id = DOUBLE;
				result->unv.dval = left->unv.dval + (double) right->unv.ival;
				break;
			    case DOUBLE:
				result->node_id = DOUBLE;
				result->unv.dval = left->unv.dval + right->unv.dval;
				break;
			    case VARIABLE:
				assert(0);
				break;
			    case BOOLEAN:
				/* Evaluation failure */
				t->computation_failed = true;
				break;
			    default:
				assert(0);
				break;
			}
			break;
		    case VARIABLE:
			/*
			 * VARIABLE + ?
			 *
			 * VARIABLE should be resolved already and
			 * converted into different type here.
			 */
			switch(right->node_id){
			    case INT:
			    case DOUBLE:
			    case VARIABLE:
			    case BOOLEAN:
				assert(0);
				break;
			    default:
				assert(0);
				break;
			}
			break;
		    case BOOLEAN:
			/* BOOLEAN + ? */
			switch(right->node_id){
			    case INT:
			    case DOUBLE:
			    case VARIABLE:
			    case BOOLEAN:
				/* Evaluation failure */
				t->computation_failed = true;
				break;
			    default:
				assert(0);
				break;
			}
		    default:
			assert(0);
			break;
		}
		break;
	    case MINUS:
		switch(left->node_id){
		    case INT:
			/* INT - ? */
			switch(right->node_id){
			    case INT:
				result->node_id = INT;
				result->unv.ival = left->unv.ival - right->unv.ival;
				break;
			    case DOUBLE:
				result->node_id = DOUBLE;
				result->unv.dval = left->unv.ival - right->unv.dval;
				break;
			    case VARIABLE:
				assert(0);
				break;
			    case BOOLEAN:
				/* Evaluation failure */
				t->computation_failed = true;
				break;
			    default:
				assert(0);
				break;
			}
			break;
		    case DOUBLE:
			/* DOUBLE - ? */
			switch(right->node_id){
			    case INT:
				result->node_id = DOUBLE;
				result->unv.dval = left->unv.dval - right->unv.ival;
				break;
			    case DOUBLE:
				result->node_id = DOUBLE;
				result->unv.dval = left->unv.dval - right->unv.dval;
				break;
			    case VARIABLE:
				assert(0);
				break;
			    case BOOLEAN:
				/* Evaluation failure */
				t->computation_failed = true;
				break;
			    default:
				assert(0);
				break;
			}
			break;
		    case VARIABLE:
			/* VARIABLE - ? */
			switch(right->node_id){
			    case INT:
			    case DOUBLE:
			    case VARIABLE:
			    case BOOLEAN:
				assert(0);
				break;
			    default:
				assert(0);
				break;
			}
			break;
		    case BOOLEAN:
			/* BOOLEAN - ? */
			switch(right->node_id){
			    case INT:
			    case DOUBLE:
			    case VARIABLE:
			    case BOOLEAN:
				/* Evaluation failure */
				t->computation_failed = true;
				break;
			    default:
				break;
			}
			break;
		    default:
			break;
		}
		break;
	    case MULTIPLY:
		switch(left->node_id){
		    case INT:
			/* INT * ? */
			switch(right->node_id){
			    case INT:
				result->node_id = INT;
				result->unv.ival = left->unv.ival * right->unv.ival;
				break;
			    case DOUBLE:
				result->node_id = DOUBLE;
				result->unv.dval = left->unv.ival * right->unv.dval;
				break;
			    case VARIABLE:
				assert(0);
				break;
			    case BOOLEAN:
				/* Evaluation failure */
				t->computation_failed = true;
				break;
			    default:
				assert(0);
				break;
			}
			break;
		    case DOUBLE:
			/* DOUBLE * ? */
			switch(right->node_id){
			    case INT:
				result->node_id = DOUBLE;
				result->unv.dval = left->unv.dval * right->unv.ival;
				break;
			    case DOUBLE:
				result->node_id = DOUBLE;
				result->unv.dval = left->unv.dval * right->unv.dval;
				break;
			    case VARIABLE:
				assert(0);
				break;
			    case BOOLEAN:
				/* Evaluation failure */
				t->computation_failed = true;
				break;
			    default:
				assert(0);
				break;
			}
			break;
		    case VARIABLE:
			/* VARIABLE * ? */
			switch(right->node_id){
			    case INT:
			    case DOUBLE:
			    case VARIABLE:
			    case BOOLEAN:
				assert(0);
				break;
			    default:
				assert(0);
				break;
			}
			break;
		    case BOOLEAN:
			/* BOOLEAN * ? */
			switch(right->node_id){
			    case INT:
			    case DOUBLE:
			    case VARIABLE:
			    case BOOLEAN:
				/* Evaluation failure */
				t->computation_failed = true;
				break;
			    default:
				assert(0);
				break;
			}
		    default:
			assert(0);
			break;
		}
		break;
	    case DIVIDE:
		switch(left->node_id){
		    case INT:
			/* INT / ? */
			switch(right->node_id){
			    case INT:
				if (right->unv.ival != 0){
				    result->node_id = INT;
				    result->unv.ival = left->unv.ival / right->unv.ival;
				    break;
				}else{
				    t->computation_failed = true;
				    break;
				}
			    case DOUBLE:
				if (right->unv.dval != 0){
				    result->node_id = DOUBLE;
				    result->unv.dval = left->unv.ival / right->unv.dval;
				    break;
				}else{
				    t->computation_failed = true;
				    break;
				}
			    case VARIABLE:
				assert(0);
				break;
			    case BOOLEAN:
				/* Evaluation failure */
				t->computation_failed = true;
				break;
			    default:
				assert(0);
				break;
			}
			break;
		    case DOUBLE:
			/* DOUBLE / ? */
			switch(right->node_id){
			    case INT:
				if (right->unv.ival != 0){
				    result->node_id = DOUBLE;
				    result->unv.dval = left->unv.dval / right->unv.ival;
				    break;
				}else{
				    t->computation_failed = true;
				    break;
				}
			    case DOUBLE:
				if (right->unv.dval != 0.0){
				    result->node_id = DOUBLE;
				    result->unv.dval = left->unv.dval / right->unv.dval;
				    break;
				}else{
				    t->computation_failed = true;
				    break;
				}
			    case VARIABLE:
				assert(0);
				break;
			    case BOOLEAN:
				/* Evaluation failure */
				t->computation_failed = true;
				break;
			    default:
				assert(0);
				break;
			}
			break;
		    case VARIABLE:
			/* VARIABLE / ? */
			switch(right->node_id){
			    case INT:
			    case DOUBLE:
			    case VARIABLE:
			    case BOOLEAN:
				assert(0);
				break;
			    default:
				assert(0);
				break;
			}
		    case BOOLEAN:
			/* BOOLEAN / ? */
			switch(right->node_id){
			    case INT:
			    case DOUBLE:
			    case VARIABLE:
			    case BOOLEAN:
				/* Evaluation failure */
				t->computation_failed = true;
				break;
			    default:
				assert(0);
				break;
			}
			break;
		    default:
			assert(0);
			break;
		}
		break;
	    case MOD:
		switch(left->node_id){
		    case INT:
			/* INT % ? */
			switch(right->node_id){
			    case INT:
				if (right->unv.ival != 0){
				    result->node_id = INT;
				    result->unv.ival = left->unv.ival % right->unv.ival;
				    break;
				}else{
				    t->computation_failed = true;
				    break;
				}
				break;
			    case DOUBLE:
				if (right->unv.dval != 0){
				    result->node_id = DOUBLE;
				    result->unv.dval = fmod(left->unv.ival,
							    right->unv.dval);
				    break;
				}else{
				    t->computation_failed = true;
				    break;
				}
			    case VARIABLE:
				assert(0);
				break;
			    case BOOLEAN:
				/* Evaluation failure */
				t->computation_failed = true;
				break;
			    default:
				assert(0);
				break;
			}
			break;
		    case DOUBLE:
			/* DOUBLE % ? */
			switch(right->node_id){
			    case INT:
				if (right->unv.ival != 0){
				    result->node_id = DOUBLE;
				    result->unv.dval = fmod(left->unv.dval,
							    right->unv.ival);
				    break;
				}else{
				    t->computation_failed = true;
				    break;
				}
			    case DOUBLE:
				if (right->unv.dval != 0){
				    result->node_id = DOUBLE;
				    result->unv.dval = fmod(left->unv.dval,
							    right->unv.dval);
				    break;
				}else{
				    t->computation_failed = true;
				    break;
				}
			    case VARIABLE:
				assert(0);
				break;
			    case BOOLEAN:
				t->computation_failed = true;
				break;
			    default:
				assert(0);
				break;
			}
			break;
		    case VARIABLE:
			/* VARIABLE % ? */
			switch(right->node_id){
			    case INT:
			    case DOUBLE:
			    case VARIABLE:
			    case BOOLEAN:
				assert(0);
				break;
			    default:
				assert(0);
				break;
			}
			break;
		    case BOOLEAN:
			/* BOOLEAN % ? */
			switch(right->node_id){
			    case INT:
			    case DOUBLE:
			    case VARIABLE:
			    case BOOLEAN:
				/* Evaluation failure */
				t->computation_failed = true;
				break;
			    default:
				assert(0);
				break;
			}
		    default:
			assert(0);
			break;
		}
		break;
	    case MIN:
		switch(left->node_id){
		    case INT:
			/* min (INT, ? ) */
			switch(right->node_id){
			    case INT:
				result->node_id = INT;
				result->unv.ival = left->unv.ival < right->unv.ival ?
				    left->unv.ival : right->unv.ival;
				break;
			    case DOUBLE:
				if (left->unv.ival < right->unv.dval){
				    result->node_id = INT;
				    result->unv.ival = left->unv.ival;
				}else{
				    result->node_id = DOUBLE;
				    result->unv.dval = right->unv.dval;
				}
				break;
			    case VARIABLE:
				assert(0);
				break;
			    case BOOLEAN:
				/* Evaluation failure */
				t->computation_failed = true;
				break;
			    default:
				assert(0);
				break;
			}
			break;
		    case DOUBLE:
			/* min (DOUBLE, ?) */
			switch(right->node_id){
			    case INT:
				if (left->unv.dval < right->unv.ival){
				    result->node_id = DOUBLE;
				    result->unv.dval = left->unv.dval;
				}else{
				    result->node_id = INT;
				    result->unv.ival = right->unv.ival;
				}
				break;
			    case DOUBLE:
				result->node_id = DOUBLE;
				result->unv.dval = left->unv.dval < right->unv.dval ?
				    left->unv.dval : right->unv.dval;
				break;
			    case VARIABLE:
				assert(0);
				break;
			    case BOOLEAN:
				/* Evaluation failure */
				t->computation_failed = true;
				break;
			    default:
				assert(0);
				break;
			}
			break;
		    case VARIABLE:
			/* min (VARIABLE, ?) */
			switch(right->node_id){
			    case INT:
			    case DOUBLE:
			    case VARIABLE:
			    case BOOLEAN:
				assert(0);
				break;
			    default:
				assert(0);
				break;
			}
			break;
		    case BOOLEAN:
			/* min (BOOLEAN, ?) */
			switch(right->node_id){
			    case INT:
			    case DOUBLE:
			    case VARIABLE:
			    case BOOLEAN:
				/* Evaluation failure */
				t->computation_failed = true;
				break;
			    default:
				assert(0);
				break;
			}
		    default:
			assert(0);
			break;
		}
		break;
	    case MAX:
		switch(left->node_id){
		    case INT:
			/* max (INT, ?) */
			switch(right->node_id){
			    case INT:
				result->node_id = INT;
				result->unv.ival = left->unv.ival > right->unv.ival ?
				    left->unv.ival : right->unv.ival;
				break;
			    case DOUBLE:
				if (left->unv.ival > right->unv.dval){
				    result->node_id = INT;
				    result->unv.ival = left->unv.ival;
				}else{
				    result->node_id = DOUBLE;
				    result->unv.dval = right->unv.dval;
				}
				break;
			    case VARIABLE:
				assert(0);
				break;
			    case BOOLEAN:
				/* Evaluation failure */
				t->computation_failed = true;
				break;
			    default:
				assert(0);
				break;
			}
			break;
		    case DOUBLE:
			/* max (DOUBLE, ?) */
			switch(right->node_id){
			    case INT:
				if (left->unv.dval > right->unv.ival){
				    result->node_id = DOUBLE;
				    result->unv.dval = left->unv.dval;
				}else{
				    result->node_id = INT;
				    result->unv.ival = right->unv.ival;
				}
				break;
			    case DOUBLE:
				result->node_id = DOUBLE;
				result->unv.dval = left->unv.dval > right->unv.dval ?
				    left->unv.dval : right->unv.dval;
				break;
			    case VARIABLE:
				assert(0);
				break;
			    case BOOLEAN:
				/* Evaluation failure */
				t->computation_failed = true;
				break;
			    default:
				assert(0);
				break;
			}
			break;
		    case VARIABLE:
			/* max ( VARIABLE, ?) */
			switch(right->node_id){
			    case INT:
			    case DOUBLE:
			    case VARIABLE:
			    case BOOLEAN:
				assert(0);
				break;
			    default:
				assert(0);
				break;
			}
			break;
		    case BOOLEAN:
			/* max (BOOLEAN, ? ) */
			switch(right->node_id){
			    case INT:
			    case DOUBLE:
			    case VARIABLE:
			    case BOOLEAN:
				/* Evaluation failure */
				t->computation_failed = true;
				break;
			    default:
				assert(0);
				break;
			}
		    default:
			assert(0);
			break;
		}
		break;
	    case POW:
		switch(left->node_id){
		    case INT:
			/* POW (INT, ?) */
			switch(right->node_id){
			    case INT:
				result->node_id = DOUBLE;
				result->unv.dval = pow(left->unv.ival,
						       right->unv.ival);
				break;
			    case DOUBLE:
				result->node_id = DOUBLE;
				result->unv.dval = pow(left->unv.ival,
						       right->unv.dval);
				break;
			    case VARIABLE:
				assert(0);
				break;
			    case BOOLEAN:
				t->computation_failed = true;
				break;
			    default:
				assert(0);
			}
			break;
		    case DOUBLE:
			/* POW (DOUBLE, ?) */
			switch(right->node_id){
			    case INT:
				result->node_id = DOUBLE;
				result->unv.dval = pow(left->unv.dval,
						       right->unv.ival);
				break;
			    case DOUBLE:
				result->node_id = DOUBLE;
				result->unv.dval = pow(left->unv.dval,
						       right->unv.dval);
				break;
			    case VARIABLE:
				assert(0);
				break;
			    case BOOLEAN:
				t->computation_failed = true;
				break;
			    default:
				assert(0);
			}
			break;
		    case VARIABLE:
			/* POW (VARIABLE, ?) */
			switch(right->node_id){
			    case INT:
			    case DOUBLE:
			    case VARIABLE:
			    case BOOLEAN:
				assert(0);
				break;
			    default:
				assert(0);
				break;
			}
			break;
		    case BOOLEAN:
			/* POW (BOOLEAN, ?) */
			switch(right->node_id){
			    case INT:
			    case DOUBLE:
				t->computation_failed = true;
				break;
			    case VARIABLE:
				assert(0);
				break;
			    case BOOLEAN:
				t->computation_failed = true;
				break;
			    default:
				assert(0);
			}
			break;
		    default:
			assert(0);
			break;
		}
		break;
	    case GREATER_THAN_OR_EQUAL_TO:
		switch(left->node_id){
		    case INT:
			/* INT >= ? */
			switch(right->node_id){
			    case INT:
				result->node_id = BOOLEAN;
				result->unv.bval = left->unv.ival >= right->unv.ival;
				break;
			    case DOUBLE:
				result->node_id = BOOLEAN;
				result->unv.bval = (double) left->unv.ival >= right->unv.dval;
				break;
			    case VARIABLE:
				assert(0);
				break;
			    case BOOLEAN:
				/* Evaluation failure */
				t->computation_failed = true;
				break;
			    default:
				assert(0);
				break;
			}
			break;
		    case DOUBLE:
			/* DOUBLE >= ? */
			switch(right->node_id){
			    case INT:
				result->node_id = BOOLEAN;
				result->unv.bval = left->unv.dval >= (double) right->unv.ival;
				break;
			    case DOUBLE:
				result->node_id = BOOLEAN;
				result->unv.bval = left->unv.dval >= right->unv.dval;
				break;
			    case VARIABLE:
				assert(0);
				break;
			    case BOOLEAN:
				t->computation_failed = true;
				break;
			    default:
				assert(0);
				break;
			}
			break;
		    case VARIABLE:
			/* VARIABLE >= ? */
			switch(right->node_id){
			    case INT:
			    case DOUBLE:
			    case VARIABLE:
			    case BOOLEAN:
			    default:
				assert(0);
				break;
			}
			break;
		    case BOOLEAN:
			/* BOOLEAN >= ? */
			switch(right->node_id){
			    case INT:
			    case DOUBLE:
			    case VARIABLE:
			    case BOOLEAN:
				t->computation_failed = true;
				break;
			    default:
				assert(0);
				break;
			}
			break;
		    default:
			assert(0);
			break;
		}
		break;
	    case LESS_THAN_OR_EQUAL_TO:
		switch(left->node_id){
		    case INT:
			/* INT <= ? */
			switch(right->node_id){
			    case INT:
				result->node_id = BOOLEAN;
				result->unv.bval = left->unv.ival <= right->unv.ival;
				break;
			    case DOUBLE:
				result->node_id = BOOLEAN;
				result->unv.bval = (double) left->unv.ival <= right->unv.dval;
				break;
			    case VARIABLE:
				assert(0);
				break;
			    case BOOLEAN:
				/* Evaluation failure */
				t->computation_failed = true;
				break;
			    default:
				assert(0);
				break;
			}
			break;
		    case DOUBLE:
			/* DOUBLE <= ? */
			switch(right->node_id){
			    case INT:
				result->node_id = BOOLEAN;
				result->unv.bval = left->unv.dval <= (double) right->unv.ival;
				break;
			    case DOUBLE:
				result->node_id = BOOLEAN;
				result->unv.bval = left->unv.dval <= right->unv.dval;
				break;
			    case VARIABLE:
				assert(0);
				break;
			    case BOOLEAN:
				/* Evaluation failure */
				t->computation_failed = true;
				break;
			    default:
				assert(0);
				break;
			}
			break;
		    case VARIABLE:
			/* VARIABLE <= ? */
			switch(right->node_id){
			    case INT:
			    case DOUBLE:
			    case VARIABLE:
				assert(0);
				break;
			    case BOOLEAN:
				/* Evaluation failure. Report an error */
				break;
			    default:
				assert(0);
				break;
			}
			break;
		    case BOOLEAN:
			/* BOOLEAN <= ? */
			switch(right->node_id){
			    case INT:
			    case DOUBLE:
			    case VARIABLE:
			    case BOOLEAN:
				/* Evaluation failure */
				t->computation_failed = true;
				break;
			    default:
				assert(0);
				break;
			}
			break;
		    default:
			assert(0);
			break;
		}
		break;
	    case GREATER_THAN:
		switch(left->node_id){
		    case INT:
			/* INT > ? */
			switch(right->node_id){
			    case INT:
				result->node_id = BOOLEAN;
				result->unv.bval = left->unv.ival > right->unv.ival;
				break;
			    case DOUBLE:
				result->node_id = BOOLEAN;
				result->unv.bval = left->unv.ival > right->unv.dval;
				break;
			    case VARIABLE:
				assert(0);
				break;
			    case BOOLEAN:
				t->computation_failed = true;
				break;
			    default:
				break;
			}
			break;
		    case DOUBLE:
			/* DOUBLE > ? */
			switch(right->node_id){
			    case INT:
				result->node_id = BOOLEAN;
				result->unv.bval = right->unv.dval > (double) left->unv.ival;
				break;
			    case DOUBLE:
				result->node_id = BOOLEAN;
				result->unv.bval = right->unv.dval > left->unv.dval;
				break;
			    case VARIABLE:
				assert(0);
				break;
			    case BOOLEAN:
				t->computation_failed = true;
				break;
			    default:
				break;
			}
			break;
		    case VARIABLE:
			/* VARIABLE > ? */
			switch(right->node_id){
			    case INT:
			    case DOUBLE:
			    case VARIABLE:
			    case BOOLEAN:
				assert(0);
				break;
			    default:
				assert(0);
				break;
			}
			break;
		    case BOOLEAN:
			/* BOOLEAN > ? */
			switch(right->node_id){
			    case INT:
			    case DOUBLE:
			    case VARIABLE:
			    case BOOLEAN:
				t->computation_failed = true;
				break;
			    default:
				break;
			}
			break;
		    default:
			assert(0);
			break;
		}
		break;
	    case LESS_THAN:
		switch(left->node_id){
		    case INT:
			/* INT < ? */
			switch(right->node_id){
			    case INT:
				break;
			    case DOUBLE:
				break;
			    case VARIABLE:
			    case BOOLEAN:
				break;
			    default:
				break;
			}
			break;
		    case DOUBLE:
			/* DOUBLE < ? */
			switch(right->node_id){
			    case INT:
			    case DOUBLE:
			    case VARIABLE:
			    case BOOLEAN:
				break;
			    default:
				break;
			}
			break;
		    case VARIABLE:
			/* VARIABLE < ? */
			switch(right->node_id){
			    case INT:
			    case DOUBLE:
			    case VARIABLE:
			    case BOOLEAN:
				break;
			    default:
				break;
			}
			break;
		    case BOOLEAN:
			/* BOOLEAN < ? */
			switch(right->node_id){
			    case INT:
			    case DOUBLE:
			    case VARIABLE:
			    case BOOLEAN:
				break;
			    default:
				break;
			}
			break;
		    default:
			assert(0);
			break;
		}
		break;
	    case NEQ:
		switch(left->node_id){
		    case INT:
			/* INT != ? */
			switch(right->node_id){
			    case INT:
			    case DOUBLE:
			    case VARIABLE:
			    case BOOLEAN:
				break;
			    default:
				break;
			}
			break;
		    case DOUBLE:
			/* DOUBLE != ? */
			switch(right->node_id){
			    case INT:
			    case DOUBLE:
			    case VARIABLE:
			    case BOOLEAN:
				break;
			    default:
				break;
			}
			break;
		    case VARIABLE:
			/* VARIABLE != ? */
			switch(right->node_id){
			    case INT:
			    case DOUBLE:
			    case VARIABLE:
			    case BOOLEAN:
				break;
			    default:
				break;
			}
			break;
		    case BOOLEAN:
			/* BOOLEAN != ? */
			switch(right->node_id){
			    case INT:
			    case DOUBLE:
			    case VARIABLE:
			    case BOOLEAN:
				break;
			    default:
				break;
			}
			break;
		    default:
			assert(0);
			break;
		}
		break;
	    case EQ:
		switch(left->node_id){
		    case INT:
			/* INT = ? */
			switch(right->node_id){
			    case INT:
				result->node_id = BOOLEAN;
				result->unv.bval = left->unv.ival == right->unv.ival;
				break;
			    case DOUBLE:
				break;
			    case VARIABLE:
				break;
			    case BOOLEAN:
				break;
			}
			break;
		    case DOUBLE:
			/* DOUBLE = ? */
			break;
		    case VARIABLE:
			/* VARIABLE = ? */
			break;
		    case BOOLEAN:
			/* BOOLEAN = ? */
			break;
		    default:
			assert(0);
			break;
		}
		break;
	    case OR:
		switch(left->node_id){
		    case INT:
			/* INT or ? */
			break;
		    case DOUBLE:
			/* DOUBLE or ? */
			break;
		    case VARIABLE:
			/* VARIABLE or ? */
			break;
		    case BOOLEAN:
			/* BOOLEAN or ? */
			switch(right->node_id){
			    case INT:
			    case DOUBLE:
			    case VARIABLE:
				break;
			    case BOOLEAN:
				result->node_id = BOOLEAN;
				result->unv.bval = left->unv.bval ||
				    right->unv.bval;
				break;
			    default:
				break;
			}
			break;
		    default:
			assert(0);
			break;
		}
		break;
	    case AND:
		switch(left->node_id){
		    case INT:
			/* INT and ? */
			break;
		    case DOUBLE:
			/* DOUBLE and ? */
			break;
		    case VARIABLE:
			/* VARIABLE and ? */
			break;
		    case BOOLEAN:
			/* BOOLEAN and ? */
			switch(right->node_id){
			    case INT:
			    case DOUBLE:
			    case VARIABLE:
				/* Evaluation failure. Report an error */
				break;
			    case BOOLEAN:
				result->node_id = BOOLEAN;
				result->unv.bval = left->unv.bval &&
				    right->unv.bval;
				break;
			}
			break;
		    default:
			assert(0);
			break;
		}
		break;
	    default:
		assert(0);
		break;

	}
    }else{
	/*
	 * This is a branch, but this node does not have any operator.
	 */
	assert(0);
    }

    return result;
}

/* The minimum necessary tests */
static bool
is_invalid_tr_node(tr_node *trn){
    if (trn == NULL)
	return true; /* invalid */

    if (trn->node_id == INT
	|| trn->node_id == DOUBLE
	|| trn->node_id == BOOLEAN)
	return false;
    else
	return true; /* invalid */
}

/*
 * Resolve variables based on 'app_data_src' and 'app_access_cb'.
 *
 * The caller must avoid evaluation if 't->resolved' is set to false.
 *
 * It means either user didn't call this function even when the expression
 * includes variable or user-defined callback returned something wrong.
 */
void
resolve_variable(tree *t, void *app_data_src,
		 tr_node *(*app_access_cb)(char *, void *)){
    tr_node *n, *tmp;
    variable *v;
    bool contain_illegal_var = false;

    assert(t != NULL);
    assert(t->list_head != NULL);

    if (app_data_src == NULL || app_access_cb == NULL)
	return;

    n = t->list_head;

    while(n != NULL){
	if (n->node_id == VARIABLE){
	    v = &n->unv.vval;

	    tmp = app_access_cb(v->vname, app_data_src);
	    if (is_invalid_tr_node(tmp))
		contain_illegal_var = true;
	    else{
		v->is_resolved = true;
		v->vdata = tmp;
	    }
	}
	n = n->list_right;
    }

    if (!contain_illegal_var)
	t->resolved = true;
}
