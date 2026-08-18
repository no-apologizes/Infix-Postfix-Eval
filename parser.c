#include "Headers/lexer.h"
#include "Headers/parser.h"
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define likely(a) __builtin_expect(!!(a), 1)

typedef struct {
    Token *elements;
    int64_t top;
} TokenStack;

/*
 * Was the last token the start of input, lparen, or any operator?
 * If yes, unary
 * Was the last token num_lit or rparen?
 * If yes, binary
 */
void resolve_arity(Token *t, int64_t token_count) {
    int64_t i = 0;
    TokenType preceding = TOKEN_COUNT;
    TokenType current = t[i].type;

    while (likely(current != TOKEN_EOF && i != token_count)) {
        if (current == TOKEN_MINUS || current == TOKEN_PLUS) {
            if (preceding != TOKEN_NUM_LIT && preceding != TOKEN_RPAREN
            ) {
                if (current == TOKEN_MINUS) { t[i].type = TOKEN_NEG; preceding = TOKEN_NEG; }
                if (current == TOKEN_PLUS)  { t[i].type = TOKEN_POS; preceding = TOKEN_POS; }
                i++;
                current = t[i].type;
            }
            else if (preceding == TOKEN_NUM_LIT || preceding == TOKEN_RPAREN) {
                if (current == TOKEN_MINUS) { t[i].type = TOKEN_SUB; preceding = TOKEN_SUB; }
                if (current == TOKEN_PLUS)  { t[i].type = TOKEN_ADD; preceding = TOKEN_ADD; }
                i++;
                current = t[i].type;
            }
        } else {
            preceding = t[i].type;
            i++;
            current = t[i].type;
        }}
}

static inline void stack_push(TokenStack *s, Token t) {
    s->elements[++s->top] = t;
}

static inline Token stack_pop(TokenStack *s) {
    return s->elements[s->top--];
}

static inline Token peek(TokenStack *s) {
    return s->elements[s->top];
}

static inline int64_t stack_empty(TokenStack *s) {
    return s->top < 0;
}

static inline int64_t get_prec(TokenType t) { // precedence
    switch (t) {
        case TOKEN_ADD: case TOKEN_SUB: return 1;
        case TOKEN_REM: case TOKEN_DIV: case TOKEN_MUL: case TOKEN_IMPLICIT_MUL: return 2;
        case TOKEN_NEG: case TOKEN_POS: return 3;
        case TOKEN_EXP: case TOKEN_ROOT: return 4;
        default: return -1; // Not an operator
    }
}

static inline int64_t is_right_associative(TokenType t) {
    return t == TOKEN_NEG || t == TOKEN_POS || t == TOKEN_EXP || t == TOKEN_ROOT;
}

/* https://mathcenter.oxford.emory.edu/site/cs171/shuntingYardAlgorithm/ ------------------------------------------------------
 * If the incoming symbols is an operand, print it.
 * If the incoming symbol is a left parenthesis, push it on the stack.
 *
 * If the incoming symbol is a right parenthesis:
 * discard the right parenthesis, pop and print the stack symbols until you see a left parenthesis. Pop the left parenthesis and discard it.
 *
 * If the incoming symbol is an operator and the stack is empty or contains a left parenthesis on top, push the incoming operator onto the stack.
 *
 * If the incoming symbol is an operator and has either higher precedence than the operator on the top of the stack,
 * or has the same precedence as the operator on the top of the stack and is right associative,
 * or if the stack is empty, or if the top of the stack is "(" (a floor) -- push it on the stack.
 *
 * If the incoming symbol is an operator and has either lower precedence than the operator on the top of the stack,
 * or has the same precedence as the operator on the top of the stack and is left associative -- continue to pop the stack until this is not true.
 * Then, push the incoming operator.
 *
 * At the end of the expression, pop and print all operators on the stack. (No parentheses should remain.)
 */
