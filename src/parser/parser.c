#include "parser.h"

static struct ast *parse_list(enum parser_status *status, struct fifo *lexer);
static struct ast *parse_and_or(enum parser_status *status, struct fifo *lexer);
static struct ast *parse_pipeline(enum parser_status *status,
                                  struct fifo *lexer);
static struct ast *parse_command(enum parser_status *status,
                                 struct fifo *lexer);
static struct ast *parse_simple_command(enum parser_status *status,
                                        struct fifo *lexer);
static void parse_element(enum parser_status *status, struct fifo *lexer,
                          struct ast *in);
static struct ast *parse_shell_command(enum parser_status *status,
                                       struct fifo *lexer);
static struct ast *parse_rule_if(enum parser_status *status,
                                 struct fifo *lexer);
static struct ast *parse_else_clause(enum parser_status *status,
                                     struct fifo *lexer);
static struct ast *parse_compound_list(enum parser_status *status,
                                       struct fifo *lexer);
static struct ast *parse_rule_while(enum parser_status *status, struct fifo *lexer);
static struct ast *parse_rule_until(enum parser_status *status, struct fifo *lexer);

// input =
// list '\n'
// | list EOF
// | '\n'
// | EOF
// ;

// La fonction au dessus va recuperer l'ast
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
    if (token == NULL)//this mean that we dont have any tokens left in the lexer
    {
        return out;
    }
    // error
    return out; //to test?
}

// list = and_or { ';' and_or } [ ';' ] ;
// struct ast_list start
static struct ast *parse_list(enum parser_status *status, struct fifo *lexer)
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
    while (1)
    {
        // TODO Fix grammar rule tied to first ;
        struct lextoken *token = fifo_head(lexer);
        if (!token || token->tok != COLUMN)
            break;
        fifo_pop(lexer);
        child = parse_and_or(status, lexer); // return a cmd
        if (*status != PARSER_OK)
        {
            *status = PARSER_OK;
            break; //this mean there is only a ; and not a and_or
        }
        ast_list_add_ast(list, child); // adding the cmd to the list
    }
    return &list->base;
}

// and_or = pipeline ;
static struct ast *parse_and_or(enum parser_status *status, struct fifo *lexer)
{
    struct ast *child = parse_pipeline(status, lexer);
    if (*status != PARSER_OK)
        return NULL; // error
    return child;
}

// pipeline = command ;
static struct ast *parse_pipeline(enum parser_status *status,
                                  struct fifo *lexer)
{
    struct ast *child = parse_command(status, lexer);
    if (*status != PARSER_OK)
        return NULL; // error
    return child;
}

// command =
// simple_command
//| shell_command

static struct ast *parse_command(enum parser_status *status, struct fifo *lexer)
{
    struct lextoken *token = fifo_head(lexer);

    if (!token)
    {
        *status = PARSER_ERROR;
        return NULL;
    }

    if (token->tok == WORDS)
        return parse_simple_command(status, lexer);

    // shell commands
    if (token->tok == IF || token->tok == WHILE || token->tok == UNTIL)
        return parse_shell_command(status, lexer);


    *status = PARSER_ERROR;
    return NULL;
}

// simple_command = WORD { element } ;
static struct ast *parse_simple_command(enum parser_status *status,
                                        struct fifo *lexer)
{
    // WORD
    struct lextoken *token = fifo_head(lexer);
    struct ast_cmd *command = ast_cmd_init(0); // initialize command ast node

    if (!token || token->tok != WORDS)
    {
        *status = PARSER_ERROR;
        struct_ast_destroy(&command->base);
        return NULL;
    }

    ast_cmd_add_word(command, token->value);
    fifo_pop(lexer);

    // { element }
    while (1)
    {
        parse_element(status, lexer, &command->base);
        if (*status != PARSER_OK)
        {
            *status = PARSER_OK; // No more elements to parse
            break;
        }
    }

    return &command->base;
}

// element = WORD ;
static void parse_element(enum parser_status *status, struct fifo *lexer,
                          struct ast *in)
{
    struct ast_cmd *out = (struct ast_cmd *)in; // its always a command
    struct lextoken *token = fifo_head(lexer);
    if (token && token->tok == WORDS)
    {
        ast_cmd_add_word(out, token->value);
        fifo_pop(lexer);
        *status = PARSER_OK;
    }
    else
    {
        *status = PARSER_ERROR;
    }
}

