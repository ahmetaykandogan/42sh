#include <err.h>
#include <stdlib.h>
#include <string.h>

#include "lexer/struct_lexer.h"
#include "utils/check_char/check_char.h"
#include "utils/malloc_err/malloc_err.h"

/* --------------------------- */
/* -- CREATING THE LEXTOKEN -- */
/* --------------------------- */
static struct lextoken *create_lextoken(enum token tok, char *token)
{
    free(token);

    struct lextoken *l_token = malloc_err(sizeof(struct lextoken));
    l_token->tok = tok;
    l_token->value = NULL;

    return l_token;
}

static struct lextoken *create_lextoken_char(enum token tok)
{
    struct lextoken *l_token = malloc_err(sizeof(struct lextoken));
    l_token->tok = tok;
    l_token->value = NULL;

    return l_token;
}

struct lextoken *create_lextoken_words(enum token tok, char *value)
{
    struct lextoken *l_token = malloc_err(sizeof(struct lextoken));
    l_token->tok = tok;
    if (!value)
        l_token->value = NULL;

    else
        l_token->value = value;

    return l_token;
}

struct lextoken *create_lextoken_commands(char *token)
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

    /* DEFAULT CASE : WORDS */
    return create_lextoken_words(WORDS, token);
}

struct lextoken *create_lextoken_single_char(char token)
{
    /* COLUMN */
    if (token == ';') // COLUMN
    {
        return create_lextoken_char(COLUMN);
    }

    /* NEWLINE */
    else if (token == '\n') // NEWLINE
    {
        return create_lextoken_char(NEWLINE);
    }

    /* NEGA */
    else if (token == '!') // NEGA
    {
        return create_lextoken_char(NEGA);
    }

    /* ESCAPE */
    else if (token == '\\') // ESCAPE
    {
        return create_lextoken_char(ESCAPE);
    }

    /* BRACE */
    else if (token == '{') // BRACE_OPEN
    {
        return create_lextoken_char(BRACE_OPEN);
    }
    else if (token == '}') // BRACE_CLOSE
    {
        return create_lextoken_char(BRACE_CLOSE);
    }

    /* BRACKET */
    else if (token == '(') // BRACKET_OPEN
    {
        return create_lextoken_char(BRACKET_OPEN);
    }
    else if (token == ')') // BRACKET_CLOSE
    {
        return create_lextoken_char(BRACKET_CLOSE);
    }

    err(2, "invalid token");
}

struct lextoken *create_lextoken_redirection(char *token)
{
    /* REDIRECTION */
    if (strcmp(token, "<") == 0) // RED_IN
    {
        return create_lextoken_words(RED_IN, token);
    }
    else if (strcmp(token, ">") == 0) // RED_OUT
    {
        return create_lextoken_words(RED_OUT, token);
    }
    else if (strcmp(token, ">>") == 0) // APPEND_OUT
    {
        return create_lextoken_words(APPEND_OUT, token);
    }
    else if (strcmp(token, "<&") == 0) // DUP_IN
    {
        return create_lextoken_words(DUP_IN, token);
    }
    else if (strcmp(token, ">&") == 0) // DUP_OUT
    {
        return create_lextoken_words(DUP_OUT, token);
    }
    else if (strcmp(token, ">|") == 0) // RED_FORCE
    {
        return create_lextoken_words(RED_FORCE, token);
    }
    else if (strcmp(token, "<>") == 0) // FD_RW
    {
        return create_lextoken_words(FD_RW, token);
    }

    free(token);
    err(2, "invalid redirection token");
}

struct lextoken *create_lextoken_operators(char *token)
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
