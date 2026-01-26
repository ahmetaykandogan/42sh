#include <stdlib.h>

#include "parser.h"
//----------------------------------------------------------------------------------
// shell_command = rule_if ;
//| rule_while ;
//| rule_until ;
//| rule_for ;
//| { compound_list } ;
struct ast *parse_shell_command(enum parser_status *status,
                                       struct fifo *lexer)
{
    // rule_if
    struct ast *child = parse_rule_if(status, lexer);
    if (*status == PARSER_ERROR)
    {
        // rule_while
        *status = PARSER_OK; // reset status
        child = parse_rule_while(status, lexer);
        if (*status == PARSER_ERROR)
        {
            // rule_until
            *status = PARSER_OK; // reset status
            child = parse_rule_until(status, lexer);
            if (*status == PARSER_ERROR)
            {
                *status = PARSER_OK; // reset status
                child = parse_rule_for(status, lexer);
                if (*status == PARSER_ERROR)
                {
                    child = parse_command_block(status, lexer); // compound_list but with {}
                }
            }
        }
    }
    if (*status == PARSER_CRITICAL_ERROR)
        return NULL;
    return child;
}
struct ast *parse_command_block(enum parser_status *status,
                                     struct fifo *lexer)
{
    //'{'
    struct lextoken *token = fifo_head(lexer);
    if (!token || token->tok != BRACE_OPEN)
    {
        *status = PARSER_ERROR;
        return NULL;
    }
    fifo_pop(lexer);
    // compound_list
    struct ast *child = parse_compound_list(status, lexer);
    if (*status != PARSER_OK)
    {
        *status = PARSER_CRITICAL_ERROR;
        return NULL; // error
    }
    //' } '
    token = fifo_head(lexer);
    if (!token || token->tok != BRACE_CLOSE)
    {
        struct_ast_destroy(child);
        *status = PARSER_CRITICAL_ERROR;
        return NULL;
    }
    fifo_pop(lexer);
    return child;
}

// rule_if = 'if' compound_list 'then' compound_list [else_clause] 'fi' ;
struct ast *parse_rule_if(enum parser_status *status, struct fifo *lexer)
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
        *status = PARSER_CRITICAL_ERROR;
        struct_ast_destroy(&out_if->base);
        return NULL; // error
    }
    out_if->condition = condition;
    //'then'
    token = fifo_head(lexer); // check for 'then'
    if (!token || token->tok != THEN)
    {
        struct_ast_destroy(&out_if->base);
        *status = PARSER_CRITICAL_ERROR;
        return NULL;
    }
    fifo_pop(lexer);
    // compound_list
    struct ast *then_body = parse_compound_list(status, lexer);
    if (*status != PARSER_OK || !then_body)
    {
        *status = PARSER_CRITICAL_ERROR;
        struct_ast_destroy(&out_if->base);
        return NULL; // error
    }
    out_if->then_body = then_body;
    //[else_clause]

    token = fifo_head(lexer); // check for 'else of elif'
    if (token && (token->tok == ELSE || token->tok == ELIF))
    {
        struct ast *else_body = parse_else_clause(status, lexer);
        if (*status != PARSER_OK)
        {
            struct_ast_destroy(&out_if->base);
            return NULL; // no else clause is ok
        }
        else
            out_if->else_body = else_body;
    }
    //'fi'
    token = fifo_head(lexer); // check for 'fi'
    if (!token || token->tok != FI)
    {
        struct_ast_destroy(&out_if->base);
        *status = PARSER_CRITICAL_ERROR;
        return NULL;
    }
    fifo_pop(lexer);
    return &out_if->base;
}

// else_clause =
//'else' compound_list
//| 'elif' compound_list 'then' compound_list [else_clause]
struct ast *parse_else_clause(enum parser_status *status, struct fifo *lexer)
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

// compound_list =
//{'\n'} and_or { ( ';' | '\n' ) {'\n'} and_or } [';'] {'\n'} ;

struct ast *parse_compound_list(enum parser_status *status, struct fifo *lexer)
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
    struct ast *child = parse_and_or(status, lexer); // cmd ou list
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

//-----------------------------------Rule
// while--------------------------------------// rule_while = 'while'
// compound_list 'do' compound_list 'done' ;

