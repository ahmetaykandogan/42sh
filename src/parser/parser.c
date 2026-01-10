#include "parser.h"
// input =
// list '\n'
// | list EOF
// | '\n'
// | EOF
// ;


//La fonction au dessus va recuperer l'ast 
struct ast *parse_input(enum parser_status *status, struct lextoken *lexer)
{
    struct lextoken token = lexer_peek(lexer); //dependent on Lexer implementation
    if (!token)
    {
        //Maybe error?
        return NULL;
    }
    //list
    struct ast *out = parse_list(status, lex); //start of the ast
    if (*status == PARSER_OK)
    {
        //EOF
        if (token.type == TOKEN_EOF)
        {
            return out;
        }
        //'\n'
        else if (token.type == TOKEN_NEWLINE)
        {
            return out;
        }
        else
        {
            return NULL; //error
        }
    }
    else
    {
        if (token.type == TOKEN_EOF)
        {
            return NULL;//nothing to be done
        }
        else if (token.type == TOKEN_NEWLINE)
        {
            return NULL;//nothing to be done
        }
    }
    return NULL; //error
}

//list = and_or { ';' and_or } [ ';' ] ;
//struct ast_list start
struct ast *parse_list(enum parser_status *status, struct lexer *lexer)
{
    struct ast_list *list = ast_list_init();
    struct ast *child = parse_and_or(status, lexer); //return a cmd
    if (*status != PARSER_OK)
    {
        printf("Error in parse_list\n");
        return NULL; //error
    }

    ast_list_add_ast(list, &child->base); //adding the cmd to the list

    // { ';' and_or }
    while (1)
    {
        struct lextoken token = lexer_peek(lexer);
        if (token.type == TOKEN_SEMICOLON)
        {
            lexer_pop(lexer);
            struct ast *to_add = parse_and_or(status, lexer); //return a cmd
            if (*status == PARSER_OK)
            {
                ast_list_add_ast(list, &to_add->base); //adding the cmd to the list
            }
            else
            {
                printf("Error in parse_list");
                return NULL; //error
            }
        }
        else
        {
            break;
        }
    }
    struct token token = lexer_peek(lexer);
    if (token.type == TOKEN_SEMICOLON)
    {
        lexer_pop(lexer);
    }
    return &list->base;
}

//and_or = pipeline ;

struct ast *parse_and_or(enum parser_status *status, struct lexer *lexer)
{
    struct ast *child = parse_pipeline(status, lexer);
    if (*status != PARSER_OK)
    {
        printf("Error in parse_and_or\n");
        return NULL; //error
    }
    return child;
}

//pipeline = command ;
struct ast *parse_pipeline(enum parser_status *status, struct lexer *lexer)
{
    struct ast *child = parse_command(status, lexer);
    if (*status != PARSER_OK)
    {
        printf("Error in parse_pipeline\n");
        return NULL; //error
    }
    return child;
}

//command =
//simple_command
//| shell_command


struct ast *parse_command(enum parser_status *status, struct lexer *lexer)
{
    struct ast *child = parse_simple_command(status, lexer);
    if (*status != PARSER_OK)
    {
        *status = PARSER_OK;
        child = parse_shell_command(status, lexer);
        if (*status != PARSER_OK)
        {
            printf("Error in parse_command\n");
            return NULL; //error
        }
        else 
        {
            parser_status = PARSER_OK; 
        }
    }
    return child;
}

//simple_command = WORD { element } ;

struct ast *parse_simple_command(enum parser_status *status, struct lexer *lexer)
{
    //WORD
    struct lextoken token = lexer_peek(lexer);
    struct ast_cmd *command = ast_cmd_init();
    if (token.type == TOKEN_WORDS)
    {
        lexer_pop(lexer);
        ast_cmd_add_word(command, token->value);
    }
    else
    {
        *status = PARSER_ERROR;
        return NULL;
    }
    // { element }
    while (1)
    {
        parse_element(status, lexer, command->base);
        if (*status != PARSER_OK)
        {
            *status = PARSER_OK; //No more elements to parse
            break;
        }
    }
    return &command->base;
}

//element = WORD ;

