
%{

#include <stdbool.h>
#include "ExportedParser.h"

lex_data ldummy = { INVALID, 0, NULL };
lex_stack lstack = { INVALID, { 0, 0, NULL} };
char lex_buffer[BUFFER_LEN];
char *next_parse_pos = lex_buffer;

extern void
lex_set_scan_buffer(const char *buffer){
    yy_scan_string(buffer);
}

lex_data **
mexpr_convert_infix_to_postfix(lex_data *infix, int size_in, int *size_out){
    return NULL;
}

static void
print_lex_data_in_stack(lex_data data, char *prefix, int index){
    if (STRING_IS_NEWLINE(data.token_val)){
	printf("%s : token = '\n' whose code is '%s' at index = %d\n",
	       prefix,
	       Mexpr_get_string_token(data.token_code),
	       index);
    }else{
	printf("%s : token = '%s' whose code is '%s' at index = %d\n",
	       prefix,
	       data.token_val,
	       Mexpr_get_string_token(data.token_code),
	       index);
    }
}

/* Stack operations */
static void
lex_push(lex_data data){
    assert(lstack.stack_pointer >= 0);

    if (lstack.stack_pointer > MAX_STACK_INDEX - 1){
	printf("The stack pointer is bigger than the upper limit = %d\n",
	       lstack.stack_pointer);
	return;
    }

    print_lex_data_in_stack(data, "\tPush", lstack.stack_pointer - 1);

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

    print_lex_data_in_stack(lstack.main_data[lstack.stack_pointer - 1],
                            "\tPop ",
                            lstack.stack_pointer - 1);

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
	print_lex_data_in_stack(lstack.main_data[i], "debug",
				lstack.stack_pointer - 1);
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

"!="             { return NEQ;       }

">="             { return GREATER_THAN_OR_EQUAL_TO; }

"<="             { return LESS_THAN_OR_EQUAL_TO;    }

"<"              { return LESS_THAN;    }

">"              { return GREATER_THAN; }

"="              { return EQ;        }

0|-?[1-9][0-9]*  { return INT;       }

-?[0-9]*\.[0-9]+ { return DOUBLE;    }

[a-zA-Z1-9_]+    { return VARIABLE;  }

[ ]+             { process_white_space_or_tab(WHITE_SPACE, yyleng); }

[\t]+            { process_white_space_or_tab(TAB, yyleng);         }

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
extern bool start_mathexpr_parse();
extern bool start_ineq_mathexpr_parse();

typedef bool (*parser_func)(void);

/*
 * Input strings as 'tested' whose last string is null.
 *
 * Iterate each string and pass it to the parser. Then,
 * check whether the parser returned the expected boolean
 * result or not.
 *
 * Note : if one string of 'tested' ended without a new line,
 * then, quit the test and exit for quick notification.
 */
static void
parser_test(parser_func parser,
	    char **tested, bool expected_result){
    char last_char;
    int iter = 0;

    while (tested[iter]){
	/* Clean up */
	parser_stack_reset();
	memset(lex_buffer, '\0', BUFFER_LEN);

	/* Pretest format check */
	if (strlen(tested[iter]) > 2){
	    last_char = tested[iter][strlen(tested[iter]) - 1];
	    if (last_char != '\n'){
		printf("invalid string format : '%s'\n",
		       tested[iter]);
		exit(-1);
	    }
	}

	/* Copy the test string to the buffer */
	strncpy(lex_buffer, tested[iter], strlen(tested[iter]));
	lex_set_scan_buffer(lex_buffer);

	if (parser() == expected_result){
	    printf("expected parsing '%s' to return %s and it did\n",
		   tested[iter], expected_result ? "true" : "false");
	}else{
	    printf("'%s' was not parsed expectedly, it was not '%s'\n",
		   tested[iter], expected_result ? "Accepted" : "Rejected");

	    exit(-1);
	}
	debug_print();

	/* Move to the next string */
	iter++;
    }
}

int
main(int argc, char **argv){
    char *success[BUFFER_LEN] = {
	/* single token */
	"1\n",
	"a\n",
	/* plus/minus */
	"1 + 2\n",
	"1 + 2 + 3\n",
	"a + 2 + 3\n",
	"a - 2 - 3\n",
	/* multiply/divide */
	"1 * 2\n",
	"1 * 2 * 3\n",
	"a * 2 * 3\n",
	"a / 2 / 3\n",
	/* brachet */
	"( 1 + 2 ) \n",
	"( 1 + 2 ) * (3 * 4) \n",
	"( 1 - 2 ) + (3 * 4) \n",
	"((1 + 2) - 3) * 4 / (5 / 6) + (7 - 8) \n",
	/* miscellaneous */
	"      a   -    b      \n",
	NULL,
    };

    char *failure[BUFFER_LEN] = {
	/* plus/minus */
	"+\n",
	"- 1\n",
	"1 + 2 + \n",
	"a - 2 - \n",
	/* multiply/divide */
	"*\n",
	"/ 1\n",
	"1 * 2 *\n",
	"1 * 2 /\n",
	"a * 2 /\n",
	"* a / 2 /\n",
	/* brachet */
	"(\n",
	"( )\n",
	"( 1 + 2\n",
	"( 1 + 2 * (3 * 4) \n",
	"3 * 4 / (5 / 6) + (7 - 8) )\n",
	/* miscellaneous */
	"1 1\n",
	"+ *\n",
	NULL,
    };

    char *ineq_success[BUFFER_LEN] = {
	"1 < 2\n",
	"1 > 2\n",
	"a > (1 + 10)\n",
	"(a - b) <= (c + d + e + (f / g))\n",
	"(a - (b * c)) >= (d + e)\n",
	"a - b = c - d\n",
	NULL,
    };

    char *ineq_failure[BUFFER_LEN] = {
	">\n"
	">=\n",
	"< 1\n",
	"() < ( )\n",
	"< <\n",
	"* < 1\n",
	"* 1 < 2\n",
	NULL,
    };

    parser_test(start_mathexpr_parse,
		success, true);
    parser_test(start_mathexpr_parse,
		failure, false);
    parser_test(start_ineq_mathexpr_parse,
		ineq_success, true);
    parser_test(start_ineq_mathexpr_parse,
		ineq_failure, false);

    printf("All tests are done gracefully\n");

    return 0;
}