//----------------------------------------------------------------------------------
// shell_command = rule_if ;
//| rule_while ;
//| rule_until ;
static struct ast *parse_shell_command(enum parser_status *status,
                                       struct fifo *lexer)
{
    // rule_if
    struct ast *child = parse_rule_if(status, lexer);
    if (*status != PARSER_OK)
    {
        // rule_while
        child = parse_rule_while(status, lexer);
        if (*status != PARSER_OK)
        {
            // rule_until
            child = parse_rule_until(status, lexer);
            if (*status != PARSER_OK)
            {
                return NULL; // error
            }
            else
            {
                *status = PARSER_OK;
            }
        }
        else
        {
            *status = PARSER_OK;
        }
    }
    return child;
}

// rule_if = 'if' compound_list 'then' compound_list [else_clause] 'fi' ;
static struct ast *parse_rule_if(enum parser_status *status, struct fifo *lexer)
{
    // initialize if ast node
    struct ast_if *out_if = ast_if_init();
    //'if'
    struct lextoken *token = fifo_head(lexer); // check for 'if'
    if (!token || token->tok != IF)
    {
        struct_ast_destroy(&out_if->base);
        *status = PARSER_ERROR;
        return NULL;
    }
    fifo_pop(lexer);
    // compound_list
    struct ast *condition = parse_compound_list(status, lexer);
    if (*status != PARSER_OK)
    {
        struct_ast_destroy(&out_if->base);
        return NULL; // error
    }
    out_if->condition = condition;
    //'then'
    token = fifo_head(lexer); // check for 'then'
    if (!token || token->tok != THEN)
    {
        struct_ast_destroy(&out_if->base);
        *status = PARSER_ERROR;
        return NULL;
    }
    fifo_pop(lexer);
    // compound_list
    struct ast *then_body = parse_compound_list(status, lexer);
    if (*status != PARSER_OK)
    {
        struct_ast_destroy(&out_if->base);
        return NULL; // error
    }
    out_if->then_body = then_body;
    //[else_clause]
    struct ast *else_body = parse_else_clause(status, lexer);
    if (*status != PARSER_OK)
        *status = PARSER_OK; // no else clause is ok
    else
        out_if->else_body = else_body;
    //'fi'
    token = fifo_head(lexer); // check for 'fi'
    if (!token || token->tok != FI)
    {
        struct_ast_destroy(&out_if->base);
        *status = PARSER_ERROR;
        return NULL;
    }
    fifo_pop(lexer);
    return &out_if->base;
}

// else_clause =
//'else' compound_list
//| 'elif' compound_list 'then' compound_list [else_clause]
static struct ast *parse_else_clause(enum parser_status *status,
                                     struct fifo *lexer)
{
    // FIRST PART
    //'else'
    struct lextoken *token = fifo_head(lexer); // check for 'else'
    if (token && token->tok == ELSE)
    {
        fifo_pop(lexer);
        // compound_list
        struct ast *else_body = parse_compound_list(status, lexer);
        if (*status != PARSER_OK)
            return NULL; // error
        return else_body;
    }
    // SECOND PART
    //'elif'
    else if (token && token->tok == ELIF)
    { // similar to rule_if parsing
        fifo_pop(lexer);
        // compound_list
        struct ast_if *out_if = ast_if_init();
        struct ast *condition = parse_compound_list(status, lexer);
        if (*status != PARSER_OK)
        {
            struct_ast_destroy(&out_if->base);
            return NULL; // error
        }
        out_if->condition = condition;
        //'then'
        token = fifo_head(lexer); // check for 'then'
        if (!token || token->tok != THEN)
        {
            struct_ast_destroy(&out_if->base);
            *status = PARSER_ERROR;
            return NULL;
        }
        fifo_pop(lexer);
        // compound_list
        struct ast *then_body = parse_compound_list(status, lexer);
        if (*status != PARSER_OK)
        {
            struct_ast_destroy(&out_if->base);
            return NULL; // error
        }
        out_if->then_body = then_body;
        //[else_clause]
        struct ast *else_body = parse_else_clause(status, lexer);
        if (*status != PARSER_OK)
            *status = PARSER_OK; // no else clause is ok
        else
            out_if->else_body = else_body;
        return &out_if->base;
    }
    else
    {
        *status = PARSER_ERROR;
        return NULL;
    }
}

