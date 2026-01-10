#ifndef PARSER_H
#define PARSER_H

#include "lexer/lexer.h"
#include "ast/ast.h"

enum parser_status
{
    PARSER_OK,
    PARSER_ERROR,
};

struct ast *parse_input(enum parser_status *status, struct lextoken *lexer);
#endif /* ! PARSER_H */
