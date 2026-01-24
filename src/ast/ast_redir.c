#include "ast.h"

#include <stdlib.h>


struct redirection *redirection_init(char *fd, char *filename, char *operator)
{
    struct redirection *out = calloc(1, sizeof(struct redirection));
    out->fd = fd;
    // out->flags = flags;
    out->filename = filename;
    out->operator= NULL;
    out->operator= operator;
    return out;
}

void redirection_destroy(struct redirection *redir)
{
    if (!redir)
        return;
    free(redir->fd);
    free(redir->filename);
    free(redir->operator);
    free(redir);
}

// THERE IS PROABLY A CLEANER WAY TO DO THIS BUT I DIDNT FIND IT
void ast_add_redirection(struct ast *node, struct redirection *redir)
{
    if (!node || !redir)
        return;

    static redir_type redirection_functions[] = {
        [AST_CMD] = ast_cmd_add_redirection,
        [AST_IF] = ast_if_add_redirection,
        [AST_WHILE] = ast_while_add_redirection,
        [AST_UNTIL] = ast_until_add_redirection,
    };

    redirection_functions[node->type](node, redir);
}

void ast_cmd_add_redirection(struct ast *ast_node, struct redirection *redir)
{
    struct ast_cmd *node = (struct ast_cmd *)ast_node;
    add_redirection(&node->redirections, redir);
}

void ast_if_add_redirection(struct ast *ast_node, struct redirection *redir)
{
    struct ast_if *node = (struct ast_if *)ast_node;
    add_redirection(&node->redirections, redir);
}

void ast_while_add_redirection(struct ast *ast_node, struct redirection *redir)
{
    struct ast_while *node = (struct ast_while *)ast_node;
    add_redirection(&node->redirections, redir);
}

void ast_until_add_redirection(struct ast *ast_node, struct redirection *redir)
{
    struct ast_until *node = (struct ast_until *)ast_node;
    add_redirection(&node->redirections, redir);
}

void add_redirection(struct redirection ***ast_redir,
                     struct redirection *new_redir)
{
    size_t count = 0;

    if (*ast_redir)
        while ((*ast_redir)[count] != NULL)
            count++;

    struct redirection **out =
        realloc(*ast_redir, sizeof(struct redirection *) * (count + 2));

    out[count] = new_redir;
    out[count + 1] = NULL;
    *ast_redir = out;
}
