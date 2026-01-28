#include "parser.h"

// simple_command = { prefix } WORD { element }
// prefix { prefix }; //TODO
struct ast *parse_simple_command(enum parser_status *status, struct fifo *lexer)
{
    // { prefix }
    struct ast_cmd *command = ast_cmd_init(0); // initialize command ast node
    int redir_counter = 0;
    struct redirection **redir_to_add =
        NULL; // list of redirections to add to the command
    while (1)
    {
        struct redirection *prefix_redir = parse_prefix(status, lexer);
        if (*status == PARSER_CRITICAL_ERROR)
        {
            return NULL; // error
        }
        if (*status != PARSER_OK)
        {
            *status = PARSER_OK; // No more prefixes to parse
            char *var = parse_prefix_vars(status, lexer);
            if (*status != PARSER_OK)
            {
                *status = PARSER_OK; // No more prefixes to parse
                break;
            }
            else
            {
                add_vars(command, var);
                redir_counter++;
                break;
            }
        }
        // add to list
        add_redirection(&redir_to_add, prefix_redir);
        redir_counter++;
    }
    // WORD
    command->redirections = redir_to_add;
    struct lextoken *token = fifo_head(lexer);
    if (!token || token->tok != WORDS)
    {
        if (redir_counter == 0) // no prefix total error
        {
            *status = PARSER_ERROR;
            struct_ast_destroy(&command->base);
            return NULL;
        }
        // this is where prefix { prefix } happens
        return &command->base; // we only have redirections
    }
    ast_cmd_add_word(command, token->value);
    fifo_pop(lexer);
    // { element }
    while (1)
    {
        parse_element(status, lexer, &command->base);
        if (*status == PARSER_CRITICAL_ERROR)
        {
            struct_ast_destroy(&command->base);
            return NULL; // error
        }
        if (*status != PARSER_OK)
        {
            *status = PARSER_OK; // No more elements to parse
            break;
        }
    }
    return &command->base;
}

// element = WORD | redirection;
void parse_element(enum parser_status *status, struct fifo *lexer,
                   struct ast *in)
{
    struct ast_cmd *out = (struct ast_cmd *)in; // its always a command
    struct lextoken *token = fifo_head(lexer);
    // WORD
    if (token && (token->tok == WORDS || token->tok == WORDS))
    {
        ast_cmd_add_word(out, token->value);
        fifo_pop(lexer);
        *status = PARSER_OK;
    }
    else if (token)
    {
        // redirection
        struct redirection *redir = parse_redirections(status, lexer);
        if (*status != PARSER_OK)
        {
            return; // error
        }
        ast_add_redirection(&out->base, redir);
        *status = PARSER_OK;
    }
    else
    {
        *status = PARSER_ERROR;
    }
}
