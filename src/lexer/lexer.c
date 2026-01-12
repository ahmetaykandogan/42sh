#include <ctype.h>
#include <string.h>

#include "../utils/malloc_err.h"
#include "../utils/fifo.h"
#include "read_file.h"
#include "struct_lexer.h"

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
// Should add the following characters for the next steps: | & < >
int check_char(char c)
{
    if (c == ';' || c == '\n' || c == '\'' || c == ' ' || c == '\0')
        return 0;
    return 1;
}

// TODO: case for echo 'hello;hello'
struct fifo *lexer(char *input)
{
    struct fifo *queue = fifo_init();

    if (!input)
    {
        return NULL;
    }
    size_t i = 0;
    while (i < strlen(input))
    {
        if (isspace(input[i]))
        {
            i++;
        }
        if (check_char(input[i]))
        {
            char *tok = malloc_err(1 * sizeof(char));

            int tok_pos = 0;
            while (check_char(input[i]))
            {
                tok = realloc_err(tok, tok_pos + 1);
                tok[tok_pos++] = input[i++];
            }
            tok = realloc_err(tok, tok_pos + 1);
            tok[tok_pos] = '\0';
            fifo_push(queue, lexing_case(tok));
        }
        else
        {
            char *tok = malloc_err(2 * sizeof(char));
            tok[0] = input[i++];
            tok[1] = '\0';
            fifo_push(queue, lexing_case(tok));
        }
    }
    return queue;
}
// Old version of the lexer
/*
struct fifo *lexer(char *input)
{
    struct fifo *queue = fifo_init();


    while (token)
    {
        fifo_push(queue, lexing_case(token));
        token = strtok_r(NULL, " ", &saveptr);
    }

    return queue;
}
*/
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
