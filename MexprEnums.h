#ifndef __MEXPR_ENUMS_
#define __MEXPR_ENUMS_

typedef enum {
    INT, /* an integer */
    DOUBLE,
    VARIABLE, /* alphanumeric one world */
    WHITE_SPACE,
    DTYPE_END, /* the end indication of all enums */
} mexppr_dtype;

typedef enum {
    PARSER_EOF = (int) DTYPE_END + 1,
} extra_type;

char *Mexpr_get_string_token(int token_code);

#endif
