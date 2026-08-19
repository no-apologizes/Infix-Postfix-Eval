#pragma once
#include <stdint.h>
#include <assert.h>
#include <limits.h>
static_assert(sizeof(double) * CHAR_BIT == 64, "Double must be 64 bits");

extern int64_t total_tokens;

typedef enum {
    TOKEN_NUM_LIT,
    TOKEN_MINUS, // '-' for lexer
    TOKEN_PLUS,  // '+' for lexer

    // Grouped in terms of precedence
    TOKEN_LPAREN,
    TOKEN_RPAREN,

    TOKEN_ROOT, // Square Root: ./a
    TOKEN_EXP,  // Exponentiation:a^a

    TOKEN_POS,  // Positive: +a
    TOKEN_NEG,  // Negation: -a

    TOKEN_IMPLICIT_MUL, // 2()
    TOKEN_MUL, // a * a
    TOKEN_DIV, // a / a
    TOKEN_REM, // a % a

    TOKEN_SUB, // a - a
    TOKEN_ADD, // a + a

    TOKEN_UNKOWN,
    TOKEN_EOF,

    TOKEN_COUNT
} TokenType;

typedef struct {
    double value;
    TokenType type;
    int64_t line;
    int64_t column;
} Token;

void lexer_init(const char *source);
Token lexer_next_token(void);