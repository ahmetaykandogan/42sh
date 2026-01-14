#include <ctype.h>
#include <err.h>
#include <stdlib.h>
#include <string.h>

#include "../utils/fifo.h"
#include "../utils/malloc_err.h"
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

// this shit is so long it cause my cancer to have more phase than Dreglord
// TODO, function auxiliaire pour separer les groupes de cas
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

    /* NEGA */
    else if (strcmp(token, "!") == 0) // NEGA
    {
        return create_lextoken(NEGA, token);
    }

    /* ESCAPE */
    else if (strcmp(token, "\\") == 0) // ESCAPE
    {
        return create_lextoken(ESCAPE, token);
    }

    /* DEFAULT CASE : WORDS */
    return create_lextoken_words(WORDS, token);
}

static struct lextoken *create_lextoken_redirection(char *token)
{
    /* REDIRECTION */
    if (strcmp(token, "<") == 0) // RED_IN
    {
        return create_lextoken(RED_IN, token);
    }
    else if (strcmp(token, ">") == 0) // RED_OUT
    {
        return create_lextoken(RED_OUT, token);
    }
    else if (strcmp(token, ">>") == 0) // APPEND_OUT
    {
        return create_lextoken(APPEND_OUT, token);
    }
    else if (strcmp(token, "<&") == 0) // DUP_IN
    {
        return create_lextoken(DUP_IN, token);
    }
    else if (strcmp(token, ">&") == 0) // DUP_OUT
    {
        return create_lextoken(DUP_OUT, token);
    }
    else if (strcmp(token, ">|") == 0) // RED_FORCE
    {
        return create_lextoken(RED_FORCE, token);
    }
    else if (strcmp(token, "<>") == 0) // FD_RW
    {
        return create_lextoken(FD_RW, token);
    }

    free(token);
    err(2, "invalid redirection token");
}

static struct lextoken *create_lextoken_operators(char *token)
{
    /* OPERATORS */
    if (strcmp(token, "&&") == 0) // AND
    {
        return create_lextoken(AND, token);
    }
    else if (strcmp(token, "||") == 0) // OR
    {
        return create_lextoken(OR, token);
    }
    /* PIPE */
    else if (strcmp(token, "|") == 0) // PIPE
    {
        return create_lextoken(PIPE, token);
    }

    free(token);
    err(2, "invalid operators token");
}

// Should add the following characters for the next steps: | & < >
static int check_char(char c)
{
    if (c == ';' || c == '\n' || c == '\\' || c == '\0' || c == '!' || c == '&'
        || c == '|' || c == '<' || c == '>' || isspace(c))
        return 0;
    if (c == '\'')
        return 1;
    if (c == '\"')
        return 3;
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

static char *lexing_double_quotes(char *input, size_t pos)
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

static char *lexing_redirection(char *input, size_t pos)
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

static char *lexing_operators(char *input, size_t pos)
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
        if (check_char(input[i]) == 2)
        {
            char *tok = lexing_words(input, i);
            i += strlen(tok);

            fifo_push(queue, lexing_case(tok));
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
        else if (check_char(input[i]) == 1)
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

        /* CASE A 1 CARACTERE (Exemple : \n) */
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
