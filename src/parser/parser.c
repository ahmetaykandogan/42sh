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

//list = and_or ;

struct ast *parse_list(enum parser_status *status, struct lexer *lexer)
{
    struct ast *child = parse_and_or(status, lexer);
    if (*status != PARSER_OK)
    {
        printf("Error in parse_list\n");
        return NULL; //error
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

//command = simple_command ;
struct ast *parse_command(enum parser_status *status, struct lexer *lexer)
{
    struct ast *child = parse_simple_command(status, lexer);
    if (*status != PARSER_OK)
    {
        printf("Error in parse_command\n");
        return NULL; //error
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
