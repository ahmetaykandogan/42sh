#ifndef STRUCT_LEXER_H
#define STRUCT_LEXER_H

enum token
{
    IF, // 'if'
    THEN, // 'then'
    ELIF, // 'elif'
    ELSE, // 'else'
    FI, // 'fi'

    COLUMN, // ';'
    NEWLINE, // '\n'

    RED_IN, // '<'
    RED_OUT, // '>'
    APPEND_OUT, // '>>'
    DUP_IN, // '<&'
    DUP_OUT, // '>&'
    RED_FORCE, // '>|'
    FD_RW, // '<>'

    PIPE, // '|'
    NEGA, // '!'

    WHILE, // 'while'
    UNTIL, // 'until'
    FOR, // 'for'
    IN, // 'in'
    DO, // 'do'
    DONE, // 'done'

    AND, // '&&'
    OR, // '||'

    GUILLMT, // ' "" '
    ESCAPE, // ' \ '
    APSTRPH, // ' '' '

    VAR, // '$name'
    BRK_VAR, // '{$name}'

    WORDS // 'words'
};

struct lextoken
{
    enum token tok;
    char *value;
};

#endif /* ! STRUCT_LEXER_H */
