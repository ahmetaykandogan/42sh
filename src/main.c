#include "ast/ast.h"
#include "lexer/lexer.h"
#include "parser/parser.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "utils/fifo.h"

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
        pretty_print_ast(parse_input(argv[2], lex_queue));
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
    struct ast *ast = parse_input(&status, lex_queue);
    run_ast(ast);
    fifo_destroy(lex_queue);
    struct_ast_destroy(ast);
    return 0;
}
