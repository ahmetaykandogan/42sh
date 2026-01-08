#ifndef LEXER_H
#define LEXER_H

#include "../utils/fifo/fifo.h"

// strtok() jusqu'a un ["] puis on fait une fonction de lexing de string 

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

struct lex
{
    enum token tok;
    char *value;
};

char *read_file(char *file, char *script);

struct lexer lexer(char *input);

#endif /* ! LEXER_H */
