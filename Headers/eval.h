#pragma once
#include "lexer.h"

double evaluate_postfix_throwaway(const Token *postfix, Token *throwaway, int64_t token_count);