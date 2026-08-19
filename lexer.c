#include "Headers/lexer.h"
#include <stdbool.h>
#include <stddef.h>
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

int64_t total_tokens = 0;

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
        //S.cursor++; // Added
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
    total_tokens = 0;
}

Token lexer_next_token(void) {
    if (unlikely(!table_initialized)) {
        for (size_t i = 0; i < 256; i++) {
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
    double value = 0;
    while (1) {
        char next = peek();
        if (next >= '0' && next <= '9') {
            value = value * 10 + (next - '0');
            S.cursor++;
            S.column++;
        } else {
            break;
        }
    }
    if (peek() == '.' && S.cursor[1] >= '0' && S.cursor[1] <= '9') {
        S.cursor++;
        S.column++;
        double divisor = 0.1;
        while (1) {
            char next = peek();
            if (next >= '0' && next <= '9') {
                value = value + (next - '0') * divisor;
                divisor *= 0.1;
                S.cursor++;
                S.column++;
            } else {
                break;
            }
        }
    }
    if (unlikely(peek() == '.') && S.cursor[1] != '/') {
        fprintf(stderr, "Leading period at position line %ld, column %ld", S.line, S.column);
        exit(EXIT_FAILURE);
    }
    token.type = TOKEN_NUM_LIT;
    token.value = value;
    total_tokens++;
    return token;
}

lex_dot: {
    if (S.cursor[1] == '/') { // Check for "./"
        advance();
        advance();
    } else if (S.cursor[1] >= '0' && S.cursor[1] <= '9') {
        S.cursor++;
        S.column++;
        double value = 0;
        double divisor = 0.1;
        while (1) {
            char next = peek();
            if (next >= '0' && next <= '9') {
                value = value + (next - '0') * divisor;
                divisor *= 0.1;
                S.cursor++;
                S.column++;
            } else {
                break;
            }
        }
        token.type = TOKEN_NUM_LIT;
        token.value = value;
        total_tokens++;
        return token;
    }
    else {
        fprintf(stderr, "Unexpected '.', did you mean './'?");
        exit(EXIT_FAILURE);
    }
    token.type = TOKEN_ROOT;
    total_tokens++;
    return token;
}

lex_lparen: {
    advance();
    token.type = TOKEN_LPAREN;
    total_tokens++;
    return token;
}

lex_rparen: {
    advance();
    token.type = TOKEN_RPAREN;
    total_tokens++;
    return token;
}

lex_exp: {
    advance();
    token.type = TOKEN_EXP;
    total_tokens++;
    return token;
}

lex_mul: {
    advance();
    token.type = TOKEN_MUL;
    total_tokens++;
    return token;
}

lex_div: {
    advance();
    token.type = TOKEN_DIV;
    total_tokens++;
    return token;
}

lex_rem: {
    advance();
    token.type = TOKEN_REM;
    total_tokens++;
    return token;
}

lex_minus: {
    advance();
    token.type = TOKEN_MINUS;
    total_tokens++;
    return token;
}

lex_plus: {
    advance();
    token.type = TOKEN_PLUS;
    total_tokens++;
    return token;
}

lex_unknown: {
    fprintf(stderr, "Invalid character, Column: %2ld", token.column);
    exit(EXIT_FAILURE);
}

lex_eof: {
    token.type = TOKEN_EOF;
    total_tokens++;
    return token;
}   }