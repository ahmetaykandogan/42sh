#include "parser.h"
// input =
// list '\n'
// | list EOF
// | '\n'
// | EOF
// ;
struct ast *parse_input(enum parser_status *status, struct lexer *lexer)
{
    struct token token = lexer_peek(lexer); //dependent on Lexer implementation
    if (!token)
    {
        //Maybe error?
        return NULL;
    }
    //EOF
    if (token.type == TOKEN_EOF)
    {
        return NULL; //nothing to be done
    }
    //'\n'
    else if (token.type == TOKEN_NEWLINE)
    {
        lexer_pop(lexer); //consume '\n' 
        return NULL; //nothing to be done
    }
    //list
    struct ast *child = parse_list(status, lex); //start of the ast
    if (*status != PARSER_OK)
    {
        printf("Error in parse_input\n");
        return NULL; //error
    }
    // There is a repeat here, may be optimized later
    token = lexer_peek(lexer); //dependent on Lexer implementation
    if (!token)
    {
        //Maybe error?
        return NULL;
    }
    //EOF
    if (token.type == TOKEN_EOF)
    {
        return child; //nothing to be done
    }
    //'\n'
    else if (token.type == TOKEN_NEWLINE)
    {
        lexer_pop(lexer); //consume '\n' 
        return child; //nothing to be done
    }
    return NULL; //error we didnt get any \n or EOF
}

//list = and_or { ';' and_or } [ ';' ] ;

struct ast *parse_list(enum parser_status *status, struct lexer *lexer)
{
    struct ast *child = parse_and_or(status, lexer);
    if (*status != PARSER_OK)
    {
        printf("Error in parse_list\n");
        return NULL; //error
    }
    while (1)
    {
        struct token token = lexer_peek(lexer);
        if (token.type == TOKEN_SEMICOLON)
        {
            lexer_pop(lexer);
            struct ast *next_child = parse_and_or(status, lexer);
            if (*status != PARSER_OK)
            {
                printf("Error in parse_list\n");
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
    return child;
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

//To ask: What is WORD?
struct ast *parse_simple_command(enum parser_status *status, struct lexer *lexer)
{
    //Implement WORD
    //Token WORD
    //WORD is anything that isnt reserved
    //Struct AST implementation
    // { element }
    while (status == PARSER_OK)
    {
        struct ast *child = parse_element(status, lexer);
        if (*status != PARSER_OK)
        {
            status == PARSER_OK; //No more elements to parse
            break;
        }
    }
    return child;
}

//element = WORD ;

struct ast *parse_element(enum parser_status *status, struct lexer *lexer)
{
    //Again, what is WORD?
    return NULL;
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