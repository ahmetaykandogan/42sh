#include "ast/ast.h"
#include "lexer/lexer.h"
#include "parser/parser.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int main(int argc, char **argv)
{
    if (strcmp(argv[1], "-c") == 0)
    {
        lexer(argv[1]);
    }
    else if (argv[1])
    {
        char *script = malloc(sizeof(char *));
        read_file(argv[1], script);
        lexer(script);
        free(script);
    }
    else
    {
        char *script = malloc(sizeof(char *));
        read_file(stdin, script);
        lexer(script);
        free(script);
    }
}
