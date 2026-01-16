#include "ast.h"
#include <assert.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdio.h>
#include <err.h>

int if_value = 0;

struct ast_cmd *ast_cmd_init(int neg)
{
    struct ast_cmd *out = calloc(1, sizeof(struct ast_cmd));
    out->base.type = AST_CMD;
    out->words = calloc(1, sizeof(char *));
    out->negated = neg;
    out->redirections = NULL;
    return out;
}

struct ast_if *ast_if_init()
{
    struct ast_if *out = calloc(1, sizeof(struct ast_if));
    out->base.type = AST_IF;
    out->condition = NULL;
    out->then_body = NULL;
    out->else_body = NULL;
    out->redirections = NULL;
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

struct ast_while *ast_while_init(void)
{
    struct ast_while *out= calloc(1, sizeof(struct ast_while));
    out->base.type = AST_WHILE;
    out->condition = NULL;
    out->body = NULL;
    out->redirections = NULL;
    return out;
}

struct ast_until *ast_until_init(void)
{
    struct ast_until *out= calloc(1, sizeof(struct ast_until));
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
    run_command(ast_cmd);
}

static void ast_run_if(struct ast *ast)
{
    struct ast_if *ast_if = (struct ast_if *)ast;

    run_ast(ast_if->condition);

    if (if_value == 0)
        run_ast(ast_if->then_body);
    else if (ast_if->else_body)
        run_ast(ast_if->else_body);
}

static void ast_run_list(struct ast *ast)
{
    struct ast_list *ast_list = (struct ast_list *)ast;
    for (size_t i = 0; i < ast_list->child_count; i++)
        run_ast(ast_list->children[i]);
}

const char *ast_repr(enum ast_type type)
{
    static const char *const names[] = {
        [AST_CMD] = "CMD",
        [AST_IF] = "IF",
        [AST_LIST] = "LIST",
        [AST_WHILE] = "WHILE",
        [AST_UNTIL] = "UNTIL",
        [AST_PIPE] = "PIPE",
    };
    return names[type];
}

static void ast_run_while(struct ast *ast)
{
    struct ast_while *ast_w = (struct ast_while *)ast;
    while (1)
    {
        run_ast(ast_w->condition);
        if (if_value != 0)
            break;
        run_ast(ast_w->body);
    }
}

static void ast_run_until(struct ast *ast)
{
    struct ast_until *ast_u = (struct ast_until *)ast;
    while (1)
    {
        run_ast(ast_u->condition);
        if (if_value == 0)
            break;
        run_ast(ast_u->body);
    }
}

void run_ast(struct ast *ast)
{
    if (!ast)
        return;
    static run_type run_functions[] = {
        [AST_CMD] = &ast_run_cmd,
        [AST_IF] = &ast_run_if,
        [AST_LIST] = &ast_run_list,
        [AST_WHILE] = &ast_run_while,
        [AST_UNTIL] = &ast_run_until,
        [AST_PIPE] = &run_pipe,
    };
    (*run_functions[ast->type])(ast);
}

void run_redirections(struct redirection **redir)
{
    if (!redir)
        return;
    for (size_t i = 0; redir[i] != NULL; i++)
    {
        struct redirection *r = redir[i];
        int fd;
        if (r->fd)
            fd = atoi(r->fd);
        else
        {
            if (r->operator[0] == '<')
                fd = 0;
            else
                fd = 1;
        }

        int flags;
        if (strcmp(r->operator, ">") == 0)
            flags = O_WRONLY | O_CREAT | O_TRUNC;
        else if (strcmp(r->operator, ">>") == 0)
            flags = O_WRONLY | O_CREAT | O_APPEND;
        else if (strcmp(r->operator, "<") == 0)
            flags = O_RDONLY;
        else
        {
            errx(2, "redirection operator %s not implemented", r->operator);
        }

        int file_desc = open(r->filename, flags, 0644);
        if (file_desc < 0)
        {
            err(2, "failed to open file %s", r->filename);
        }
        if (dup2(file_desc, fd) < 0)
        {
            err(2, "failed to duplicate file descriptor");
        }
        close(file_desc);
    }
}

void run_pipe(struct ast_pipe *pipe)
{
    if (!pipe || !pipe->pipelines || pipe->pipelines->child_count == 0)
        return;

    struct ast_list *pipelines = pipe->pipelines;
    struct ast *child = pipelines->children[0];
    int counter = 0;

    for (size_t i = 1; i < pipelines->child_count; i++)
    {
        struct ast *child2 = pipelines->children[i];
        if (child2->type != AST_CMD)
        {
            errx(2, "pipeline can only contain commands");
        }

        if (counter == 0)
        {
            run_command((struct ast_cmd *)child);
            counter++;
            continue;
        }
    }
}

void run_command(struct ast_cmd *cmd)
{
    char **words = cmd->words;
    if (!words || !words[0])
        return;

    int pid = fork();
    if (pid == 0)
    {
        run_redirections(cmd->redirections);
        execvp(words[0], words);
        err(2, "exec dont recognize it");
    }

    int wstatus;
    waitpid(pid, &wstatus, 0);

    if (WIFEXITED(wstatus))
        if_value = WEXITSTATUS(wstatus);
    else
        if_value = 1;

    if (cmd->negated)
        if_value = !if_value;
}

//----------------------------------DESTROY AST
//TYPES------------------------------

void struct_ast_destroy(struct ast *node)
{
    if (!node)
        return;
    static run_type destroy_functions[] = {
        [AST_CMD] = (run_type)&ast_cmd_destroy,
        [AST_IF] = (run_type)&ast_if_destroy,
        [AST_LIST] = (run_type)&ast_list_destroy,
        [AST_WHILE] = (run_type)&ast_while_destroy,
        [AST_UNTIL] = (run_type)&ast_until_destroy,
        [AST_PIPE] = (run_type)&ast_pipe_destroy,
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
//--------------------------------------redirections-----------------------------

struct redirection *redirection_init(char *fd, char *filename, char *operator)
{
    struct redirection *out = calloc(1, sizeof(struct redirection));
    out->fd = fd;
    //out->flags = flags;
    out->filename = filename;
    out->operator = NULL;
    out->operator = operator;
    return out;
}

void redirection_destroy(struct redirection *redir)
{
    if (!redir)
        return;
    free(redir);
}

//THERE IS PROABLY A CLEANER WAY TO DO THIS BUT I DIDNT FIND IT
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