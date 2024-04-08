#ifndef __EXPORTED_PARSER__
#define __EXPORTED_PARSER__

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "MexprEnums.h"

/* Stack the parsed results */
#define MAX_STACK_INDEX 512
typedef struct lex_data {
    int token_code;
    int token_len;
    char *token_val;
} lex_data;
extern lex_data ldummy;

typedef struct lex_stack {
    int stack_pointer;
    lex_data main_data[MAX_STACK_INDEX];
} lex_stack;

extern lex_stack lstack;

#define STACK_TOPMOST_ELEM (lstack.main_data[lstack.stack_pointer - 1])

/* Parse inputs */
#define BUFFER_LEN 512
extern char lex_buffer[BUFFER_LEN];
extern char *next_parse_pos;

/* Manage lex_stack */
extern int cyylex();
extern void yyrewind(int n);
extern void parser_stack_reset(void);
extern void lex_set_scan_buffer(const char *buffer);
#define STRING_IS_NEWLINE(s) \
    (s != NULL && (strncmp(s, "\n", strlen("\n")) == 0))

#endif
