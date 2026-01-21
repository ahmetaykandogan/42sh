#ifndef PARSER_H
#define PARSER_H

#include <stdio.h>

#include "../ast/ast.h"
#include "../lexer/lexer.h"
#include "../utils/fifo/fifo.h"

enum parser_status
{
    PARSER_OK,
    PARSER_ERROR,
    PARSER_CRITICAL_ERROR,
};

struct ast *main_parse(enum parser_status *status, struct fifo *lexer);
struct ast *parse_input(enum parser_status *status, struct fifo *lexer);

#endif /* ! PARSER_H */
