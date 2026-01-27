#ifndef PARSER_H
#define PARSER_H

#include "../ast/ast.h"
#include "../utils/fifo/fifo.h"

enum parser_status
{
    PARSER_OK,
    PARSER_ERROR,
    PARSER_CRITICAL_ERROR,
};

struct ast *main_parse(enum parser_status *status, struct fifo *lexer);
struct ast *parse_input(enum parser_status *status, struct fifo *lexer);
struct ast *parse_list(enum parser_status *, struct fifo *);
struct ast *parse_and_or(enum parser_status *, struct fifo *);
struct ast *parse_pipeline(enum parser_status *, struct fifo *);

struct ast *parse_command(enum parser_status *, struct fifo *);
struct ast *parse_simple_command(enum parser_status *status, struct fifo *lexer, char *var);
void parse_element(enum parser_status *status, struct fifo *lexer,
                   struct ast *in);

struct ast *parse_shell_command(enum parser_status *, struct fifo *);
struct ast *parse_rule_if(enum parser_status *, struct fifo *);
struct ast *parse_rule_while(enum parser_status *, struct fifo *);
struct ast *parse_rule_until(enum parser_status *, struct fifo *);
struct ast *parse_rule_for(enum parser_status *, struct fifo *);
struct ast *parse_compound_list(enum parser_status *, struct fifo *);
struct ast *parse_else_clause(enum parser_status *, struct fifo *);
struct ast *parse_command_block(enum parser_status *status,
                                     struct fifo *lexer);
struct ast *parse_words(enum parser_status *status, struct fifo *lexer);

struct redirection *parse_redirections(enum parser_status *, struct fifo *);
struct redirection *parse_prefix(enum parser_status *, struct fifo *);
char *parse_prefix_vars(enum parser_status *, struct fifo *);

char **create_parse_for_body(enum parser_status *, struct fifo *);
#endif /* ! PARSER_H */
