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

tr_node *evaluate_node(tr_node *self);

/* free callback */
static void
free_dynamic_calculated_node(void *p){
    node *lln = (node *) p;
    tr_node *n = (tr_node *) lln->data;

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
	case REMAINDER:
	case MIN:
	case MAX:
	case POW:
	    n->node_id = ld->token_code;
	    n->unv.operator = ld->token_val;
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
convert_postfix_to_tree(linked_list *postfix_array){
    stack *node_stack = stack_init(ll_get_length(postfix_array));
    node *lln;
    lex_data *curr;
    tr_node *trn, *prev;
    tree *t = gen_tree();

    ll_begin_iter(postfix_array);

    while((lln = ll_get_iter_node(postfix_array)) != NULL){
	curr = (lex_data *) lln->data;
	trn = gen_tr_node_from_lex_data(curr);

	if (is_operand(curr->token_code)){
	    stack_push(node_stack, (void *) trn);

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
	    stack_push(node_stack, (void *) trn);

	}else if (is_binary_operator(curr->token_code)){

	    trn->right = stack_pop(node_stack);
	    trn->left = stack_pop(node_stack);
	    stack_push(node_stack, (void *) trn);

	}
    }

    ll_end_iter(postfix_array);

    assert((t->root = stack_pop(node_stack)) != NULL);

    stack_destroy(node_stack);

    return t;
}

void
evaluate_tree(tree *t){
    tr_node *result;

    if (t->require_resolution && !t->resolved){
	printf("variable included in expression but not resolved\n");
	return;
    }

    ll_tmp_calc = ll_init(NULL,
			  free_dynamic_calculated_node);

    result = evaluate_node(t->root);
    switch(result->node_id){
	case INT:
	    printf("=> %d\n",
		   result->unv.ival);
	    break;
	case DOUBLE:
	    printf("=> %f\n",
		   result->unv.dval);
	    break;
	case VARIABLE:
	    printf("=> %s\n",
		   result->unv.vval.vname);
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
 * As for the path to access the VARIABLE node, there are
 * assert() statements. This is because the recursive call
 * of evaluate_node() returns concrete value types such as
 * INT and DOUBLE.
 */
tr_node *
evaluate_node(tr_node *self){
    tr_node *result, *left, *right;

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
	left = evaluate_node(self->left);

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
	    default:
		assert(0);
		return NULL;
	}
    }else if (is_binary_operator(self->node_id)){
	assert(self->left != NULL);
	assert(self->right != NULL);

	result = gen_null_tr_node();
	left = evaluate_node(self->left);
	right = evaluate_node(self->right);

	assert(left != NULL);
	assert(right != NULL);

	switch(self->node_id){
	    case PLUS:
		switch(left->node_id){
		    case INT:
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
			    default:
				assert(0);
				break;
			}
			break;
		    case DOUBLE:
			switch(right->node_id){
			    case INT:
				result->node_id = DOUBLE;
				result->unv.dval = left->unv.dval + right->unv.ival;
				break;
			    case DOUBLE:
				result->node_id = DOUBLE;
				result->unv.dval = left->unv.dval + right->unv.dval;
				break;
			    case VARIABLE:
				assert(0);
				break;
			    default:
				assert(0);
				break;
			}
			break;
		    case VARIABLE:
			switch(right->node_id){
			    case INT:
			    case DOUBLE:
			    case VARIABLE:
				assert(0);
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
	    case MINUS:
		switch(left->node_id){
		    case INT:
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
			    default:
				assert(0);
				break;
			}
			break;
		    case DOUBLE:
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
			    default:
				assert(0);
				break;
			}
			break;
		    case VARIABLE:
			switch(right->node_id){
			    case INT:
			    case DOUBLE:
			    case VARIABLE:
				assert(0);
				break;
			    default:
				assert(0);
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
			    default:
				assert(0);
				break;
			}
			break;
		    case DOUBLE:
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
			    default:
				assert(0);
				break;
			}
			break;
		    case VARIABLE:
			switch(right->node_id){
			    case INT:
			    case DOUBLE:
			    case VARIABLE:
				assert(0);
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
	    case DIVIDE:
		switch(left->node_id){
		    case INT:
			switch(right->node_id){
			    case INT:
				result->node_id = INT;
				result->unv.ival = left->unv.ival / right->unv.ival;
				break;
			    case DOUBLE:
				result->node_id = DOUBLE;
				result->unv.dval = left->unv.ival / right->unv.dval;
				break;
			    case VARIABLE:
				assert(0);
				break;
			    default:
				assert(0);
				break;
			}
			break;
		    case DOUBLE:
			switch(right->node_id){
			    case INT:
				result->node_id = DOUBLE;
				result->unv.dval = left->unv.dval / right->unv.ival;
				break;
			    case DOUBLE:
				result->node_id = DOUBLE;
				result->unv.dval = left->unv.dval / right->unv.dval;
				break;
			    case VARIABLE:
				assert(0);
				break;
			    default:
				assert(0);
				break;
			}
			break;
		    case VARIABLE:
			switch(right->node_id){
			    case INT:
			    case DOUBLE:
			    case VARIABLE:
				assert(0);
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
	    case REMAINDER:
		switch(left->node_id){
		    case INT:
			switch(right->node_id){
			    case INT:
				result->node_id = INT;
				result->unv.ival = left->unv.ival % right->unv.ival;
				break;
			    case DOUBLE:
				/* Evaluation failure. Report an error */
				break;
			    case VARIABLE:
				assert(0);
				break;
			    default:
				assert(0);
				break;
			}
			break;
		    case DOUBLE:
			switch(right->node_id){
			    case INT:
				/* Evaluation failure. Report an error */
				break;
			    case DOUBLE:
				/* Evaluation failure. Report an error */
				break;
			    case VARIABLE:
				assert(0);
				break;
			    default:
				assert(0);
				break;
			}
			break;
		    case VARIABLE:
			switch(right->node_id){
			    case INT:
			    case DOUBLE:
			    case VARIABLE:
				assert(0);
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
	    case MIN:
		switch(left->node_id){
		    case INT:
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
			    default:
				assert(0);
				break;
			}
			break;
		    case DOUBLE:
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
			    default:
				assert(0);
				break;
			}
			break;
		    case VARIABLE:
			switch(right->node_id){
			    case INT:
			    case DOUBLE:
			    case VARIABLE:
				assert(0);
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
	    case MAX:
		switch(left->node_id){
		    case INT:
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
			    default:
				assert(0);
				break;
			}
			break;
		    case DOUBLE:
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
			    default:
				assert(0);
				break;
			}
			break;
		    case VARIABLE:
			switch(right->node_id){
			    case INT:
			    case DOUBLE:
			    case VARIABLE:
				assert(0);
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
	    case POW:
		switch(left->node_id){
		    case INT:
			break;
		    case DOUBLE:
			break;
		    case VARIABLE:
			switch(right->node_id){
			    case INT:
			    case DOUBLE:
			    case VARIABLE:
				assert(0);
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

void
resolve_variable(tree *t, void *app_data_src,
		 tr_node *(*app_access_cb)(struct variable *, void *)){
    tr_node *n;
    variable *v;

    assert(t != NULL);
    assert(t->list_head != NULL);

    n = t->list_head;

    while(n != NULL){
	if (n->node_id == VARIABLE){
	    v = &n->unv.vval;
	    app_access_cb(v, app_data_src);
	}
	n = n->list_right;
    }

    t->resolved = true;
}
