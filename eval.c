#include "Headers/lexer.h"
#include <assert.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Frees passed throwaway array
double evaluate_postfix_throwaway(const Token *postfix, Token *throwaway, int64_t token_count) {
    if (token_count == 0) { fprintf(stderr, "0 tokens passed, lol.\n"); exit(EXIT_FAILURE);}
    assert(postfix[token_count - 1].type == TOKEN_EOF); // Fail if last token isn't EOF
    memset(throwaway, 0, sizeof(Token) * (unsigned long)token_count);
    static void *dispatch_table[TOKEN_COUNT] = {
        [TOKEN_NUM_LIT] = &&eval_operand,
        [TOKEN_PLUS] = &&eval_error, [TOKEN_MINUS] = &&eval_error,
        [TOKEN_LPAREN] = &&eval_error, [TOKEN_RPAREN] = &&eval_error,
        [TOKEN_ROOT] = &&eval_unop, [TOKEN_EXP] = &&eval_binop,
        [TOKEN_POS] = &&eval_unop, [TOKEN_NEG] = &&eval_unop,
        [TOKEN_IMPLICIT_MUL] = &&eval_binop, [TOKEN_MUL] = &&eval_binop, [TOKEN_DIV] = &&eval_binop, [TOKEN_REM] = &&eval_binop,
        [TOKEN_SUB] = &&eval_binop, [TOKEN_ADD] = &&eval_binop,
        [TOKEN_EOF] = &&do_eof,
    };

    int64_t i = 0;
    int64_t sp = 0;

    goto *dispatch_table[postfix[i].type];

eval_operand: {
    throwaway[sp++].value = postfix[i++].value;
    goto *dispatch_table[postfix[i].type];
}

eval_unop: {
    double top = throwaway[--sp].value;
    switch (postfix[i].type) {
        case TOKEN_ROOT: {
            if (top < 0) { fprintf(stderr, "Taking square root of a negative number"); exit(EXIT_FAILURE); }
            throwaway[sp++].value = sqrt(top);
            i++;
            break;
        }
        case TOKEN_POS: {
            throwaway[sp++].value = top;
            i++;
            break;
        }
        case TOKEN_NEG: {
            throwaway[sp++].value = -top;
            i++;
            break;
        }
        default: { fprintf(stderr, "How?"); exit(EXIT_FAILURE); }
    }
    goto *dispatch_table[postfix[i].type];
}

eval_binop: {
    double rhs = throwaway[--sp].value;
    double lhs = throwaway[--sp].value;

    switch (postfix[i].type) {
        case TOKEN_EXP: {
            throwaway[sp++].value = pow(lhs, rhs);
            i++;
            break;
        }
        case TOKEN_IMPLICIT_MUL:
        case TOKEN_MUL: {
            throwaway[sp++].value = lhs * rhs;
            i++;
            break;
        }
        case TOKEN_DIV: {
            if (rhs == 0) { fprintf(stderr, "Division by zero"); exit(EXIT_FAILURE); }
            throwaway[sp++].value = lhs / rhs;
            i++;
            break;
        }
        case TOKEN_REM: {
            if (rhs == 0) { fprintf(stderr, "No remainder when dividing by zero"); exit(EXIT_FAILURE); }
            throwaway[sp++].value = fmod(lhs, rhs);
            i++;
            break;
        }
        case TOKEN_SUB: {
            throwaway[sp++].value = lhs - rhs;
            i++;
            break;
        }
        case TOKEN_ADD: {
            throwaway[sp++].value = lhs + rhs;
            i++;
            break;
        }
        default: { fprintf(stderr, "How?"); exit(EXIT_FAILURE); }
    }
    goto *dispatch_table[postfix[i].type];
}

eval_error: {
    fprintf(stderr, "Please run evaluation last\n");
    exit(EXIT_FAILURE);
}

do_eof: {
    assert(sp == 1);
    double result = throwaway[sp - 1].value;
    free(throwaway);
    return result;
}   }