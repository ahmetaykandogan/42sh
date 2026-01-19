#include "ast/ast.h"
#include "lexer/lexer.h"
#include "parser/parser.h"
#include "pretty_printer/pretty_printer.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "utils/fifo/fifo.h"

int main(int argc, char **argv)
{
    struct fifo *lex_queue;
    if (argc < 2)
    {
        lex_queue = lexing_from_stdin(stdin);
    }

    //pretty print mode pour debug et la testsuite, pas sûr si le parse input est correctement utilisé, à verifier
    if (strcmp(argv[1], "-p") == 0)
    {
        lex_queue = lexer(argv[2]);
        enum parser_status status = PARSER_OK;
        struct ast *ast = parse_input(&status, lex_queue);
        pretty_print_ast(ast);
        fifo_destroy(lex_queue);
        struct_ast_destroy(ast);
        return 0;
    }
    
    else if (strcmp(argv[1], "-c") == 0)
    {
        lex_queue = lexer(argv[2]);
    }

    else if (argv[1])
    {
        lex_queue = lexing_from_file(argv[1]);
    }
    enum parser_status status = PARSER_OK;
    struct ast *ast = main_parse(&status, lex_queue);
    run_ast(ast);
    fifo_destroy(lex_queue);
    if (!ast)
    {
        return 2; //exit with error 2
    }
    struct_ast_destroy(ast);
    return 0;
}
