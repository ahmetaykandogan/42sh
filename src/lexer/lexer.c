#include "lexer.h"

#include <string.h>

char *read_file(char *file, char *script)
{
    /* lire le fichier et mettre tout le contenu dans un tableau (string) */
    // TODO
}

static struct lextoken *create_lextoken(enum token tok, char *value)
{
    struct lextoken *token;
    token->tok = tok;
    if (!value)
        token->value = NULL;

    else
        token->value = value;

    return token;
}

static struct lextoken *lexing_case(char *token)
{
    /* IF THEN ELIF ELSE FI */
    if (strcmp(token, "if"))
        return create_lextoken(IF, NULL);

    else if (strcmp(token, "then"))
        return create_lextoken(THEN, NULL);

    else if (strcmp(token, "elif"))
        return create_lextoken(ELIF, NULL);

    else if (strcmp(token, "else"))
        return create_lextoken(ELSE, NULL);

    else if (strcmp(token, "fi"))
        return create_lextoken(FI, NULL);

    /* COLUMN NEWLINE APSTRPH */
    else if (strcmp(token, ";"))
        return create_lextoken(COLUMN, NULL);

    else if (strcmp(token, "\n"))
        return create_lextoken(NEWLINE, NULL);

    else if (strcmp(token, "\'"))
        return create_lextoken(APSTRPH, NULL);

    /* DEFAULT CASE : WORDS */
    return create_lextoken(WORDS, token);
}

struct fifo *lexer(char *input)
{
    struct fifo *queue = fifo_init();

    char *saveptr = NULL;
    char *token = strtok_r(input, " ", &saveptr);
    while (token)
    {
        fifo_push(queue, lexing_case(token));
        token = strtok_r(NULL, " ", &saveptr);
    }

    return queue;
}

struct fifo *lexing_from_file(char *path)
{
    char *script = malloc(sizeof(char *));
    read_file(path, script);
    struct fifo *lex_queue = lexer(script);
    free(script);

    return lex_queue;
}
