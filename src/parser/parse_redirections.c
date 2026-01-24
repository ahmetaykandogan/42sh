#include "parser.h"

#include <stdlib.h>
// prefix = ASSIGNMENT_WORD
//| redirection;

struct redirection *parse_prefix(enum parser_status *status,
                                        struct fifo *lexer)
{
    struct redirection *out = parse_redirections(status, lexer);
    if (*status != PARSER_OK)
    {
        return NULL; // error
    }
    return out;
}
// So basically since prefix return redirection, I cant expand it
// So Ill seperate the two, when we test prefix, we test for vars too
char *parse_prefix_vars(enum parser_status *status, struct fifo *lexer)
{
    struct lextoken *token = fifo_head(lexer);
    if (!token || token->tok != WORDS)
    {
        *status = PARSER_ERROR;
        return NULL; // error
    }
    int check = 0;
    for (int i = 0; token->value[i]; i++)
    {
        if (token->value[i] == '=')
        {
            check = 1;
            break;
        }
    }
    if (check == 0)
    {
        *status = PARSER_ERROR;
        return NULL; // error
    }
    // if we reach here, its a var
    char *var = token->value;
    fifo_pop(lexer);
    return var;
}

// redirection = [IONUMBER] ( '>' | '<' | '>>' | '>&' | '<&' | '>|' | '<>' )
// WORD ;

struct redirection *parse_redirections(enum parser_status *status,
                                              struct fifo *lexer)
{
    struct lextoken *token = fifo_head(lexer);
    char *fd = NULL;
    // int flags = 0; //May be used in future steps
    //  [IONUMBER]

    if (token && token->tok == IONUMBER)
    {
        fd = token->value; // May be destroyed by the pop
        fifo_pop(lexer);
        token = fifo_head(lexer);
    }
    // ( '>' | '<' | '>>' | '>&' | '<&' | '>|' | '<>' )
    char *operator;
    if (!token
        || (token->tok != RED_OUT && token->tok != RED_IN
            && token->tok != APPEND_OUT && token->tok != DUP_IN
            && token->tok != DUP_OUT && token->tok != RED_FORCE
            && token->tok != FD_RW))
    {
        *status = PARSER_ERROR;
        return NULL;
    }
    else
    {
        operator= token->value;
    }
    // May need to stock the redirection type
    fifo_pop(lexer); // MUST NOT FREE TOKEN VALUE IF REDIRECTION
    // CHECK THIS PART IF MEMORY ERROR (Use after free)
    //  WORD
    token = fifo_head(lexer);
    if (!token || token->tok != WORDS)
    {
        free(operator);
        *status = PARSER_CRITICAL_ERROR;
        return NULL;
    }
    struct redirection *redir = redirection_init(fd, token->value, operator);
    fifo_pop(lexer);
    *status = PARSER_OK;
    return redir;
}