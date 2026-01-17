#include "parser.h"
#include <stdlib.h>
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
static struct redirection *parse_redirections(enum parser_status *status, struct fifo *lexer);
static struct redirection *parse_prefix(enum parser_status *status, struct fifo *lexer);
// input =
// list '\n'
// | list EOF
// | '\n'
// | EOF
// ;
int activated_negation = 0;// will check if negation is activated

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

//the problem here is that this dont read everything,
//it just read one line or one shell command
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

//and_or = pipeline { ( '&&' | '||' ) {'\n'} pipeline } ;
static struct ast *parse_and_or(enum parser_status *status, struct fifo *lexer)
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
            ast_list_add_ast(out->children, child); //we only add the child, not the operator
        }
        ast_and_or_add_ast(out, child2, type);
        counter = 1;
    }
    if (counter == 1)
        return &out->base;
    return child;
}

//pipeline = ['!'] command { '|' {'\n'} command } ;

static struct ast *parse_pipeline(enum parser_status *status,
                                  struct fifo *lexer)
{
    // ['!']
    struct lextoken *token = fifo_head(lexer);
    if (token && token->tok == NEGA)
    {
        activated_negation = 1;
        fifo_pop(lexer);
    }
    struct ast *child = parse_command(status, lexer); // this is either cmd or a if/until/while
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
        if (counter == 0) // first time creating pipeline so we add the first command
        {
            pipeline = ast_pipeline_init(activated_negation);
            ast_list_add_ast(pipeline->pipelines, child);   
        }
        ast_list_add_ast(pipeline->pipelines, child2);
        counter = 1 ;
    }
    if (counter == 1)
        return &pipeline->base;
    return child;
}

// command =
// simple_command
//| shell_command { redirection } ;

static struct ast *parse_command(enum parser_status *status, struct fifo *lexer)
{
    struct lextoken *token = fifo_head(lexer);

    if (!token)
    {
        *status = PARSER_ERROR;
        return NULL;
    }
    struct ast *out;
    if (token->tok == WORDS)
        out = parse_simple_command(status, lexer);

    // shell commands
    else if (token->tok == IF || token->tok == WHILE || token->tok == UNTIL)
        out = parse_shell_command(status, lexer);
    else
    {
        *status = PARSER_ERROR;
        return NULL;
    }
    while (1)
    {
        struct redirection *redir = parse_redirections(status, lexer); //created the struct to add 
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

// simple_command = { prefix } WORD { element } 
// prefix { prefix };
static struct ast *parse_simple_command(enum parser_status *status,
                                        struct fifo *lexer)
{
    // { prefix }
    struct redirection **redir_to_add = NULL; //list of redirections to add to the command
    while (1)
    {

        struct redirection *prefix_redir = parse_prefix(status, lexer);
        if (*status != PARSER_OK)
        {
            *status = PARSER_OK; // No more prefixes to parse
            break;
        }
        //add to list
        add_redirection(&redir_to_add, prefix_redir);
    }
    // WORD
    struct lextoken *token = fifo_head(lexer);
    struct ast_cmd *command = ast_cmd_init(activated_negation); // initialize command ast node

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
    //add redirections to command
    if (redir_to_add)
    {
        command->redirections = redir_to_add;
    }
    return &command->base;
}
//prefix = redirection ;

static struct redirection *parse_prefix(enum parser_status *status,
                              struct fifo *lexer)
{
    struct redirection *out = parse_redirections(status, lexer);
    if (*status != PARSER_OK)
    {
        return NULL; // error
    }
    return out;
}

//redirection = [IONUMBER] ( '>' | '<' | '>>' | '>&' | '<&' | '>|' | '<>' ) WORD ;

static struct redirection *parse_redirections(enum parser_status *status,
                                              struct fifo *lexer)
{
    struct lextoken *token = fifo_head(lexer);
    char *fd = NULL;
    //int flags = 0; //May be used in future steps
    // [IONUMBER]
    
    if (token && token->tok == IONUMBER)
    {
        fd = token->value; //May be destroyed by the pop
        fifo_pop(lexer);
        token = fifo_head(lexer);
    }
    // ( '>' | '<' | '>>' | '>&' | '<&' | '>|' | '<>' )
    char *operator;
    if (!token ||
        (token->tok != RED_OUT &&
         token->tok != RED_IN &&
         token->tok != APPEND_OUT &&
         token->tok != DUP_IN &&
         token->tok != DUP_OUT &&
         token->tok != RED_FORCE &&
         token->tok != FD_RW))
    {
        *status = PARSER_ERROR;
        return NULL;
    }
    else
    {
        operator = token->value;
    }
    // May need to stock the redirection type
    fifo_pop(lexer); //MUST NOT FREE TOKEN VALUE IF REDIRECTION 
    //CHECK THIS PART IF MEMORY ERROR (Use after free)
    // WORD
    token = fifo_head(lexer);
    if (!token || token->tok != WORDS)
    {
        *status = PARSER_ERROR;
        return NULL;
    }
    struct redirection *redir = redirection_init(fd, token->value, operator);
    fifo_pop(lexer);
    *status = PARSER_OK;
    return redir;
}

// element = WORD | redirection;
static void parse_element(enum parser_status *status, struct fifo *lexer,
                          struct ast *in)
{
    struct ast_cmd *out = (struct ast_cmd *)in; // its always a command
    struct lextoken *token = fifo_head(lexer);
    // WORD
    if (token && token->tok == WORDS)
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
    int counter = 0;
    struct ast_list *new_list = ast_list_init();
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
        if (counter == 0)
        {
            ast_list_add_ast(new_list, child);
            counter = 1;
        }
        ast_list_add_ast(new_list, next_child);
        child = &new_list->base;
    }
    if (counter == 0)
    {
        ast_list_destroy(new_list);
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
    if (counter == 1)
        return &new_list->base;
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