Token *shunting_yard(Token *tokens, int64_t token_count, int64_t *out_count) {
    if (token_count == 0) { fprintf(stderr, "0 tokens passed, lol."); exit(EXIT_FAILURE);}
    assert(tokens[token_count - 1].type == TOKEN_EOF); // Fail if last token isn't EOF
    Token *output = malloc(sizeof(Token) * (unsigned long)token_count);
    if (!output) { fprintf(stderr, "Failed to allocate memory for output stack"); exit(EXIT_FAILURE); }
    if (output != NULL) { memset(output, 0, sizeof(Token) * (unsigned long)token_count); } // No parallelized for loops :(
    TokenStack op_stack;
    op_stack.elements = malloc(sizeof(Token) * (unsigned long)token_count);
    if (!op_stack.elements) { fprintf(stderr, "Failed to allocate memory for operator stack"); exit(EXIT_FAILURE); }
    if (op_stack.elements != NULL) { memset(op_stack.elements, 0, sizeof(Token) * (unsigned long)token_count); }
    op_stack.top = -1;

    static void *dispatch_table[TOKEN_COUNT] = {
        [TOKEN_NUM_LIT] = &&do_operand,
        [TOKEN_LPAREN] = &&do_lparen,
        [TOKEN_RPAREN] = &&do_rparen,
        [TOKEN_ROOT] = &&do_operator, [TOKEN_EXP] = &&do_operator,
        [TOKEN_POS] = &&do_operator, [TOKEN_NEG] = &&do_operator,
        [TOKEN_IMPLICIT_MUL] = &&do_operator, [TOKEN_MUL] = &&do_operator, [TOKEN_DIV] = &&do_operator, [TOKEN_REM] = &&do_operator,
        [TOKEN_SUB] = &&do_operator, [TOKEN_ADD] = &&do_operator,
        [TOKEN_EOF] = &&do_eof,
    };

    int64_t i = 0;
    int64_t out_pos = 0; // How many tokens are in the output so far

    goto *dispatch_table[tokens[i].type];

do_operand: {
    output[out_pos++] = tokens[i]; // Write to output at current write pos and then advance it
    i++; // Look at next token
    goto *dispatch_table[tokens[i].type];
}

do_operator: {
    TokenType current = tokens[i].type;
    while (
        !stack_empty(&op_stack) &&
        // Check for a boundary marker, operators can't reach across them
        peek(&op_stack).type != TOKEN_LPAREN &&
        // If operator binds atleast as tight, GE
        get_prec(peek(&op_stack).type) >= get_prec(current) &&
        // If it's the same prec, only pop when it's left associative
        !(get_prec(peek(&op_stack).type) == get_prec(current) && is_right_associative(current))
    ) {
        output[out_pos++] = stack_pop(&op_stack);
    }
    // Now push current operator to wait its turn now that everything its gone
    stack_push(&op_stack, tokens[i]);
    i++;
    goto *dispatch_table[tokens[i].type];
}

do_lparen: {
    stack_push(&op_stack, tokens[i]);
    i++; // Look at next token
    goto *dispatch_table[tokens[i].type];
}

do_rparen: {
    while (!stack_empty(&op_stack) && peek(&op_stack).type != TOKEN_LPAREN) {
        output[out_pos++] = stack_pop(&op_stack);
    }
    if (stack_empty(&op_stack)) {
        fprintf(stderr, "Mismatched Parentheses, missing '('?");
        exit(EXIT_FAILURE);
    }
    // Found '('
    stack_pop(&op_stack);
    i++;
    goto *dispatch_table[tokens[i].type];
}

do_eof: {
    while (!stack_empty(&op_stack)) {
        if (peek(&op_stack).type == TOKEN_LPAREN) {
            fprintf(stderr, "Mismatched Parentheses, missing ')'?");
            exit(EXIT_FAILURE);
        }
        output[out_pos++] = stack_pop(&op_stack);
    }
    free(op_stack.elements);
    *out_count = out_pos; // How many valid entries are in output, there may be discarded parentheses
    return output;
}   }