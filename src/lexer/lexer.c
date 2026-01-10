#include "lexer.h"

#include <string.h>

#include "../utils/extra/malloc_err.h"
#include "read_file.h"

static struct lextoken *create_lextoken(enum token tok, char *value)
{
    struct lextoken *token = malloc_err(sizeof(struct lextoken));
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
    if (strcmp(token, "if") == 0)
        return create_lextoken(IF, NULL);

    else if (strcmp(token, "then") == 0)
        return create_lextoken(THEN, NULL);

    else if (strcmp(token, "elif") == 0)
        return create_lextoken(ELIF, NULL);

    else if (strcmp(token, "else") == 0)
        return create_lextoken(ELSE, NULL);

    else if (strcmp(token, "fi") == 0)
        return create_lextoken(FI, NULL);

    /* COLUMN NEWLINE APSTRPH */
    else if (strcmp(token, ";") == 0)
        return create_lextoken(COLUMN, NULL);

    else if (strcmp(token, "\n") == 0)
        return create_lextoken(NEWLINE, NULL);

    else if (strcmp(token, "\'") == 0)
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
    char *script = read_file(open_file(path));
    struct fifo *lex_queue = lexer(script);

    return lex_queue;
}

struct fifo *lexing_from_stdin(FILE *f)
{
    char *script = read_file(f);
    struct fifo *lex_queue = lexer(script);

    return lex_queue;
}
