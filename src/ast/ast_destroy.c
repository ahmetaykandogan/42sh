#include <stdlib.h>

#include "ast.h"
#include "functions.h"
void struct_ast_destroy(struct ast *node)
{
    if (!node)
        return;
    static destroy_type destroy_functions[] = {
        [AST_CMD] = (destroy_type)&ast_cmd_destroy,
        [AST_IF] = (destroy_type)&ast_if_destroy,
        [AST_LIST] = (destroy_type)&ast_list_destroy,
        [AST_WHILE] = (destroy_type)&ast_while_destroy,
        [AST_UNTIL] = (destroy_type)&ast_until_destroy,
        [AST_PIPE] = (destroy_type)&ast_pipe_destroy,
        [AST_AND_OR] = (destroy_type)&ast_and_or_destroy,
        [AST_FOR] = (destroy_type)&ast_for_destroy,
        [AST_FUNCTION] = (destroy_type)&ast_func_destroy,
    };
    (*destroy_functions[node->type])(node);
}

void ast_cmd_destroy(struct ast_cmd *node)
{
    if (!node)
        return;
    size_t i = 0;
    while (node->words[i] != NULL)
    {
        free(node->words[i]);
        i++;
    }
    free(node->words);
    for (i = 0; node->redirections && node->redirections[i] != NULL; i++)
        redirection_destroy(node->redirections[i]);
    for (i = 0; node->assignments && node->assignments[i] != NULL; i++)
        free(node->assignments[i]);
    free(node->assignments);
    free(node->redirections);
    free(node);
}

void ast_if_destroy(struct ast_if *node)
{
    if (!node)
        return;
    struct_ast_destroy(node->condition);
    struct_ast_destroy(node->then_body);
    struct_ast_destroy(node->else_body);
    free(node);
}

void ast_list_destroy(struct ast_list *node)
{
    if (!node)
        return;
    for (size_t i = 0; i < node->child_count; i++)
        struct_ast_destroy(node->children[i]);
    free(node->children);
    free(node);
}

void ast_while_destroy(struct ast_while *node)
{
    if (!node)
        return;
    struct_ast_destroy(node->condition);
    struct_ast_destroy(node->body);
    free(node);
}

void ast_until_destroy(struct ast_until *node)
{
    if (!node)
        return;
    struct_ast_destroy(node->condition);
    struct_ast_destroy(node->body);
    free(node);
}

void ast_pipe_destroy(struct ast_pipe *node)
{
    if (!node)
        return;
    struct_ast_destroy((struct ast *)node->pipelines);
    free(node);
}

void ast_and_or_destroy(struct ast_and_or *node)
{
    if (!node)
        return;
    struct_ast_destroy((struct ast *)node->children);
    free(node->op);
    free(node);
}

void ast_for_destroy(struct ast_for *node)
{
    if (!node)
        return;
    free(node->var_name);
    if (node->list)
    {
        for (size_t i = 0; node->list[i] != NULL; i++)
        {
            free(node->list[i]);
        }
        free(node->list);
    }
    struct_ast_destroy(node->body);
    free(node);
}

void ast_func_destroy(struct ast_function *node)
{
    if (!node)
        return;
    free(node->name);
    struct_ast_destroy(node->body);
    free(node);
}