// compound_list = and_or [';'] {'\n'} ; //easy version

// compound_list =
//{'\n'} and_or { ( ';' | '\n' ) {'\n'} and_or } [';'] {'\n'} ;

static struct ast *parse_compound_list(enum parser_status *status,
                                       struct fifo *lexer)
{
    // {'\n'}
    while (1)
    {
        struct lextoken *token = fifo_head(lexer);
        if (!token || token->tok != NEWLINE)
            break;
        fifo_pop(lexer);
    }
    // and_or
    struct ast *child = parse_and_or(status, lexer); //cmd ou list
    if (*status != PARSER_OK)
        return NULL; // error
    struct lextoken *token;
    // { ( ';' | '\n' ) {'\n'} and_or }
    while (1)
    {
        token = fifo_head(lexer);
        // ( ';' | '\n' )
        if (!token || (token->tok != COLUMN && token->tok != NEWLINE))
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
        // and_or
        struct ast *next_child = parse_and_or(status, lexer);
        if (*status != PARSER_OK)
        {
            *status = PARSER_OK;
            break; // error
        }
        // create a list to hold child and next_child
        struct ast_list *new_list = ast_list_init();
        ast_list_add_ast(new_list, child);
        ast_list_add_ast(new_list, next_child);
        child = &new_list->base;
    }
    // [';']
    token = fifo_head(lexer);
    if (token && token->tok == COLUMN)
        fifo_pop(lexer);
    // {'\n'}
    while (1)
    {
        token = fifo_head(lexer);
        if (!token || token->tok != NEWLINE)
            break;
        fifo_pop(lexer);
    }
    return child;
}


//-----------------------------------Rule while--------------------------------------//
//rule_while = 'while' compound_list 'do' compound_list 'done' ;

static struct ast *parse_rule_while(enum parser_status *status, struct fifo *lexer)
{  
    //'while'
    struct lextoken *token = fifo_head(lexer);
    if (token->tok != WHILE)
    {
        *status = PARSER_ERROR;
        return NULL;
    }
    fifo_pop(lexer);
    //compound_list->condition
    struct ast_while *out_while = ast_while_init();
    struct ast *condition = parse_compound_list(status, lexer);
    if (*status != PARSER_OK)
    {
        return NULL; //Error
    }
    out_while->condition = condition;
    //'do'
    token = fifo_head(lexer);
    if (token->tok != DO)
    {
        *status = PARSER_ERROR;
        struct_ast_destroy(&out_while->base);
        return NULL;
    }
    fifo_pop(lexer);
    //compound_list->body
    struct ast *body = parse_compound_list(status, lexer);
    if (*status != PARSER_OK)
    {
        struct_ast_destroy(&out_while->base);
        return NULL;
    }    
    out_while->body = body;
    //'done'
    token = fifo_head(lexer);
    if (token->tok != DONE)
    {
        *status = PARSER_ERROR;
        struct_ast_destroy(&out_while->base);
        return NULL;
    }
    fifo_pop(lexer);
    return &out_while->base;
}

//rule_until = 'until' compound_list 'do' compound_list 'done' ;

static struct ast *parse_rule_until(enum parser_status *status, struct fifo *lexer)
{  
    //'until'
    struct lextoken *token = fifo_head(lexer);
    if (token->tok != UNTIL)
    {
        *status = PARSER_ERROR;
        return NULL;
    }
    fifo_pop(lexer);
    //compound_list->condition
    struct ast_until *out_until = ast_until_init();
    struct ast *condition = parse_compound_list(status, lexer);
    if (*status != PARSER_OK)
    {
        return NULL; //Error
    }
    out_until->condition = condition;
    //'do'
    token = fifo_head(lexer);
    if (token->tok != DO)
    {
        *status = PARSER_ERROR;
        struct_ast_destroy(&out_until->base);
        return NULL;
    }
    fifo_pop(lexer);
    //compound_list->body
    struct ast *body = parse_compound_list(status, lexer);
    if (*status != PARSER_OK)
    {
        struct_ast_destroy(&out_until->base);
        return NULL;
    }    
    out_until->body = body;
    //'done'
    token = fifo_head(lexer);
    if (token->tok != DONE)
    {
        *status = PARSER_ERROR;
        struct_ast_destroy(&out_until->base);
        return NULL;
    }
    fifo_pop(lexer);
    return &out_until->base;
}