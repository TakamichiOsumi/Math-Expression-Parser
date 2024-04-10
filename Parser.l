
%{

#include "ExportedParser.h"

lex_data ldummy = { INVALID, 0, NULL };
lex_stack lstack = { INVALID, { 0, 0, NULL} };
char lex_buffer[BUFFER_LEN];
char *next_parse_pos = lex_buffer;

extern void
lex_set_scan_buffer(const char *buffer){
    yy_scan_string(buffer);
}

/* Stack operations */
static void
lex_push(lex_data data){
    if (lstack.stack_pointer < 0){
	printf("The stack pointer is smaller than zero = %d\n",
	       lstack.stack_pointer);
	return;
    }else if (lstack.stack_pointer > MAX_STACK_INDEX - 1){
	printf("The stack pointer is bigger than the upper limit = %d\n",
	       lstack.stack_pointer);
	return;
    }

    if (STRING_IS_NEWLINE(data.token_val)){
	printf("Pushed '\\n' whose code is '%s' at index=%d\n",
	       Mexpr_get_string_token(data.token_code),
	       lstack.stack_pointer - 1);
    }else{
	printf("Pushed '%s' whose code is '%s' at index=%d\n",
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
	printf("The stack is empty. Do not pop()\n");

	return ldummy;
    }

    ldp = &lstack.main_data[lstack.stack_pointer - 1];

    /* debug print */
    if (STRING_IS_NEWLINE(ldp->token_val)){
	printf("\tPopped '\\n' whose code is '%s' at index=%d\n",
	       Mexpr_get_string_token(ldp->token_code),
	       lstack.stack_pointer - 1);
    }else{
	printf("\tPopped '%s' whose code is '%s' at index=%d\n",
	       ldp->token_val, Mexpr_get_string_token(ldp->token_code),
	       lstack.stack_pointer - 1);
    }

    lstack.stack_pointer--;

    return lstack.main_data[lstack.stack_pointer];
}

static void
process_white_space_or_tab(mexpr_dtype type, int n){
    lex_data ldata;

    next_parse_pos += n;
    ldata.token_code = type;
    ldata.token_len = n;

    /* Create the copy of the whitespaces/tabs */
    ldata.token_val = (char *) malloc(yyleng + 1);
    if (ldata.token_val == NULL){
	perror("malloc");
	exit(-1);
    }
    memset(ldata.token_val, '\0', yyleng + 1);
    strncpy(ldata.token_val, yytext, yyleng + 1);

    lex_push(ldata);
}

/* Dump any useful information for debugging */
static void
debug_print(void){
    int i;

    printf("-----\n");
    for (i = 0; i < lstack.stack_pointer; i++){
	if (STRING_IS_NEWLINE(lstack.main_data[i].token_val)){
	    printf("\tPopped '\\n' whose code is '%s' at index=%d\n",
		   Mexpr_get_string_token(lstack.main_data[i].token_code),
		   lstack.stack_pointer - 1);
	}else{
	    printf("[%03d] string '%s' is '%s' token\n", i,
		   lstack.main_data[i].token_val,
		   Mexpr_get_string_token(lstack.main_data[i].token_code));
	}
    }
    printf("-----\n");
}

extern void
parser_stack_reset(void){
    int i, removed_string_len = 0;
    lex_data *ldata;

    if ((lstack.stack_pointer < 0) ||
	(lstack.stack_pointer > MAX_STACK_INDEX - 1)){
	printf("the stack pointer = %d is invalid\n",
	       lstack.stack_pointer);
	assert(0);
    }

    for (i = 0; i < lstack.stack_pointer; i++){
	/* printf("Cleaning stack level = %d\n", i); */
	ldata = &lstack.main_data[i];
	ldata->token_code = INVALID;
	removed_string_len += ldata->token_len;
	ldata->token_len = 0;
	if (ldata->token_val != NULL){
	    free(ldata->token_val);
	    ldata->token_val = NULL;
	}
    }

    lstack.stack_pointer = 0;
    next_parse_pos -= removed_string_len;
    lex_set_scan_buffer(next_parse_pos);
}

%}

%%

\n               { return PARSER_EOF; }

"min"            { return MIN;   }

"max"            { return MAX;   }

"sin"            { return SIN;   }

"cos"            { return COS;   }

"pow"            { return POW;   }

"("              { return BRACKET_START; }

")"              { return BRACKET_END;   }

"+"              { return PLUS;      }

"-"              { return MINUS;     }

"*"              { return MULTIPLY;  }

"/"              { return DIVIDE;    }

"%"              { return REMAINDER; }

","              { return COMMA;     }

"="              { return EQ;        }

0|-?[1-9][0-9]*  { return INT;       }

-?[0-9]*\.[0-9]+ { return DOUBLE;    }

[a-zA-Z1-9_]+    { return VARIABLE;  }

[ ]+             { process_white_space_or_tab(WHITE_SPACE, yyleng); }

[\t]+            { process_white_space_or_tab(TAB, yyleng); }

.                { printf("detected invalid input '%s'\n", yytext); }

%%

int
cyylex(){
    int token_code;
    lex_data ldata;

    token_code = yylex();

    ldata.token_code = token_code;
    ldata.token_len = yyleng;
    next_parse_pos += yyleng;

    /* Create the copy of the parsed text */
    ldata.token_val = (char *) malloc(yyleng + 1);
    if (ldata.token_val == NULL){
	perror("malloc");
	exit(-1);
    }
    memset(ldata.token_val, '\0', yyleng + 1);
    strncpy(ldata.token_val, yytext, yyleng + 1);

    /* Save the info into the stack */
    lex_push(ldata);

    return token_code;
}

/* 'n' : the number to pop up the stack */
void
yyrewind(int n){
    int removed_token_len = 0;
    lex_data ldata;

    printf("Rewinding %d stack levels ...\n", n);

    assert(n >= 0);

    if (next_parse_pos == lex_buffer)
	return;

    while(n){
	/* Safeguard for the case 'n' > lstack's 'stack_pointer' */
	if (lstack.stack_pointer < 0){
	    break;
	}

	ldata = lex_pop();
	removed_token_len += ldata.token_len;
	n--;

	ldata.token_code = 0;
	ldata.token_len = 0;
    }

    next_parse_pos -= removed_token_len;

    /*
     * Notify the lexical parser of the moved starting position
     * to parse.
     */
    yy_scan_string(next_parse_pos);
}

extern void parse_equations();
extern void start_sql_parse();
extern void start_mathexpr_parse();

int
main(int argc, char **argv){
    int token_code;

    while (1){
	memset(lex_buffer, '\0', BUFFER_LEN);

	printf("USER INPUT : ");

	fgets(lex_buffer, sizeof(lex_buffer), stdin);

	lex_set_scan_buffer(lex_buffer);

	/* Part1 : Evaluate input string as circle equation */
	/* parse_equations(); */

	/* Part2 : Evaluate input string as select command */
	/* start_sql_parse(); */

	/* Part3 : Leave this piece of codes for debug */
	/*
	token_code = cyylex();
	while(token_code != PARSER_EOF){
	    token_code = cyylex();
	}
	*/

	/* Part4 : Evaluate input string as math expression */
	start_mathexpr_parse();

	/* Dump the stack */
	debug_print();

	/* Clean up the stack */
	parser_stack_reset();
    }

    return 0;
}
