#ifndef LEXER_H
#define LEXER_H

// strtok() jusqu'a un ["] puis on fait une fonction de lexing de string 

enum token 
{
    IF,
    ELIF,
    ELSE,
    THEN,

    COLUMN,
    NEWLINE
};

struct lexer
{
    enum token tok;
    char *value;
};

char *read_file(char *file, char *script);

struct lexer lexer(char *input);

#endif /* ! LEXER_H */
