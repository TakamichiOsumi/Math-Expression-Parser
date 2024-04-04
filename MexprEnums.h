#ifndef __MEXPR_ENUMS_
#define __MEXPR_ENUMS_

typedef enum {
    MATH_INT, /* an integer */
    MATH_DOUBLE,
    MATH_VARIABLE, /* alphanumeric one world */
    MATH_DTYPE_END, /* the end indication of all enums */
} mexppr_dtype;

typedef enum {
    PARSER_EOF = (int) MATH_DTYPE_END + 1,
} extra_type;

char *Mexpr_get_string_token(int token_code);

#endif
