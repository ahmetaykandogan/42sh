#include "parser.h"

#include <stdlib.h>

// list '\n'
// | list EOF
// | '\n'
// | EOF
// ;
int activated_negation = 0; // will check if negation is activated

// La fonction au dessus va recuperer l'ast
struct ast *main_parse(enum parser_status *status, struct fifo *lexer)
{
    struct ast_list *out = ast_list_init();
    struct lextoken *token = fifo_head(lexer);
    if (!token)
    {
        *status = PARSER_OK;
        return &out->base; // EOF
    }
    while (1)
    {
        token = fifo_head(lexer);
        if (!token)
        {
            *status = PARSER_OK;
            return &out->base; // EOF
        }
        struct ast *child = parse_input(status, lexer);
        if (*status != PARSER_OK)
        {
            struct_ast_destroy(&out->base);
            return NULL; // error
        }
        ast_list_add_ast((struct ast_list *)out, child);
    }
    return &out->base;
}

// the problem here is that this dont read everything,
// it just read one line or one shell command
struct ast *parse_input(enum parser_status *status, struct fifo *lexer)
{
    struct lextoken *token = fifo_head(lexer);

    // EOF
    if (!token)
    {
        *status = PARSER_OK;
        return NULL;
    }

    // '\n'
    if (token->tok == NEWLINE)
    {
        fifo_pop(lexer);
        *status = PARSER_OK;
        return NULL;
    }

    // list '\n' | list EOF
    struct ast *out = parse_list(status, lexer);
    token = fifo_head(lexer);
    if (*status != PARSER_OK)
        return NULL;

    if (token && (token->tok == NEWLINE))
    {
        fifo_pop(lexer);
        *status = PARSER_OK;
        return out;
    }
    if (token
        == NULL) // this mean that we dont have any tokens left in the lexer
    {
        return out;
    }
    // error
    struct_ast_destroy(out);
    *status = PARSER_ERROR;
    return NULL; // PARSING WAS INCOMPLETE
}

// list = and_or { ';' and_or } [ ';' ] ;
// struct ast_list start
struct ast *parse_list(enum parser_status *status, struct fifo *lexer)
{
    struct ast_list *list = ast_list_init();
    struct ast *child = parse_and_or(status, lexer); // return a cmd
    if (*status != PARSER_OK)
    {
        ast_list_destroy(list);
        return NULL; // error
    }
    ast_list_add_ast(list, child); // adding the cmd to the list

    // { ';' and_or } [ ; ]
    int column_counter = 0;
    while (1)
    {
        // TODO Fix grammar rule tied to first ;
        struct lextoken *token = fifo_head(lexer);
        if (!token || token->tok != COLUMN)
            break;
        column_counter++;
        fifo_pop(lexer);
        child = parse_and_or(status, lexer); // return a cmd
        if (*status != PARSER_OK)
        {
            *status = PARSER_OK;
            break; // this mean there is only a ; and not a and_or
        }
        column_counter = 0;
        ast_list_add_ast(list, child); // adding the cmd to the list
    }
    // [ ; ]
    struct lextoken *token = fifo_head(lexer);
    if (token && token->tok == COLUMN)
    {
        if (column_counter == 1)
        {
            *status = PARSER_ERROR;
            ast_list_destroy(list);
            return NULL; // error: two ; in a row
        }
        fifo_pop(lexer);
    }
    return &list->base;
}

// and_or = pipeline { ( '&&' | '||' ) {'\n'} pipeline } ;
struct ast *parse_and_or(enum parser_status *status, struct fifo *lexer)
{
    // pipeline
    struct ast *child = parse_pipeline(status, lexer);
    if (*status != PARSER_OK)
        return NULL; // error
    // { ( '&&' | '||' ) {'\n'} pipeline }
    struct lextoken *token;
    struct ast_and_or *out = NULL;
    int counter = 0;
    while (1)
    {
        // ( '&&' | '||' )
        token = fifo_head(lexer);
        int type;
        if (token && token->tok == AND)
            type = 0;
        else if (token && token->tok == OR)
            type = 1;
        else
            break;
        fifo_pop(lexer);
        // {'\n'}
        while (1)
        {
            token = fifo_head(lexer);
            if (!token || token->tok != NEWLINE)
                break;
            fifo_pop(lexer);
        }
        // pipeline
        struct ast *child2 = parse_pipeline(status, lexer);
        if (*status != PARSER_OK)
            return NULL; // error
        // if we reach here, it means we at least have one and_or
        if (counter == 0)
        {
            out = ast_and_or_init();
            ast_list_add_ast(out->children,
                             child); // we only add the child, not the operator
        }
        ast_and_or_add_ast(out, child2, type);
        counter = 1;
    }
    if (counter == 1)
        return &out->base;
    return child;
}

