#ifndef LEXING_H
#define LEXING_H

#include <stddef.h>

char *lexing_words(char *input, size_t pos);

char *lexing_words_echo(char *input, size_t pos);

char *lexing_quotes(char *input, size_t pos);

char *lexing_double_quotes(char *input, size_t pos);

char *lexing_numbers(char *input, size_t pos);

char *lexing_redirection(char *input, size_t pos);

char *lexing_operators(char *input, size_t pos);

char *lexing_variables(char *input, size_t pos);

#endif /* ! LEXING_H */
