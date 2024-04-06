
%{
/*
 * Define all supportive data structures definitions.
 */

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

typedef struct lex_stack {
    int stack_pointer;
    lex_data main_data[MAX_STACK_INDEX];
} lex_stack;

extern lex_stack lstack;
lex_stack lstack = { 0, { 0, 0, NULL} };

/* Parse inputs */
#define BUFFER_LEN 512
extern char lex_buffer[BUFFER_LEN];
char lex_buffer[BUFFER_LEN];
extern char *next_parse_pos;
char *next_parse_pos = lex_buffer;

/* Manage lex_stack */
extern int cyylex();
extern void yyrewind(int n);

/* Stack operations */
static void
lex_push(lex_data data){
    assert(lstack.stack_pointer >= 0 &&
	   lstack.stack_pointer < MAX_STACK_INDEX);
    lstack.main_data[lstack.stack_pointer++] = data;
}

static lex_data
lex_pop(){
    assert(lstack.stack_pointer >= 0);
    return lstack.main_data[lstack.stack_pointer--];
}

static void
process_white_space(int n){
    lex_data ldata;
    next_parse_pos += n;
    ldata.token_code = WHITE_SPACE;
    ldata.token_len = n;
    ldata.token_val = NULL;
    lex_push(ldata);
}

/* Make use of this function to dump any debug information */
static void
debug_print(void){
    int i;
    
    for (i = 0; i < lstack.stack_pointer; i++){
	/* Make the output of '\n' tidy */
	if (lstack.main_data[i].token_val != NULL &&
	    strncmp(lstack.main_data[i].token_val, "\n", strlen("\n")) == 0){
	    printf("[%03d] string '\\n' is '%s' token\n",
		   i, Mexpr_get_string_token(PARSER_EOF));
	}else{
	    printf("[%03d] string '%s' is '%s' token\n", i,
		   lstack.main_data[i].token_val != NULL ?
		   lstack.main_data[i].token_val : " " /* or TAB */,
		   Mexpr_get_string_token(lstack.main_data[i].token_code));
	}
    }
}

%}

%%

\n              { return PARSER_EOF; }

debug            { debug_print(); }

0|-?[1-9][0-9]*  { return INT; }

-?[0-9]*\.[0-9]+ { return DOUBLE; }

[a-zA-Z1-9_]+    { return VARIABLE; }

[ ]              { process_white_space(1); }
[\t]             { process_white_space(strlen("\t")); }

.               { printf("detected invalid input '%s'\n", yytext); }

%%

int
cyylex(){
    int token_code = yylex();
    lex_data ldata;

    ldata.token_code = token_code;
    ldata.token_len = yyleng;
    next_parse_pos += yyleng;
    ldata.token_val = (char *) malloc(yyleng + 1);
    if (ldata.token_val == NULL){
	perror("malloc");
	exit(-1);
    }

    memset(ldata.token_val, '\0', yyleng + 1);
    strncpy(ldata.token_val, yytext, yyleng + 1);
    lex_push(ldata);

    return token_code;
}

void
yyrewind(int n){
    
}

int
main(int argc, char **argv){
    int token_code;

    while (1){

	printf("Input -> ");
	fgets(lex_buffer, sizeof(lex_buffer), stdin);

	/* Tell the lex which buffer to parse */
	yy_scan_string(lex_buffer);
	/* Calling yylex() sets up 'token_code', 'yytext' and 'yyleng' */
	token_code = cyylex();

	while(token_code != PARSER_EOF){
	    printf("token code = '%s', token = '%s', length = '%lu'\n",
		   Mexpr_get_string_token(token_code), yytext, yyleng);
	    token_code = cyylex();
	}
    }

    return 0;
}
