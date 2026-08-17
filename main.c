#include "Headers/lexer.h"
#include <stdio.h>

int main(void) {
    const char *test =
        "5(2 + ./4)2 / 6^2 - -2\n"
        "+ +5 * 1 % 1";

    printf("Test Script:\n");
    printf("%s\n\n", test);
    printf("Lexical Analysis Output:\n\n");

    lexer_init(test);

    Token t;
    do {
        t = lexer_next_token();

        printf("Type: %2d, Column: %2ld, Line: %2ld, Value: %ld\n", t.type, t.column, t.line, t.value);
    } while (t.type != TOKEN_EOF && t.type != TOKEN_UNKOWN);

    if (t.type == TOKEN_UNKOWN) {
        printf("Unkown token");
    }
}