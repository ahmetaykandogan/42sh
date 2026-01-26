#include <stdlib.h>

#include "ast.h"

void ast_cmd_add_word(struct ast_cmd *in, char *word)
{
    size_t count = 0;
    while (in->words[count] != NULL)
        count++;
    char **out = realloc(in->words, sizeof(char *) * (count + 2));
    out[count] = word;
    out[count + 1] = NULL;
    in->words = out;
}

void ast_list_add_ast(struct ast_list *in, struct ast *ast)
{
    size_t count = in->child_count;
    struct ast **out =
        realloc(in->children, sizeof(struct ast *) * (count + 1));
    out[count] = ast;
    in->children = out;
    in->child_count += 1;
}

void ast_and_or_add_ast(struct ast_and_or *in, struct ast *ast, int type)
{
    ast_list_add_ast(in->children, ast);
    in->child_count++;
    in->op = realloc(in->op, sizeof(int) * (in->child_count - 1));
    in->op[in->child_count - 2] = type;
    return;
}

void add_vars(struct ast_cmd *cmd, char *new_var)
{
    size_t count = 0;
    if (cmd->assignments)
    {
        while (cmd->assignments[count] != NULL)
            count++;
    }
    char **out = realloc(cmd->assignments, sizeof(char *) * (count + 2));
    out[count] = new_var;
    out[count + 1] = NULL;
    cmd->assignments = out;
}