// pipeline = ['!'] command { '|' {'\n'} command } ;

struct ast *parse_pipeline(enum parser_status *status, struct fifo *lexer)
{
    // ['!']
    struct lextoken *token = fifo_head(lexer);
    if (token && token->tok == NEGA)
    {
        activated_negation = 1;
        fifo_pop(lexer);
    }
    struct ast *child =
        parse_command(status, lexer); // this is either cmd or a if/until/while
    if (*status != PARSER_OK)
        return NULL; // error
    // { '|' {'\n'} command }
    int counter = 0;
    struct ast_pipe *pipeline = NULL;
    while (1)
    {
        // '|'
        token = fifo_head(lexer);
        if (!token || token->tok != PIPE)
            break;
        fifo_pop(lexer);
        // {'\n'}
        while (1)
        {
            token = fifo_head(lexer);
            if (!token || token->tok != NEWLINE)
                break;
            fifo_pop(lexer);
        }
        // command
        struct ast *child2 = parse_command(status, lexer);
        if (*status != PARSER_OK)
            return NULL;
        // if we reach here, it means we at least have one pipeline
        if (counter
            == 0) // first time creating pipeline so we add the first command
        {
            pipeline = ast_pipeline_init(activated_negation);
            ast_list_add_ast(pipeline->pipelines, child);
        }
        ast_list_add_ast(pipeline->pipelines, child2);
        counter = 1;
    }
    if (counter == 1)
        return &pipeline->base;
    return child;
}

// command =
// simple_command
//| shell_command { redirection } ;
// funcdec { redirection }

struct ast *parse_command(enum parser_status *status, struct fifo *lexer)
{
    struct lextoken *token = fifo_head(lexer);

    if (!token)
    {
        *status = PARSER_ERROR;
        return NULL;
    }
    struct ast *out;

    // shell commands
    if (token->tok == IF || token->tok == WHILE || token->tok == UNTIL
        || token->tok == FOR || token->tok == BRACE_OPEN)
        out = parse_shell_command(status, lexer);
    else if (token->tok == WORDS || token->tok == RED_IN
             || token->tok == RED_OUT)
        out = parse_words(
            status, lexer); // will check if its a simple command, or a funcdec
    else
    {
        *status = PARSER_ERROR;
        return NULL;
    }
    if (*status == PARSER_CRITICAL_ERROR)
    {
        return NULL;
    }
    while (1)
    {
        struct redirection *redir =
            parse_redirections(status, lexer); // created the struct to add
        if (*status == PARSER_CRITICAL_ERROR)
        {
            struct_ast_destroy(out);
            return NULL; // error
        }
        if (*status != PARSER_OK)
        {
            *status = PARSER_OK; // no redirections is ok
            break;
        }
        else
            ast_add_redirection(out, redir);
    }
    return out;
}

struct ast *parse_words(enum parser_status *status, struct fifo *lexer)
{
    struct lextoken *token = fifo_head(lexer);
    struct lextoken *next_token = fifo_access_i(lexer, 1);
    // check if it's a function declaration
    if (token->tok == RED_IN || token->tok == RED_OUT)
    {
        return parse_simple_command(status, lexer);
    }
    if (next_token && next_token->tok == BRACKET_OPEN)
    {
        // this is a funcdec
        char *first_word = token->value;
        fifo_pop(lexer); // pop function name
        fifo_pop(lexer); // pop '('
        token = fifo_head(lexer);
        if (token && token->tok == BRACKET_CLOSE)
        {
            fifo_pop(lexer); // pop ')'
            // { \n }
            while (1)
            {
                token = fifo_head(lexer);
                if (!token || token->tok != NEWLINE)
                    break;
                fifo_pop(lexer);
            }
            struct ast *body = parse_shell_command(status, lexer);
            if (*status != PARSER_OK)
                return NULL; // error
            struct ast_function *func = ast_function_init();
            func->name = first_word;
            func->body = body;
            return &func->base;
        }
        else
        {
            *status = PARSER_ERROR;
            return NULL; // error
        }
    }
    else
    {
        // simple command, since the token was never popped, no change
        return parse_simple_command(status, lexer);
    }
}
