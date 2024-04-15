#ifndef __MEXPR_ENUMS_
#define __MEXPR_ENUMS_

typedef enum Mexpr_Operators {
    INVALID = 0,

    MIN_OPERATOR,

    /* Symbols */
    BRACKET_START,
    BRACKET_END,
    COMMA,

    /* Arithmetic */
    PLUS,
    MINUS,
    MULTIPLY,
    DIVIDE,
    REMAINDER,

    /* Functions */
    MIN,
    MAX,
    SIN,
    COS,
    POW,

    /* INEQ operators */
    GREATER_THAN_OR_EQUAL_TO,
    GREATER_THAN,
    LESS_THAN_OR_EQUAL_TO,
    LESS_THAN,
    NEQ,
    EQ,

    /* TODO : Logical operators */
    /* OR, */
    /* AND, */

    WHITE_SPACE,
    TAB,
    PARSER_EOF,

    MAX_OPERATOR,
} Mexpr_operators;

typedef enum {

    MIN_DATA_TYPE = MAX_OPERATOR + 1,

    /* Basic types */
    INT,
    DOUBLE,
    VARIABLE,

    MAX_DATA_TYPE,

} Mexpr_data_types;

char *Mexpr_get_string_token(int token_code);
int Mexpr_operator_precedence(int token_code);

#endif
