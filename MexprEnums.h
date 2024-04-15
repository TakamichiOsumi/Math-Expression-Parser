#ifndef __MEXPR_ENUMS_
#define __MEXPR_ENUMS_

typedef enum mexpr_operator {
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
    POW,

    SIN,
    COS,
    SQR,
    SQRT,

    /* Inequality operators */
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
} mexpr_operator;

typedef enum mexpr_data_type {

    INVALID_DATA_TYPE = MAX_OPERATOR + 1,

    MIN_DATA_TYPE,

    /* Basic types */
    INT,
    DOUBLE,
    VARIABLE,

    MAX_DATA_TYPE,

} mexpr_data_type;

char *Mexpr_get_string_token(int token_code);
int Mexpr_operator_precedence(int token_code);

#endif
