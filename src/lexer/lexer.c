#include <err.h>
#include <string.h>
#include <stdlib.h>

#include "lexer/create_lextoken/create_lextoken.h"
#include "lexer/lexing/lexing.h"
#include "lexer/read_file/read_file.h"
#include "lexer/struct_lexer.h"
#include "utils/check_char/check_char.h"
#include "utils/fifo/fifo.h"
#include "utils/malloc_err/malloc_err.h"

/* --------------------------- */
/* -- MAIN LEXING FUNCTIONS -- */
/* --------------------------- */

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
        if (input[i] == ' ')
        {
            i++;
            continue;
        }

        /* CASE WORDS */
        if (check_char(input[i]) == 42)
        {
            char *tok = lexing_words(input, i);
            i += strlen(tok);

            fifo_push(queue, create_lextoken_commands(tok));
        }

        /* CASE APOSTROPHE + QUOTE */
        else if (check_char(input[i]) == 1)
        {
            i++; // on skip la premiere quote

            char *tok = lexing_quotes(input, i);
            i += strlen(tok) + 1; // on skip la seconde quote

            fifo_push(queue, create_lextoken_words(APSTRPH, tok));
        }

        /* CASE GUILLMT + DOUBLE QUOTE */
        else if (check_char(input[i]) == 2)
        {
            i++; // on skip la premiere quote

            char *tok = lexing_double_quotes(input, i);
            i += strlen(tok) + 1; // on skip la seconde quote

            fifo_push(queue, create_lextoken_words(APSTRPH, tok));
        }

        /* CASE REDIRECTION */
        else if (input[i] == '<' || input[i] == '>')
        {
            char *tok = lexing_redirection(input, i);
            i += strlen(tok);

            fifo_push(queue, create_lextoken_redirection(tok));
        }

        /* CASE OPERATORS + PIPE */
        else if (input[i] == '|' || input[i] == '&')
        {
            char *tok = lexing_operators(input, i);
            i += strlen(tok);

            fifo_push(queue, create_lextoken_operators(tok));
        }

        /* CASE VARIABLE */
        else if (input[i] == '$')
        {
            i++;

            char *tok = lexing_variables(input, i);
            i += strlen(tok);

            fifo_push(queue, create_lextoken_words(VAR, tok));
        }   

        /* CASE A 1 CARACTERE (Exemple : \n) */
        else
        {
            char *tok = malloc_err(2);
            tok[0] = input[i++];
            tok[1] = '\0';
            fifo_push(queue, create_lextoken_single_char(tok));
        }
    }
    return queue;
}

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
