#include "Headers/lexer.h"
#include "Headers/parser.h"
#include "Headers/eval.h"
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define unlikely(a) __builtin_expect(!!(a), 0)

Token* lexical_analysis(const char *c, bool dump_tokens) {
    if (unlikely(dump_tokens == true)) { printf("===== Lexical Analysis =====\n\n"); }
    printf("\nInfix Script:\n");
    printf("%s\n", c);
    if (unlikely(dump_tokens == true)) {  printf("Lexical Analysis Output:\n\n"); }
    lexer_init(c);

    Token count_tokens;
    do { count_tokens = lexer_next_token(); } while (count_tokens.type != TOKEN_EOF && count_tokens.type != TOKEN_UNKOWN);

    Token *output = malloc(sizeof(Token) * (unsigned long)total_tokens);
    if (!output) { fprintf(stderr, "Failed to allocate memory for output tokens\n"); exit(EXIT_FAILURE); }
    if (output != NULL) { memset(output, 0, sizeof(Token) * (unsigned long)total_tokens); }
    int64_t token_count = total_tokens;
    lexer_init(c);

    int64_t current_token = 0;

    Token t;
    do {
        t = lexer_next_token();
        output[current_token++] = t;

        if (unlikely(dump_tokens == true)) { printf("Type: %2d, Column: %2d, Line: %2d, Value: %g\n", t.type, t.column, t.line, t.value); }
    } while (t.type != TOKEN_EOF && current_token < token_count);
    if (unlikely(dump_tokens == true)) {
        printf("\nTotal Tokens: %2ld\n", total_tokens);
        printf("\n===== End of Lexical Analysis =====\n\n"); }
    return output;
}

static inline void print_actual_token(Token t) {
    char *space = " ";
    switch (t.type) {
        case TOKEN_NUM_LIT: printf("%g%s",t.value,  space); break;
        case TOKEN_ROOT: printf("./%s", space); break;
        case TOKEN_EXP: printf("^%s", space); break;
        case TOKEN_PLUS: case TOKEN_ADD: printf("+%s", space); break;
        case TOKEN_POS: printf("+(pos)%s", space); break;
        case TOKEN_MINUS: case TOKEN_SUB: printf("-%s", space); break;
        case TOKEN_NEG: printf("-(neg)%s", space); break;
        case TOKEN_IMPLICIT_MUL: case TOKEN_MUL: printf("*%s", space); break;
        case TOKEN_DIV: printf("/%s", space); break;
        case TOKEN_REM: printf("%%%s", space); break;
        case TOKEN_EOF: printf("EOF"); return;
        default: fprintf(stderr, "How?"); exit(EXIT_FAILURE);
    }
}

static char *read_file_contents(const char *path) {
    FILE *file = fopen(path, "rb");
    if (!file) {
        fprintf(stderr, "Could not open source file '%s'\n", path);
        exit(EXIT_FAILURE);
    }

    fseek(file, 0L, SEEK_END);
    int64_t raw_size = ftell(file);
    if (raw_size < 0) {
        fprintf(stderr, "Failed to determine file size for '%s'\n", path);
        fclose(file);
        exit(EXIT_FAILURE);
    }
    const size_t file_size = (size_t)raw_size;
    rewind(file);

    char *buffer = malloc(file_size + 1);
    if (!buffer) { fprintf(stderr, "Failed to allocate memory for file buffer\n"); exit(EXIT_FAILURE); }

    const size_t bytes_read = fread(buffer, sizeof(char), file_size, file);
    buffer[bytes_read] = '\0';

    fclose(file);
    return buffer;
}

static inline void print_tokens(Token *t, int64_t tokens) {
    int64_t current_token_num = 0;
    Token current_token;
    if (tokens < 0) { tokens = total_tokens; }
    while (current_token_num < tokens) {
        current_token = t[current_token_num];
        printf("Type: %2d, Column: %2d, Line: %2d, Value: %g\n", current_token.type, current_token.column, current_token.line, current_token.value);
        current_token_num++;
    }
    printf("\n");
}

int main(const int argc, char **argv) {
    if (argc < 2) {
        printf("Please provide a file");
        return 1;
    }

    bool dump_tokens = false;

    const char *source_path = argv[1];
    for (int64_t i = 2; i < argc; i++) {
        if (strcmp(argv[i], "-d") == 0) {
            dump_tokens = true;
        }
    }

    const char *source_code = read_file_contents(source_path);
    Token *t = lexical_analysis(source_code, dump_tokens);

    int64_t imp_count;
    if (unlikely(dump_tokens == true)) { printf("===== Imp Mul Resolvement =====\n\n"); }
    Token *imp = resolve_imp_mul(t, total_tokens, &imp_count);
    if (unlikely(dump_tokens == true)) {
        print_tokens(imp, imp_count);
        printf("===== End of Imp Mul Resolvement =====\n\n");
    }
    resolve_arity(imp, imp_count);
    if (unlikely(dump_tokens == true)) {
        printf("===== Arity Resolvement =====\n\n");
        print_tokens(imp, -1);
        printf("===== End of Arity Resolvement =====\n\n");
    }
    int64_t out_count;
    Token *postfix = shunting_yard(imp, imp_count, &out_count);
    free(imp);
    if (unlikely(dump_tokens == true)) {
        printf("===== Postfix Conversion =====\n\n");
        print_tokens(postfix, out_count);
        printf("===== End of Postfix Conversion =====\n\n");
    }
    double result = evaluate_postfix_throwaway(postfix, t, out_count);
    printf("\nPostfix:\n");
    int64_t current_token = 0;
    do {
        print_actual_token(postfix[current_token++]);
    } while (postfix[current_token].type != TOKEN_EOF && current_token < out_count);
    free(postfix);

    printf("\n\nResult: %g\n\n", result);

    return 0;
}