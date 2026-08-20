#include "Headers/lexer.h"
#include "Headers/parser.h"
#include "Headers/eval.h"
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

Token* lexical_analysis(const char *c) {
    printf("===== Lexical Analysis =====\n\n");

    printf("Input Script:\n");
    printf("%s\n\n", c);
    printf("Lexical Analysis Output:\n\n");

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

        printf("Type: %2d, Column: %2d, Line: %2d, Value: %g\n", t.type, t.column, t.line, t.value);
    } while (t.type != TOKEN_EOF && current_token < token_count);
    printf("\nTotal Tokens: %2ld\n", total_tokens);

    if (t.type == TOKEN_UNKOWN) {
        printf("Unkown token");
    }
    printf("\n===== End of Lexical Analysis =====\n\n");
    return output;
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
        return 0;
    }

    const char *source_path = argv[1];
    const char *source_code = read_file_contents(source_path);
    Token *t = lexical_analysis(source_code);

    int64_t imp_count;
    printf("===== Imp Mul Resolvement =====\n\n");
    Token *imp = resolve_imp_mul(t, total_tokens, &imp_count);
    //free(t);
    print_tokens(imp, imp_count);
    printf("===== End of Imp Mul Resolvement =====\n\n");
    resolve_arity(imp, imp_count);
    printf("===== Arity Resolvement =====\n\n");
    print_tokens(imp, -1);
    printf("===== End of Arity Resolvement =====\n\n");
    int64_t out_count;
    Token *postfix = shunting_yard(imp, imp_count, &out_count);
    free(imp);
    printf("===== Postfix Conversion =====\n\n");
    print_tokens(postfix, out_count);
    printf("===== End of Postfix Conversion =====\n\n");
    double result = evaluate_postfix_throwaway(postfix, t, out_count);
    free(postfix);

    printf("Result: %g\n\n", result);

    return 0;
}