void *parse_element(enum parser_status *status, struct lexer *lexer, struct ast *in)
{
    struct ast_cmd *out = (struct ast_cmd *)in; //its always a command
    struct lextoken token = lexer_peek(lexer);
    if (token.type == TOKEN_WORDS)
    {
        ast_cmd_add_word(out, token->value);
        lexer_pop(lexer);
    }
    else
    {
        *status = PARSER_ERROR;
    }
}


//shell_command = rule_if ;

struct ast *parse_shell_command(enum parser_status *status, struct lexer *lexer)
{
    struct ast *child = parse_rule_if(status, lexer);
    if (*status != PARSER_OK)
    {
        printf("Error in parse_shell_command\n");
        return NULL; //error
    }
    return child;
}

//rule_if = 'if' compound_list 'then' compound_list [else_clause] 'fi' ;

struct ast *parse_rule_if(enum parser_status *status, struct lexer *lexer)
{
    struct token token = lexer_peek(lexer);
    if (token.type != TOKEN_IF)
    {
        *status = PARSER_ERROR;
        return NULL; //error
    }
    lexer_pop(lexer);

    struct ast *child = parse_compound_list(status, lexer);
    if (*status != PARSER_OK)
    {
        printf("Error in parse_rule_if\n");
        return NULL; //error
    }

    token = lexer_peek(lexer);
    if (token.type != TOKEN_THEN)
    {
        *status = PARSER_ERROR;
        return NULL; //error
    }
    lexer_pop(lexer);

    child = parse_compound_list(status, lexer);
    if (*status != PARSER_OK)
    {
        printf("Error in parse_rule_if\n");
        return NULL; //error
    }

    // [else_clause]
    child = parse_else_clause(status, lexer);
    if (*status != PARSER_OK)
    {
        status = PARSER_OK; //else_clause is optional
        return NULL; //error
    }
    token = lexer_peek(lexer);
    if (token.type != TOKEN_FI)
    {
        *status = PARSER_ERROR;
        return NULL; //error
    }
    lexer_pop(lexer);

    return child;
}

//else_clause =
//'else' compound_list
//| 'elif' compound_list 'then' compound_list [else_clause]


struct ast *parse_else_clause(enum parser_status *status, struct lexer *lexer)
{
    struct token token = lexer_peek(lexer);
    if (token.type == TOKEN_ELSE)
    {
        lexer_pop(lexer);
        struct ast *child = parse_compound_list(status, lexer);
        if (*status != PARSER_OK)
        {
            printf("Error in parse_else_clause\n");
            return NULL; //error
        }
        return child;
    }
    else if (token.type == TOKEN_ELIF)
    {
        lexer_pop(lexer);
        struct ast *child = parse_compound_list(status, lexer);
        if (*status != PARSER_OK)
        {
            printf("Error in parse_else_clause\n");
            return NULL; //error
        }
        token = lexer_peek(lexer);
        if (token.type != TOKEN_THEN)
        {
            *status = PARSER_ERROR;
            return NULL; //error
        }
        lexer_pop(lexer);
        child = parse_compound_list(status, lexer);
        if (*status != PARSER_OK)
        {
            printf("Error in parse_else_clause\n");
            return NULL; //error
        }
        // [else_clause]
        struct ast *next_child = parse_else_clause(status, lexer);
        if (*status != PARSER_OK)
        {
            status = PARSER_OK; //else_clause is optional
            return child; 
        }
        return child;
    }
    else
    {
        *status = PARSER_ERROR;
        return NULL; //error
    }
}

//compound_list = and_or [';'] {'\n'} ;

//compound_list =
//{'\n'} and_or { ( ';' | '\n' ) {'\n'} and_or } [';'] {'\n'} ;
//This version is the correct one but fuck it its too long Im not doing it till 
//I am srue
struct ast *parse_compound_list(enum parser_status *status, struct lexer *lexer)
{
    struct ast *child = parse_and_or(status, lexer);
    if (*status != PARSER_OK)
    {
        printf("Error in parse_compound_list\n");
        return NULL; //error
    }
    struct token token = lexer_peek(lexer);
    if (token.type == TOKEN_SEMICOLON)
    {
        lexer_pop(lexer);
    }
    if (token.type == TOKEN_NEWLINE)
    {
        while (token.type == TOKEN_NEWLINE)
        {
            lexer_pop(lexer);
            token = lexer_peek(lexer);
        }
    }
    return child;
}
