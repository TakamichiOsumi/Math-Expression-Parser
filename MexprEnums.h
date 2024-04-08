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
    DIVIDE,
    REMAINDER,
    EQ,

    /* Functions */
    MIN, /* min() */
    MAX, /* max() */
    SIN, /* sin() */
    COS, /* cos() */
    POW, /* pow() */

    /* Ignored character */
    WHITE_SPACE,

    PARSER_EOF,
} mexpr_dtype;

char *Mexpr_get_string_token(int token_code);

#endif
