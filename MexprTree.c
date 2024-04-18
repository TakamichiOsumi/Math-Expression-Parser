#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include "MexprEnums.h"
#include "Linked-List/linked_list.h"
#include "Stack/stack.h"
#include "ExportedParser.h"
#include "MexprTree.h"

static tree*
gen_tree(void){
    tree *t;

    t = (tree *) malloc(sizeof(tree));
    if (t == NULL){
	perror("malloc");
	exit(-1);
    }

    t->root = t->list_head = NULL;

    return t;
}

/*
 * The caller must pass each token processed by postfix converter.
 */
static tr_node*
gen_tr_node(lex_data *ld){
    tr_node *n;

    if ((n = (tr_node *) malloc(sizeof(node))) == NULL){
	perror("malloc");
	exit(-1);
    }

    n->parent = n->left = n->right
	= n->list_left = n->list_right = NULL;

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
	    n->unv.vval = ld->token_val;
	    break;

	/* Unary operator */
	case SIN:
	case COS:
	case SQR:
	case SQRT:
	    n->node_id = ld->token_code;
	    n->unv.vval = ld->token_val;
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
	    n->node_id = ld->token_code;
	    n->unv.vval = ld->token_val;
	    break;

	default:
	    assert(0);
	    break;
    }

    return n;
}

tree*
convert_postfix_to_tree(linked_list *postfix_array){
    stack *node_stack = stack_init(ll_get_length(postfix_array));
    node *lln;
    lex_data *curr;
    tr_node *trn;
    tree *t = gen_tree();

    printf("--- <Convert postfix to tree> ---\n");

    ll_begin_iter(postfix_array);
    while((lln = ll_get_iter_node(postfix_array)) != NULL){

	curr = (lex_data *) lln->data;
	trn = gen_tr_node(curr);

	if (is_operand(curr->token_code)){
	    printf("push operand = '%s'\n",
		   get_string_token(curr->token_code));
	    stack_push(node_stack, (void *) trn);
	}else if (is_unary_operator(curr->token_code)){
	    trn->left = stack_pop(node_stack);
	    printf("push unary operator = '%s' with its child = '%s'\n",
		   get_string_token(trn->node_id),
		   get_string_token(trn->left->node_id));
	    stack_push(node_stack, (void *) trn);
	}else if (is_binary_operator(curr->token_code)){
	    trn->right = stack_pop(node_stack);
	    trn->left = stack_pop(node_stack);
	    printf("push binary operator = '%s' with its children (left : '%s', right '%s')\n",
		   get_string_token(curr->token_code),
		   get_string_token(trn->left->node_id),
		   get_string_token(trn->right->node_id));
	    stack_push(node_stack, (void *) trn);
	}
    }
    ll_end_iter(postfix_array);

    assert((t->root = stack_pop(node_stack)) != NULL);
    printf("root = '%s'\n", get_string_token(t->root->node_id));

    printf("----------------------\n");

    stack_destroy(node_stack);

    return t;
}
