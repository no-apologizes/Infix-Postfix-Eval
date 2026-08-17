#include "Headers/lexer.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define likely(a) __builtin_expect(!!(a), 1)
#define unlikely(a) __builtin_expect(!!(a), 0)

typedef struct {
    const char *cursor;
    int64_t line;
    int64_t column;
} LexerState;

static LexerState S;

static const void *dispatch_table[256];
static bool table_initialized = false;

static inline char advance(void) {
    char c = *S.cursor;
    if (likely((c != '\0'))) {
        S.cursor++;
        S.column++;
        if (c == '\n') {
            S.line++;
            S.column = 1;
        }
    } else {
        S.column++;
    }
    return c;
}

static inline char peek(void) {
    return *S.cursor;
}

static inline void skip_whitespace(void) {
    while (1) {
        char c = peek();
        if (c == ' '|| c == '\t' || c == '\r') { // Purpose fallthorugh to lex_eof
            S.cursor++;
            S.column++;
        } else if (c == '\n') {
            S.cursor++;
            S.line++;
            S.column = 1;
        } else {
            break;
        }}
}

void lexer_init(const char *source) {
    S.cursor = source;
    S.line = 1;
    S.column = 1;
}

Token lexer_next_token(void) {
    if (unlikely(!table_initialized)) {
        for (int64_t i = 0; i < 256; i++) {
            if (i >= '0' && i <= '9') {
                dispatch_table[i] = &&lex_num_lit;
            } else {
                dispatch_table[i] = &&lex_unknown;
            }
        }
        dispatch_table['.'] = &&lex_dot;

        dispatch_table['('] = &&lex_lparen;
        dispatch_table[')'] = &&lex_rparen;

        dispatch_table['^'] = &&lex_exp;

        dispatch_table['*'] = &&lex_mul;
        dispatch_table['/'] = &&lex_div;
        dispatch_table['%'] = &&lex_rem;

        dispatch_table['+'] = &&lex_plus;
        dispatch_table['-'] = &&lex_minus;

        dispatch_table['\0'] = &&lex_eof;

        table_initialized = true;
    }
    skip_whitespace();

    Token token;
    token.line = S.line;
    token.column = S.column;
    token.value = 0; // Initialize to 0

    char c = peek();

    goto *dispatch_table[(unsigned char)c];

lex_num_lit: {
    int64_t value = 0;
    while (1) {
        char next = peek();
        if (next >= '0' && next <= '9') {
            int64_t digit = next - '0';
            if (value > (9223372036854775807 - digit) / 10) {
                fprintf(stderr, "Number literal too large");
                exit(EXIT_FAILURE);
            }
            value = value * 10 + digit;
            S.cursor++;
            S.column++;
        } else {
            break;
        }
    }
    if (peek() == '.') {
        fprintf(stderr, "Only 64-bit integers bucko");
        exit(EXIT_FAILURE);
    }
    token.type = TOKEN_NUM_LIT;
    token.value = value;
    return token;
}

lex_dot: {
    if (S.cursor[1] == '/') { // Check for "./"
        advance();
        advance();
    } else {
        fprintf(stderr, "Unexpected '.', did you mean './'?");
        exit(EXIT_FAILURE);
    }
    token.type = TOKEN_ROOT;
    return token;
}

lex_lparen: {
    advance();
    token.type = TOKEN_LPAREN;
    return token;
}

lex_rparen: {
    advance();
    token.type = TOKEN_RPAREN;
    return token;
}

lex_exp: {
    advance();
    token.type = TOKEN_EXP;
    return token;
}

lex_mul: {
    advance();
    token.type = TOKEN_MUL;
    return token;
}

lex_div: {
    advance();
    token.type = TOKEN_DIV;
    return token;
}

lex_rem: {
    advance();
    token.type = TOKEN_REM;
    return token;
}

lex_minus: {
    advance();
    token.type = TOKEN_MINUS;
    return token;
}

lex_plus: {
    advance();
    token.type = TOKEN_PLUS;
    return token;
}

lex_unknown: {
    fprintf(stderr, "Invalid character, Column: %2ld", token.column);
    exit(EXIT_FAILURE);
}

lex_eof: {
    token.type = TOKEN_EOF;
    return token;
}
}