struct ast *parse_rule_while(enum parser_status *status, struct fifo *lexer)
{
    //'while'
    struct lextoken *token = fifo_head(lexer);
    if (token->tok != WHILE)
    {
        *status = PARSER_ERROR;
        return NULL;
    }
    fifo_pop(lexer);
    // compound_list->condition
    struct ast_while *out_while = ast_while_init();
    struct ast *condition = parse_compound_list(status, lexer);
    if (*status != PARSER_OK)
    {
        *status = PARSER_CRITICAL_ERROR;
        return NULL; // Error
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
    // compound_list->body
    struct ast *body = parse_compound_list(status, lexer);
    if (*status != PARSER_OK)
    {
        struct_ast_destroy(&out_while->base);
        *status = PARSER_CRITICAL_ERROR;
        return NULL;
    }
    out_while->body = body;
    //'done'
    token = fifo_head(lexer);
    if (token->tok != DONE)
    {
        *status = PARSER_CRITICAL_ERROR;
        struct_ast_destroy(&out_while->base);
        return NULL;
    }
    fifo_pop(lexer);
    return &out_while->base;
}

// rule_until = 'until' compound_list 'do' compound_list 'done' ;

struct ast *parse_rule_until(enum parser_status *status, struct fifo *lexer)
{
    //'until'
    struct lextoken *token = fifo_head(lexer);
    if (token->tok != UNTIL)
    {
        *status = PARSER_ERROR;
        return NULL;
    }
    fifo_pop(lexer);
    // compound_list->condition
    struct ast_until *out_until = ast_until_init();
    struct ast *condition = parse_compound_list(status, lexer);
    if (*status != PARSER_OK)
    {
        *status = PARSER_CRITICAL_ERROR;
        return NULL; // Error
    }
    out_until->condition = condition;
    //'do'
    token = fifo_head(lexer);
    if (token->tok != DO)
    {
        *status = PARSER_CRITICAL_ERROR;
        struct_ast_destroy(&out_until->base);
        return NULL;
    }
    fifo_pop(lexer);
    // compound_list->body
    struct ast *body = parse_compound_list(status, lexer);
    if (*status != PARSER_OK)
    {
        *status = PARSER_CRITICAL_ERROR;
        struct_ast_destroy(&out_until->base);
        return NULL;
    }
    out_until->body = body;
    //'done'
    token = fifo_head(lexer);
    if (token->tok != DONE)
    {
        *status = PARSER_CRITICAL_ERROR;
        struct_ast_destroy(&out_until->base);
        return NULL;
    }
    fifo_pop(lexer);
    return &out_until->base;
}

// rule_for =
//'for' WORD ( [';'] | [ {'\n'} 'in' { WORD } ( ';' | '\n' ) ] ) {'\n'} 'do'
// compound_list 'done' ;

struct ast *parse_rule_for(enum parser_status *status, struct fifo *lexer)
{
    //'for'
    struct lextoken *token = fifo_head(lexer);
    if (!token || token->tok != FOR)
    {
        *status = PARSER_ERROR;
        return NULL;
    }
    fifo_pop(lexer);
    // WORD
    token = fifo_head(lexer);
    if (!token || token->tok != WORDS)
    {
        *status = PARSER_CRITICAL_ERROR;
        return NULL;
    }
    char *var_name = token->value;
    fifo_pop(lexer);
    char **inside = create_parse_for_body(status, lexer);
    if (*status != PARSER_OK)
    {
        *status = PARSER_CRITICAL_ERROR;
        return NULL; // error
    }
    //'do'
    token = fifo_head(lexer);
    if (!token || token->tok != DO)
    {
        *status = PARSER_CRITICAL_ERROR;
        return NULL;
    }
    fifo_pop(lexer);
    // compound_list
    struct ast *body = parse_compound_list(status, lexer);
    if (*status != PARSER_OK)
    {
        *status = PARSER_CRITICAL_ERROR;
        return NULL; // error
    }
    //'done'
    token = fifo_head(lexer);
    if (!token || token->tok != DONE)
    {
        *status = PARSER_CRITICAL_ERROR;
        return NULL;
    }
    fifo_pop(lexer);
    struct ast_for *out = ast_for_init();
    out->var_name = var_name;
    out->list = inside;
    out->body = body;
    return &out->base;
}
static char **add_word_to_words(char **words, char *word)
{
    size_t count = 0;
    if (words)
    {
        while (words[count] != NULL)
        {
            count++;
        }
    }
    char **new_inside = realloc(words, sizeof(char *) * (count + 2));
    if (!new_inside)
    {
        return NULL;
    }
    new_inside[count] = word;
    new_inside[count + 1] = NULL;
    words = new_inside;
    return words;
}
//[ [';'] | [ {'\n'} 'in' { WORD } ( ';' | '\n' ) ]
char **create_parse_for_body(enum parser_status *status, struct fifo *lexer)
{
    char **inside = NULL;
    struct lextoken *token = fifo_head(lexer);
    // [';']
    if (!token)
    {
        *status = PARSER_ERROR;
        return NULL;
    }
    if (token->tok == COLUMN)
    {
        fifo_pop(lexer);
        return NULL; // no list, just ;
    }
    else
    {
        // {'\n'}
        while (1)
        {
            token = fifo_head(lexer);
            if (!token || token->tok != NEWLINE)
                break;
            fifo_pop(lexer);
        }
        // 'in'
        token = fifo_head(lexer);
        if (!token || token->tok != IN)
        {
            *status = PARSER_ERROR;
            return NULL;
        }
        fifo_pop(lexer);
        // { WORD }
        while (1)
        {
            token = fifo_head(lexer);
            if (!token || token->tok != WORDS)
                break;
            inside = add_word_to_words(inside, token->value);
            fifo_pop(lexer);
        }
        // ( ';' | '\n' )
        token = fifo_head(lexer);
        if (!token || (token->tok != COLUMN && token->tok != NEWLINE))
        {
            for (int i = 0; inside && inside[i]; i++)
            {
                free(inside[i]);
            }
            free(inside);
            *status = PARSER_ERROR;
            return NULL;
        }
        fifo_pop(lexer);
    }
    return inside;
}
