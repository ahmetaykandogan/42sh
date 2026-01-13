#include <ctype.h>
#include <err.h>
#include <string.h>
#include <stdlib.h>

#include "../utils/malloc_err.h"
#include "../utils/fifo.h"
#include "read_file.h"
#include "struct_lexer.h"

static struct lextoken *create_lextoken(enum token tok, char *token)
{
    free(token);

    struct lextoken *l_token = malloc_err(sizeof(struct lextoken));
    l_token->tok = tok;
    l_token->value = NULL;

    return l_token;
}

static struct lextoken *create_lextoken_words(enum token tok, char *value)
{
    struct lextoken *l_token = malloc_err(sizeof(struct lextoken));
    l_token->tok = tok;
    if (!value)
        l_token->value = NULL;

    else
        l_token->value = value;

    return l_token;
}

static struct lextoken *lexing_case(char *token)
{
    /* IF THEN ELIF ELSE FI */
    if (strcmp(token, "if") == 0) // IF
    {
        return create_lextoken(IF, token);
    }
    else if (strcmp(token, "then") == 0) // THEN
    {
        return create_lextoken(THEN, token);
    }
    else if (strcmp(token, "elif") == 0) // ELIF
    {
        return create_lextoken(ELIF, token);
    }
    else if (strcmp(token, "else") == 0) // ELSE
    {
        return create_lextoken(ELSE, token);
    }
    else if (strcmp(token, "fi") == 0) // FI
    {
        return create_lextoken(FI, token);
    }

    /* WHILE UNTIL FOR IN DO DONE */
    else if (strcmp(token, "while") == 0) // WHILE
    {
        return create_lextoken(WHILE, token);
    }
    else if (strcmp(token, "until") == 0) // UNTIL
    {
        return create_lextoken(UNTIL, token);
    }
    else if (strcmp(token, "for") == 0) // FOR
    {
        return create_lextoken(FOR, token);
    }
    else if (strcmp(token, "in") == 0) // IN
    {
        return create_lextoken(IN, token);
    }
    else if (strcmp(token, "do") == 0) // DO
    {
        return create_lextoken(DO, token);
    }
    else if (strcmp(token, "done") == 0) // DONE
    {
        return create_lextoken(DONE, token);
    }

    /* COLUMN NEWLINE */
    else if (strcmp(token, ";") == 0) // COLUMN
    {
        return create_lextoken(COLUMN, token);
    }
    else if (strcmp(token, "\n") == 0) // NEWLINE
    {
        return create_lextoken(NEWLINE, token);
    }

    /* DEFAULT CASE : WORDS */
    return create_lextoken_words(WORDS, token);
}

// Should add the following characters for the next steps: | & < >
static int check_char(char c)
{
    if (c == ';' || c == '\n' || c == '\'' || c == ' ' || c == '\0' || c == '!' || c == '&' || c == '|' || c == '<' || c == '>')
        return 0;
    if (c == '\'')
        return 1;
    return 2;
}

static char *lexing_words(char *input, size_t pos)
{
    size_t capacity = 8;
    char *tok = malloc_err(capacity);

    size_t tok_pos = 0;
    while (pos < strlen(input) && check_char(input[pos]) == 2)
    {
        if (tok_pos >= capacity - 1)
        {
            capacity *= 2;
            tok = realloc_err(tok, capacity);
        }
        tok[tok_pos++] = input[pos++];
    }
    if (tok_pos >= capacity)
    {
        capacity *= 2;
        tok = realloc_err(tok, capacity);
    }
    tok[tok_pos] = '\0';
    tok = realloc_err(tok, tok_pos + 1);

    return tok;
}

static char *lexing_quotes(char *input, size_t pos)
{
    size_t input_len = strlen(input);
    size_t capacity = 8;
    char *tok = malloc_err(capacity);

    size_t tok_pos = 0;
    while (pos < input_len && input[pos] != '\'')
    {
        if (tok_pos >= capacity - 1)
        {
            capacity *= 2;
            tok = realloc_err(tok, capacity);
        }
        tok[tok_pos++] = input[pos++];
    }
    if (pos >= input_len)
    {
        free(tok);
        err(2, "Quote not closed");
    }
    if (tok_pos >= capacity)
    {
        capacity *= 2;
        tok = realloc_err(tok, capacity);
    }
    tok[tok_pos] = '\0';
    tok = realloc_err(tok, tok_pos + 1);

    return tok;
}

struct fifo *lexer(char *input)
{
    struct fifo *queue = fifo_init();

    if (!input || input[0] == '\0')
    {
        return queue;
    }

    size_t input_len = strlen(input);
    size_t i = 0;

    while (i < input_len)
    {
        if (isspace(input[i]))
        {
            i++;
            continue;
        }

        /* CASE WORDS */
        if (check_char(input[i]) == 2)
        {
            char *tok = lexing_words(input, i);
            i += strlen(tok);

            fifo_push(queue, lexing_case(tok));
        }

        /* CASE APOSTROPHE QUOTE */
        else if (check_char(input[i]) == 1)
        {
            i++; // on skip la premiere quote

            char *tok = lexing_quotes(input, i);
            i += strlen(tok);

            i++; // on skip la seconde quote

            fifo_push(queue, create_lextoken_words(APSTRPH, tok));
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

    if (!script)
    {
        return fifo_init();
    }

    struct fifo *lex_queue = lexer(script);
    free(script);

    return lex_queue;
}

struct fifo *lexing_from_stdin(FILE *f)
{
    char *script = read_file(f);

    if (!script)
    {
        return fifo_init();
    }

    struct fifo *lex_queue = lexer(script);
    free(script);

    return lex_queue;
}
