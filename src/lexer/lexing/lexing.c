#include <ctype.h>
#include <err.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "utils/check_char/check_char.h"
#include "utils/malloc_err/malloc_err.h"

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
        if (input[pos] == '\\' && pos + 1 < strlen(input))
        {
            if (tok_pos >= capacity - 1)
            {
                capacity *= 2;
                tok = realloc_err(tok, capacity);
            }
            tok[tok_pos++] = input[pos++];

            if (tok_pos >= capacity - 1)
            {
                capacity *= 2;
                tok = realloc_err(tok, capacity);
            }
            tok[tok_pos++] = input[pos++];
        }

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

char *lexing_numbers(char *input, size_t pos)
{
    size_t capacity = 4;
    char *tok = malloc_err(capacity);

    size_t tok_pos = 0;
    while (pos < strlen(input) && isdigit(input[pos]))
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

static char *lexing_special_variables(char *input, size_t pos)
{
    if (!input[pos])
    {
        char *tok = malloc_err(2);
        tok[0] = '$';
        tok[1] = '\0';
        return tok;
    }
    if (input[pos] == '@' || input[pos] == '*' || input[pos] == '?'
        || input[pos] == '$' || input[pos] == '#' || isdigit(input[pos]))
    {
        char *tok = malloc_err(3);
        tok[0] = '$';
        tok[1] = input[pos];
        tok[2] = '\0';
        return tok;
    }
    return NULL;
}

static char *lexing_variables_no_braces(char *input, size_t pos)
{
    char *special_var = lexing_special_variables(input, pos);
    if (special_var)
    {
        return special_var;
    }
    size_t capacity = 8;
    char *tok = malloc_err(capacity);

    size_t tok_pos = 0;
    while (pos < strlen(input) && (isalnum(input[pos]) || input[pos] == '_'))
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

char *lexing_variables(char *input, size_t pos)
{
    if (input[pos] && input[pos] != '{')
    {
        return lexing_variables_no_braces(input, pos);
    }
    pos++;

    size_t input_len = strlen(input);
    size_t capacity = 8;
    char *tok = malloc_err(capacity);

    size_t tok_pos = 0;

    bool single_quoted = 0;
    bool double_quoted = 0;
    while (pos < input_len)
    {
        char c = input[pos];

        if (c == '\\' && pos + 1 < input_len && !single_quoted)
        {
            if (tok_pos >= capacity - 2)
            {
                capacity *= 2;
                tok = realloc_err(tok, capacity);
            }
            tok[tok_pos++] = input[pos++;]
            tok[tok_pos++] = input[pos++;]
            continue;
        }

        if (c == '\'' && !double_quoted)
        {
            single_quoted = !single_quoted;
        }

        else if (c == '\"' && !single_quoted)
        {
            double_quoted = !double_quoted;
        }

        else if (c == '}' && !single_quoted && !double_quoted)
        {
            break;
        }

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
        err(2, "Brace not closed");
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
