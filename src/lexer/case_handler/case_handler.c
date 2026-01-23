#include <stdbool.h>
#include <ctype.h>
#include <stddef.h>
#include <string.h>

#include "utils/malloc_err/malloc_err.h"
#include "lexer/lexing/lexing.h"
#include "lexer/create_lextoken/create_lextoken.h"
#include "utils/fifo/fifo.h"

size_t handle_whitespace(char *input, size_t i)
{
    while (input[i] == ' ' || input[i] == '\t')
    {
        i++;
    }
    return i;
}

size_t handle_comment(char *input, size_t i, struct fifo *queue)
{
    if (i == 0 || isspace(input[i - 1]) || input[i - 1] == ';')
    {
        while (input[i] && input[i] != '\n')
        {
            i++;
        }
    }
    else
    {
        char *tok = lexing_words(input, i);
        i += strlen(tok);

        fifo_push(queue, create_lextoken_commands(tok));
    }
    return i;
}

size_t handle_number(char *input, size_t i, size_t input_len, struct fifo *queue)
{
    char *tok = lexing_numbers(input, i);
    i += strlen(tok);

    if (i < input_len && (input[i] == '<' || input[i] == '>'))
    {
        fifo_push(queue, create_lextoken_words(IONUMBER, tok));
    }
    else
    {
        fifo_push(queue, create_lextoken_words(WORDS, tok));
    }

    return i;
}

size_t handle_word(char *input, size_t i, struct fifo *queue)
{
    char *tok = lexing_words(input, i);
    i += strlen(tok);

    fifo_push(queue, create_lextoken_commands(tok));

    return i;
}

size_t handle_single_quote(char *input, size_t i, struct fifo *queue)
{
    i++; // on skip la premiere quote

    char *tok = lexing_quotes(input, i);
    i += strlen(tok) + 1; // on skip la seconde quote

    fifo_push(queue, create_lextoken_words(APSTRPH, tok));

    return i;
}

size_t handle_double_quote(char *input, size_t i, struct fifo *queue)
{
    i++; // on skip la premiere quote

    char *tok = lexing_double_quotes(input, i);
    i += strlen(tok) + 1; // on skip la seconde quote

    fifo_push(queue, create_lextoken_words(GUILLMT, tok));

    return i;
}


size_t handle_redirection(char *input, size_t i, struct fifo *queue)
{
    char *tok = lexing_redirection(input, i);
    i += strlen(tok);

    fifo_push(queue, create_lextoken_redirection(tok));

    return i;
}

size_t handle_operator(char *input, size_t i, struct fifo *queue)
{
    char *tok = lexing_operators(input, i);
    i += strlen(tok);

    fifo_push(queue, create_lextoken_operators(tok));

    return i;
}

size_t handle_variable(char *input, size_t i, size_t input_len, struct fifo *queue)
{
    i++; // skip le $

    bool braced = (i < input_len && input[i] == '{');
    if (braced)
    {
        i++;
    }

    char *tok = lexing_variables(input, i);
    i += strlen(tok);

    braced = (i < input_len && input[i] == '}');
    if (braced)
    {
        i++;
    }

    if (strcmp(tok, "$") == 0)
    {
        fifo_push(queue, create_lextoken_words(WORDS, tok));
    }
    else
    {
        fifo_push(queue, create_lextoken_words(VAR, tok));
    }

    return i;
}

size_t handle_single_char(char *input, size_t i, struct fifo *queue)
{
    char *tok = malloc_err(2);
    tok[0] = input[i];
    tok[1] = '\0';

    fifo_push(queue, create_lextoken_single_char(tok));
    return i + 1; 
}
