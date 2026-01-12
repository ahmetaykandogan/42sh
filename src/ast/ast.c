#include "ast.h"
#include <assert.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdio.h>
struct ast_cmd *ast_cmd_init()
{
    struct ast_cmd *out = calloc(1, sizeof(struct ast_cmd));
    out->base.type = AST_CMD;
    out->words = calloc(1, sizeof(char *));
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

//--------------------------------ADDINGS------------------------------------

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

//-----------------LOOKUP TABLE FOR RUN FUNCTIONS-----------------
static void ast_run_cmd(struct ast *ast)
{
    struct ast_cmd *ast_cmd = (struct ast_cmd *)ast;
    run_command(ast_cmd->words);
}

static void ast_run_if(struct ast *ast)
{
    struct ast_if *ast_if = (struct ast_if *)ast;
    run_ast(ast_if->condition);
    run_ast(ast_if->then_body);
    if (ast_if->else_body)
        run_ast(ast_if->else_body);
}

static void ast_run_list(struct ast *ast)
{
    struct ast_list *ast_list = (struct ast_list *)ast;
    for (size_t i = 0; i < ast_list->child_count; i++)
    {
        run_ast(ast_list->children[i]);
    }
}
const char *ast_repr(enum ast_type type)
{
    static const char *const names[] = {
        [AST_CMD] = "CMD",
        [AST_IF] = "IF",
        [AST_LIST] = "LIST",
    };
    return names[type];
}

void run_ast(struct ast *ast)
{
    if (!ast)
        return;
    static run_type run_functions[] = {
        [AST_CMD] = &ast_run_cmd,
        [AST_IF] = &ast_run_if,
        [AST_LIST] = &ast_run_list,
    };
    (*run_functions[ast->type])(ast);
}
void run_command(char **words)
{
    if (!words || !words[0])
        return;

    pid_t pid = fork();
    if (pid < 0)
    {
        perror("fork");
        return;
    }

    if (pid == 0)
    {
        execvp(words[0], words);
        perror("execvp");
        _exit(127);
    }

    int wstatus;
    if (waitpid(pid, &wstatus, 0) == -1)
    {
        perror("waitpid");
    }
}

//----------------------------------DESTROY AST
//TYPES------------------------------

void struct_ast_destroy(
    struct ast *node) // the generic destroy. Same as lookup table
{
    if (!node)
        return;
    static run_type destroy_functions[] = {
        [AST_CMD] = (run_type)&ast_cmd_destroy,
        [AST_IF] = (run_type)&ast_if_destroy,
        [AST_LIST] = (run_type)&ast_list_destroy,
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
    {
        struct_ast_destroy(node->children[i]);
    }
    free(node->children);
    free(node);
}
