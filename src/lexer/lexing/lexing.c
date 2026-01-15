#include <stdbool.h>
#include <stddef.h>
#include <string.h>
#include <stdlib.h>
#include <err.h>

#include "utils/malloc_err/malloc_err.h"
#include "utils/check_char/check_char.h"
/* ------------------------------ */
/* -- LEXING THE VARIOUS CASES -- */
/* ------------------------------ */
char *lexing_words(char *input, size_t pos)
{
    size_t capacity = 8;
    char *tok = malloc_err(capacity);

    size_t tok_pos = 0;
    while (pos < strlen(input) && check_char(input[pos]) == 42)
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

char *lexing_quotes(char *input, size_t pos)
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

char *lexing_double_quotes(char *input, size_t pos)
{
    size_t input_len = strlen(input);
    size_t capacity = 8;
    char *tok = malloc_err(capacity);

    size_t tok_pos = 0;
    while (pos < input_len && input[pos] != '\"')
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

char *lexing_redirection(char *input, size_t pos)
{
    char *tok = malloc_err(3);
    size_t tok_len = 0;

    tok[tok_len++] = input[pos++];

    if (input[pos - 1] == '<')
    {
        if (input[pos] && (input[pos] == '&' || input[pos] == '>'))
        {
            tok[tok_len++] = input[pos];
        }
    }

    else if (input[pos - 1] == '>')
    {
        if (input[pos]
            && (input[pos] == '&' || input[pos] == '|' || input[pos] == '>'))
        {
            tok[tok_len++] = input[pos];
        }
    }
    tok[tok_len] = '\0';

    return tok;
}

char *lexing_operators(char *input, size_t pos)
{
    char *tok = malloc_err(3);
    size_t tok_len = 0;

    tok[tok_len++] = input[pos++];

    if (input[pos - 1] == '&')
    {
        if (input[pos] && input[pos] == '&')
        {
            tok[tok_len++] = input[pos];
        }
    }

    else if (input[pos - 1] == '|')
    {
        if (input[pos] && input[pos] == '|')
        {
            tok[tok_len++] = input[pos];
        }
    }
    tok[tok_len] = '\0';

    return tok;
}

char *lexing_variables(char *input, size_t pos)
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
