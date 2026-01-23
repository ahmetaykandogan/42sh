#include <ctype.h>
#include <err.h>
#include <stdlib.h>
#include <string.h>

#include "lexer/case_handler/case_handler.h"
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
        if (input[i] == ' ' || input[i] == '\t')
        {
            i = handle_whitespace(input, i);
        }

        /* CASE COMMENTS */
        else if (input[i] == '#')
        {
            i = handle_comment(input, i, queue);
        }

        /* CASE NUMBER + IONUMBER */
        else if (isdigit(input[i]))
        {
            i = handle_number(input, i, input_len, queue);
        }

        /* CASE WORDS */
        else if (check_char(input[i]) == 42)
        {
            i = handle_word(input, i, queue);
        }

        /* CASE SINGLE QUOTE */
        else if (check_char(input[i]) == 1)
        {
            i = handle_single_quote(input, i, queue);
        }

        /* CASE DOUBLE QUOTE */
        else if (check_char(input[i]) == 2)
        {
            i = handle_double_quote(input, i, queue);
        }

        /* CASE REDIRECTION */
        else if (input[i] == '<' || input[i] == '>')
        {
            i = handle_redirection(input, i, queue);
        }

        /* CASE OPERATORS + PIPE */
        else if (input[i] == '|' || input[i] == '&')
        {
            i = handle_operator(input, i, queue);
        }

        /* CASE VARIABLE */
        else if (input[i] == '$')
        {
            i = handle_variable(input, i, input_len, queue);
        }

        /* CASE A 1 CARACTERE (Exemple : \n) */
        else
        {
            i = handle_single_char(input, i, queue);
        }
    }
    return queue;
}

/* CASE FROM FILE */
/* Ex : ./src/42sh script.sh */
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

/* CASE FROM STDIN */
/* Ex : cat script.sh | ./src/42sh */
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
