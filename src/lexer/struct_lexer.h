#ifndef STRUCT_LEXER_H
#define STRUCT_LEXER_H

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

#endif /* ! STRUCT_LEXER_H */
