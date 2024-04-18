#ifndef __MATH_EXPR_TREE__
#define __MATH_EXPR_TREE__

union node_value {

    int ival;
    double dval;

    /*
     * VARIABLE or string representation of operator
     */
    char *vval;

};

typedef struct tr_node {
    int node_id;

    union node_value unv;

    /* Refer to the other operand or operator */
    struct tr_node *parent;
    struct tr_node *left;
    struct tr_node *right;

    /*
     * Create a doubly linked list among leaf nodes
     * for quick reference.
     */
    struct tr_node *list_left;
    struct tr_node *list_right;

} tr_node;

typedef struct tree {
    /* Refer to the root node */
    struct tr_node *root;

    /* Refer to the leftmost leaf node */
    struct tr_node *list_head;
} tree;

tree* convert_postfix_to_tree(linked_list *postfix_array);

#endif
