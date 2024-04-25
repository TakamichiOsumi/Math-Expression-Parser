#ifndef __EXPORTED_PARSER__
#define __EXPORTED_PARSER__

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "Linked-List/linked_list.h"
#include "MexprEnums.h"
#include "MexprTree.h"

/* Stack the parsed results */
#define MAX_STACK_INDEX 512
typedef struct lex_data {
    int token_code;
    int token_len;
    char *token_val;
} lex_data;

typedef struct lex_stack {
    int stack_pointer;
    lex_data main_data[MAX_STACK_INDEX];
} lex_stack;

extern lex_stack lstack;

/* Parse inputs */
#define BUFFER_LEN 512
extern char lex_buffer[BUFFER_LEN];
extern char *next_parse_pos;

#define STRING_IS_NEWLINE(s) \
    (s != NULL && (strncmp(s, "\n", strlen("\n")) == 0))

/* Manage lex_stack */
extern int cyylex();
extern void yyrewind(int n);
extern void parser_stack_reset(void);

/* The caller must declare and pass one variable for below macros */
#define CHECKPOINT(checkpoint_index) \
    { checkpoint_index = lstack.stack_pointer; }
#define RESTORE_CHECKPOINT(checkpoint_index) \
    { yyrewind(lstack.stack_pointer - checkpoint_index); }

extern bool start_mathexpr_parse();
extern bool start_ineq_mathexpr_parse();
extern bool start_logical_mathexpr_parse();

void resolve_and_evaluate_test(bool (*parser)(void), char *target, void *app_data_src,
			       tr_node *(*app_access_cb)(struct variable *, void *));

#endif
