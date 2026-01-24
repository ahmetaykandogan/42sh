#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ast/ast.h"
#include "lexer/lexer.h"
#include "parser/parser.h"
#include "pretty_printer/pretty_printer.h"
#include "utils/fifo/fifo.h"

int main(int argc, char **argv)
{
    struct fifo *lex_queue;

    /* There are three ways 42sh must read an input Shell program: */
    /* When no other source is provided, it must read commands from standard input */
    if (argc < 2)
    {
        lex_queue = lexing_from_stdin(stdin);
    }

    /* pretty print mode pour debug et la testsuite */
    else if (strcmp(argv[1], "-p") == 0)
    {
        lex_queue = lexer(argv[2]);
        enum parser_status status = PARSER_OK;
        struct ast *ast = parse_input(&status, lex_queue);
        pretty_print_ast(ast);
        fifo_destroy(lex_queue);
        struct_ast_destroy(ast);
        return 0;
    }

    /* It must read its input from a string given using -c */
    else if (strcmp(argv[1], "-c") == 0)
    {
        lex_queue = lexer(argv[2]);
    }

    /* It must read from a file directly given as a positional argument */
    else
    {
        lex_queue = lexing_from_file(argv[1]);
    }

    enum parser_status status = PARSER_OK;
    struct ast *ast = main_parse(&status, lex_queue);
    run_ast(ast, argc - 1, argv + 1);
    if (!ast)
    {
        fifo_destroy_failed(lex_queue); // if parsing has failed, the fifo
                                        // destroy dont free the words
        err(2, "Parsing has failed"); // exit with error 2
    }
    fifo_destroy(lex_queue);
    struct_ast_destroy(ast);
    return return_value_to_main();
}
