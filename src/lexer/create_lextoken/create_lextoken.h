#ifndef CREATE_LEXTOKEN_H
#define CREATE_LEXTOKEN_H

#include "lexer/struct_lexer.h"

struct lextoken *create_lextoken_words(enum token tok, char *value);

struct lextoken *create_lextoken_commands(char *token);

struct lextoken *create_lextoken_single_char(char *token);

struct lextoken *create_lextoken_redirection(char *token);

struct lextoken *create_lextoken_operators(char *token);

#endif /* ! CREATE_LEXTOKEN_H */
