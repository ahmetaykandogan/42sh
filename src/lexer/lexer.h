#ifndef LEXER_H
#define LEXER_H

#include "../utils/fifo/fifo.h"

enum token
{
    IF,
    THEN,
    ELIF,
    ELSE,
    FI,

    COLUMN,
    NEWLINE,

    APSTRPH,
    WORDS
};

struct lextoken
{
    enum token tok;
    char *value;
};

char *read_file(char *file, char *script);

struct fifo *lexer(char *input);

struct fifo *lexing_from_file(char *path);
struct fifo *lexing_from_stdin(FILE *f);

#endif /* ! LEXER_H */
