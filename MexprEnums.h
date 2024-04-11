#ifndef __MEXPR_ENUMS_
#define __MEXPR_ENUMS_

typedef enum {
    INVALID = 0,

    /* Basic types */
    INT,
    DOUBLE,
    VARIABLE,

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
    MIN, /* min() */
    MAX, /* max() */
    SIN, /* sin() */
    COS, /* cos() */
    POW, /* pow() */

    /* INEQ operators */
    NEQ,
    GREATER_THAN_OR_EQUAL_TO,
    GREATER_THAN,
    LESS_THAN_OR_EQUAL_TO,
    LESS_THAN,
    EQ,

    WHITE_SPACE,
    TAB,

    PARSER_EOF,
} mexpr_dtype;

char *Mexpr_get_string_token(int token_code);

#endif
