
%{
/*
 * Define all supportive data structures definitions.
 */

#include <stdio.h>
#include <string.h>
#include "MexprEnums.h"
#define BUFFER_LEN 512

// extern "C" int yylex(); /* Suppress bug in the parser code */
extern char lex_buffer[BUFFER_LEN];
char lex_buffer[BUFFER_LEN];

%}


%%

\n              { printf("\\n found\n"); return PARSER_EOF; }

0|-?[1-9][0-9]*  { return MATH_INT; }

-?[0-9]*\.[0-9]+ { return MATH_DOUBLE; }

[a-zA-Z1-9_]+    { return MATH_VARIABLE; }

[ \t]+              { /* Whitespace/tab should be ignored */ }

.               { printf("invalid input '%s' has failed to match any patterns\n",
                  yytext); }

%%

int
main(int argc, char **argv){
    int token_code;

    while (1){

	printf("Input -> ");
	fgets(lex_buffer, sizeof(lex_buffer), stdin);

	if (lex_buffer[0] == '\n'){
	    lex_buffer[0] = 0;
	    continue;
	}

	/* Tell the lex which buffer to parse */
	yy_scan_string(lex_buffer);

	token_code = yylex();
	while(token_code != PARSER_EOF){
	    printf("token code = '%s', token = '%s', length = '%lu'\n",
		   Mexpr_get_string_token(token_code),
		   yytext, yyleng);
	    token_code = yylex();
	}
    }

    return 0;
}
