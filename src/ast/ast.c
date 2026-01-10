#include "ast.h"


struct ast_cmd *ast_cmd_init()
{
    struct ast_cmd *out = calloc(1, sizeof(struct ast_cmd));
    out->base.type = AST_CMD;
    out->words = calloc(1,sizeof(char *));
    out->nb_word = 0;
    return out;
}

struct ast_if *ast_if_init()
{
    struct ast_if *out = calloc(1, sizeof(struct ast_if));
    out->base.type = AST_IF;
    out->condition = NULL;
    out->then_body = NULL;
    out->else_body = NULL;
    return out;
}

struct ast_list *ast_list_init()
{
    struct ast_list *out = calloc(1, sizeof(struct ast_list));
    out->base.type = AST_LIST;
    out->children = calloc(1, sizeof(struct ast *));
    out->child_count = 0;
    return out;
}


