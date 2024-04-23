#ifndef __MATH_EXPR_TREE__
#define __MATH_EXPR_TREE__

#include <stdbool.h>

typedef struct tr_node tr_node;

/*
 * VARIABLE type defined in 'tr_node'.
 *
 * VARIABLE type should be retrieved by its operand name
 * as key from its data source. Meanwhile, this is a part
 * of library code. Then, we have to make it possible that
 * each VARIABLE fetches its data provided by application
 * side. Application needs to give two pieces of information,
 * 'app_data_src' and 'app_access_cb' for that purpose.
 *
 * See the beginning of evaluate_node() for the usage.
 */
typedef struct variable {
    char *vname;

    /* True if the 'vdata' is already fetched */
    bool is_resolved;
    tr_node *vdata;

    /*
     * Application-defined external data storage
     * and access callback.
     */
    void *app_data_src;
    tr_node *(*app_access_cb)(void *);

} variable;

typedef union node_value {

    /*
     * String representation of operator
     */
    char *operator;

    /* INT */
    int ival;

    /* DOUBLE */
    double dval;

    /* VARIABLE */
    variable vval;

    /* BOOLEAN */
    bool bval;

} node_value;

typedef struct tr_node {
    int node_id;

    /* This 'unv' represents union value */
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
