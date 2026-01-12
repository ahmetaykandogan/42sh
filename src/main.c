#include "ast/ast.h"
#include "lexer/lexer.h"
#include "parser/parser.h"

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

    if (strcmp(argv[1], "-c") == 0)
    {
        lex_queue = lexer(argv[2]);
    }
    else if (argv[1])
    {
        lex_queue = lexing_from_file(argv[1]);
    }
    struct ast *ast = parse_input(PARSER_OK, lex_queue);
    run_ast(ast);
    fifo_destroy(lex_queue);
    struct_ast_destroy(ast);
    return 0;
}
