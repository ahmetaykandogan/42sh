#include <stdlib.h>

#include "ast.h"

struct ast_cmd *ast_cmd_init(int neg)
{
    struct ast_cmd *out = calloc(1, sizeof(struct ast_cmd));
    out->base.type = AST_CMD;
    out->words = calloc(1, sizeof(char *));
    out->negated = neg;
    out->redirections = NULL;
    out->assignments = NULL;
    return out;
}

struct ast_if *ast_if_init(void)
{
    struct ast_if *out = calloc(1, sizeof(struct ast_if));
    out->base.type = AST_IF;
    out->condition = NULL;
    out->then_body = NULL;
    out->else_body = NULL;
    out->redirections = NULL;
    return out;
}

struct ast_list *ast_list_init(void)
{
    struct ast_list *out = calloc(1, sizeof(struct ast_list));
    out->base.type = AST_LIST;
    out->children = calloc(1, sizeof(struct ast *));
    out->child_count = 0;
    return out;
}

struct ast_while *ast_while_init(void)
{
    struct ast_while *out = calloc(1, sizeof(struct ast_while));
    out->base.type = AST_WHILE;
    out->condition = NULL;
    out->body = NULL;
    out->redirections = NULL;
    return out;
}

struct ast_until *ast_until_init(void)
{
    struct ast_until *out = calloc(1, sizeof(struct ast_until));
    out->base.type = AST_UNTIL;
    out->condition = NULL;
    out->body = NULL;
    out->redirections = NULL;
    return out;
}

struct ast_pipe *ast_pipeline_init(int neg)
{
    struct ast_pipe *out = calloc(1, sizeof(struct ast_pipe));
    out->base.type = AST_PIPE;
    out->pipelines = ast_list_init();
    out->negated = neg;
    out->redirections = NULL;
    return out;
}

struct ast_and_or *ast_and_or_init(void)
{
    struct ast_and_or *out = calloc(1, sizeof(struct ast_and_or));
    out->base.type = AST_AND_OR;
    out->children = ast_list_init();
    out->op = NULL;
    out->child_count = 1;
    return out;
}

struct ast_for *ast_for_init(void)
{
    struct ast_for *out = calloc(1, sizeof(struct ast_for));
    out->base.type = AST_FOR;
    out->var_name = NULL;
    out->list = NULL;
    out->body = NULL;
    return out;
}

struct ast_function *ast_function_init(void)
{
    struct ast_function *out = calloc(1, sizeof(struct ast_function));
    out->base.type = AST_FUNCTION;
    out->name = NULL;
    out->body = NULL;
    return out;
}