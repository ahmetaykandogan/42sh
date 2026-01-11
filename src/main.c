#include "ast/ast.h"
#include "lexer/lexer.h"
#include "parser/parser.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int main(int argc, char **argv)
{
    if (argc < 2)
    {
        struct fifo *lex_queue = lexing_from_stdin(stdin);
    }

    if (strcmp(argv[1], "-c") == 0)
    {
        struct fifo *lex_queue = lexer(argv[2]);
    }
    else if (argv[1])
    {
        struct fifo *lex_queue = lexing_from_file(argv[1]);
    }
    struct ast *ast = parse(lex_queue);
    run_ast(ast);
    fifo_destroy(lex_queue);
    struct_ast_destroy(ast);
    return 0;
}
