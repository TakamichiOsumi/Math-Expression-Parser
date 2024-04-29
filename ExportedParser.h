#ifndef __EXPORTED_PARSER__
#define __EXPORTED_PARSER__

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "Linked-List/linked_list.h"
#include "MexprEnums.h"
#include "MexprTree.h"

#define BUFFER_LEN 512
#define MAX_STACK_INDEX (BUFFER_LEN / 2)

typedef struct lex_data {
    int token_code;
    int token_len;
    char *token_val;
} lex_data;

typedef struct lex_stack {
    int stack_pointer;
    lex_data main_data[MAX_STACK_INDEX];
} lex_stack;

/*
 * Manipulate lex_stack. Exported for expression rules.
 *
 * The user of below two macros must declare and pass one int variable.
 */
extern lex_stack lstack;
extern int cyylex();
extern void yyrewind(int n);
extern int lex_stack_pointer(void);
extern void parser_stack_reset(void);
#define CHECKPOINT(checkpoint_index) \
    { checkpoint_index = lex_stack_pointer(); }
#define RESTORE_CHECKPOINT(checkpoint_index) \
    { yyrewind(lex_stack_pointer() - checkpoint_index); }

/*
 * Parse one string, construct a tree and evalute it.
 */
extern void init_buffer(char *target);
extern bool parsed_format_validation(char *s);
extern bool start_mathexpr_parse();
extern bool start_ineq_mathexpr_parse();
extern bool start_logical_mathexpr_parse();
extern linked_list *convert_infix_to_postfix(lex_data *infix, int size_in);
void resolve_and_evaluate_test(bool (*parser)(void), char *target, void *app_data_src,
			       tr_node *(*app_access_cb)(struct variable *, void *));

#endif
