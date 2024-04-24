#ifndef __MEXPR_ENUMS_
#define __MEXPR_ENUMS_

#include <stdbool.h>

/*
 * List up all token enums without overlapping
 * with each other value.
 */
enum whitespace_symbol {
    INVALID = 0,
    WHITE_SPACE,
    TAB,
    PARSER_EOF,
};

enum punctuation_marker {
    BRACKET_START = PARSER_EOF + 1,
    BRACKET_END,
    COMMA,
};

enum reserved_key_word {
    TRUE = COMMA + 1,
    FALSE,
};

enum operator {
    /* Binary operator */
    PLUS = FALSE + 1,
    MINUS,
    MULTIPLY,
    DIVIDE,
    REMAINDER,
    MIN,
    MAX,
    POW,

    /* Unary operator */
    SIN,
    COS,
    SQR,
    SQRT,

    /* Inequality operator */
    GREATER_THAN_OR_EQUAL_TO,
    LESS_THAN_OR_EQUAL_TO,
    GREATER_THAN,
    LESS_THAN,
    NEQ,
    EQ,

    /* Logical operator */
    OR,
    AND,
};

enum data_type {
    INT = AND + 1,
    DOUBLE,
    VARIABLE,
    BOOLEAN,
};

char *get_string_token(int token_code);
int operator_precedence(int token_code);
bool is_skipped_token(int token_code);
bool is_operand(int token_code);
bool is_operator(int token_code);
bool is_unary_operator(int token_code);
bool is_binary_operator(int token_code);

#endif
