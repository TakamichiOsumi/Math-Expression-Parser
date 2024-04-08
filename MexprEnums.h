#ifndef __MEXPR_ENUMS_
#define __MEXPR_ENUMS_

typedef enum {
    INVALID = 0,

    /* Basic types */
    INT,
    DOUBLE,
    VARIABLE,

    /* Symbols */
    MATH_BRACKET_START,
    MATH_BRACKET_END,
    MATH_COMMA,

    /* Arithmetic */
    MATH_PLUS,
    MATH_MINUS,
    MATH_DIVIDE,
    MATH_REMAINDER,
    MATH_EQ,

    /* Functions */
    MATH_MIN, /* min() */
    MATH_MAX, /* max() */
    MATH_SIN, /* sin() */
    MATH_COS, /* cos() */
    MATH_POW, /* pow() */

    /* Ignored character */
    WHITE_SPACE,

    DTYPE_END, /* The end indication of all enums */
} mexppr_dtype;

typedef enum {
    PARSER_EOF = (int) DTYPE_END + 1,
} extra_type;

char *Mexpr_get_string_token(int token_code);

#endif
