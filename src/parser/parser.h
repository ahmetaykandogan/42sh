#ifndef PARSER_H
#define PARSER_H

#include <stdio.h>

#include "../lexer/lexer.h"
#include "../ast/ast.h"

enum parser_status
{
    PARSER_OK,
    PARSER_ERROR,
};

struct ast *parse_input(enum parser_status *status, struct fifo *lexer);

#endif /* ! PARSER_H */
