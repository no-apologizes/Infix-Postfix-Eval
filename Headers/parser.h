#pragma once
#include "lexer.h"
void resolve_arity(Token *t, int64_t token_count);
Token *resolve_imp_mul(Token *token, int64_t token_count, int64_t *out_count);
Token *shunting_yard(Token *tokens, int64_t token_count, int64_t *out_count);