
%{
/* Define all supportive data structures definitions */

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
 lex_data ldummy = { INVALID, 0, NULL };

typedef struct lex_stack {
    int stack_pointer;
    lex_data main_data[MAX_STACK_INDEX];
} lex_stack;

extern lex_stack lstack;
lex_stack lstack = { INVALID, { 0, 0, NULL} };

/* Parse inputs */
#define BUFFER_LEN 512
extern char lex_buffer[BUFFER_LEN];
char lex_buffer[BUFFER_LEN];

extern char *next_parse_pos;
char *next_parse_pos = lex_buffer;

/* Manage lex_stack */
extern int cyylex();
extern void yyrewind(int n);
extern void parser_stack_reset(void);
extern void lex_set_scan_buffer(const char *buffer);

#define STRING_IS_NEWLINE(s) (s != NULL && (strncmp(s, "\n", strlen("\n")) == 0))

/* Stack operations */
static void
lex_push(lex_data data){
    if (lstack.stack_pointer < 0){
	printf("The stack pointer is invalid = %d. Do nothing\n",
	       lstack.stack_pointer);
	return;
    }
    if(lstack.stack_pointer > MAX_STACK_INDEX - 1){
	printf("The stack is full. Push nothing.\n");
	return;
    }

    /* Don't push the newline char */
    if (STRING_IS_NEWLINE(data.token_val)){
	return;
    }else{
	printf("Pushed '%s' (code = '%s') at index=%d\n",
	       data.token_val,
	       Mexpr_get_string_token(data.token_code),
	       lstack.stack_pointer);
    }

    lstack.main_data[lstack.stack_pointer++] = data;
}

static lex_data
lex_pop(){
    lex_data ldata = { INVALID, 0, NULL };
    lex_data *ldp;

    if (lstack.stack_pointer <= 0){
	/* The below code will be replaced with assert(0); */
	printf("The stack is empty. Failed to pop()\n");

	return ldummy;
    }

    ldp = &lstack.main_data[lstack.stack_pointer - 1];

    /* debug print */
    printf("Popped '%s' (code = '%s') at index=%d\n",
               ldp->token_val, Mexpr_get_string_token(ldp->token_code),
	       lstack.stack_pointer);

    lstack.stack_pointer--;

    return lstack.main_data[lstack.stack_pointer];
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
	printf("[%03d] string '%s' is '%s' token\n", i,
	       lstack.main_data[i].token_val == NULL ? " " /* or tab */:
	       lstack.main_data[i].token_val,
	       Mexpr_get_string_token(lstack.main_data[i].token_code));
    }
}

extern void
parser_stack_reset(void){
    int i;
    lex_data *ldata;

    for (i = 0; i < lstack.stack_pointer; i++){
	ldata = &lstack.main_data[i];
	ldata->token_code = INVALID;
	ldata->token_len = 0;
	if (ldata->token_val){
	    free(ldata->token_val);
	    ldata->token_val = NULL;
	}
    }
    lstack.stack_pointer = 0;

    /*
     * If the buffer is not reset by fgets(), then reset the next parse position
     * to the beginning of the 'lex_buffer'.
     */
    /* next_parse_pos = lex_buffer; */
}

extern void
lex_set_scan_buffer(const char *buffer){
    yy_scan_string(buffer);
}

%}

%%

\n               { return PARSER_EOF; }

debug            { /* Only for the development */ debug_print(); }

b[1-9]           { /* Only for the development */ yyrewind(yytext[1] - '0'); }

D                { /* Only for the development */ parser_stack_reset(); }

0|-?[1-9][0-9]*  { return INT; }

-?[0-9]*\.[0-9]+ { return DOUBLE; }

[a-zA-Z1-9_]+    { return VARIABLE; }

[ ]              { process_white_space(1); }

[\t]             { process_white_space(1); }

.                { printf("detected invalid input '%s'\n", yytext); }

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

/* 'n' : the number to pop up the stack */
void
yyrewind(int n){
    int removed_total_strlen = 0;
    lex_data ldata;

    printf("Rewinding %d stack levels ...\n", n);

    if (n <= 0)
	return;

    if (next_parse_pos == lex_buffer)
	return;

    while(n){
	/* Safeguard for the case 'n' > lstack's 'stack_pointer' */
	if (lstack.stack_pointer < 0){
	    break;
	}

	ldata = lex_pop();
	removed_total_strlen += ldata.token_len;
	if (ldata.token_code == WHITE_SPACE){
	    continue;
	}else
	    n--;

	ldata.token_code = 0;
	ldata.token_len = 0;
    }

    next_parse_pos -= removed_total_strlen;

    /*
     * Notify the lexical parser of the moved starting position to parse,
     * if the buffer is not reset by fgets().
     */
    /* yy_scan_string(next_parse_pos); */
}

int
main(int argc, char **argv){
    int token_code;

    memset(lex_buffer, '\0', BUFFER_LEN);

    while (1){

	printf("Input : ");
	fgets(lex_buffer, sizeof(lex_buffer), stdin);

	/* Tell the lex which buffer to parse */
	yy_scan_string(lex_buffer);

	/* Calling yylex() sets up 'token_code', 'yytext' and 'yyleng' */
	token_code = cyylex();

	while(token_code != PARSER_EOF){
	    /* printf("token code = '%s', token = '%s', length = '%lu'\n",
	       Mexpr_get_string_token(token_code), yytext, yyleng); */
	    token_code = cyylex();
	}
    }

    return 0;
}
