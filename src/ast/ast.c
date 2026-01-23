#include "ast.h"

#include <assert.h>
#include <err.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

#include "../builtins/builtins.h"
#include "variable.c"
#include "variable.h"

int if_value = 0;
static char **dup_words(char **src)
{
    size_t n = 0;
    while (src && src[n])
        n++;

    char **dst = calloc(n + 1, sizeof(char *));
    for (size_t i = 0; i < n; i++)
        dst[i] = my_strdup(src[i]);
    dst[n] = NULL;
    return dst;
}

static void expand_words(char **words, char **argv, int argc, int if_value)
{
    for (size_t i = 0; words && words[i]; i++)
        words[i] = expand_word(words[i], argv, argc, if_value);
}

static void free_words(char **words)
{
    if (!words)
        return;
    for (size_t i = 0; words[i]; i++)
        free(words[i]);
    free(words);
}

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
//-----------------LOOKUP TABLE FOR RUN FUNCTIONS-----------------

static void ast_run_cmd(struct ast *ast, int argc, char **argv)
{
    struct ast_cmd *ast_cmd = (struct ast_cmd *)ast;
    run_command(ast_cmd, argc, argv);
}

static void ast_run_if(struct ast *ast, int argc, char **argv)
{
    struct ast_if *ast_if = (struct ast_if *)ast;

    run_ast(ast_if->condition, argc, argv);

    if (if_value == 0)
        run_ast(ast_if->then_body, argc, argv);
    else if (ast_if->else_body)
        run_ast(ast_if->else_body, argc, argv);
}

static void ast_run_list(struct ast *ast, int argc, char **argv)
{
    struct ast_list *ast_list = (struct ast_list *)ast;
    for (size_t i = 0; i < ast_list->child_count; i++)
        run_ast(ast_list->children[i], argc, argv);
}

const char *ast_repr(enum ast_type type)
{
    static const char *const names[] = {
        [AST_CMD] = "CMD",       [AST_IF] = "IF",       [AST_LIST] = "LIST",
        [AST_WHILE] = "WHILE",   [AST_UNTIL] = "UNTIL", [AST_PIPE] = "PIPE",
        [AST_AND_OR] = "AND_OR",
    };
    return names[type];
}

static void ast_run_while(struct ast *ast, int argc, char **argv)
{
    struct ast_while *ast_w = (struct ast_while *)ast;
    while (1)
    {
        run_ast(ast_w->condition, argc, argv);
        if (if_value != 0)
            break;
        run_ast(ast_w->body, argc, argv);
    }
}

static void ast_run_until(struct ast *ast, int argc, char **argv)
{
    struct ast_until *ast_u = (struct ast_until *)ast;
    while (1)
    {
        run_ast(ast_u->condition, argc, argv);
        if (if_value == 0)
            break;
        run_ast(ast_u->body, argc, argv);
    }
}

static void ast_run_and_or(struct ast *ast, int argc, char **argv)
{
    struct ast_and_or *ast_ao = (struct ast_and_or *)ast;
    if (ast_ao->child_count == 0)
        return;

    run_ast(ast_ao->children->children[0], argc, argv);
    for (size_t i = 0; i < ast_ao->child_count - 1; i++)
    {
        int operator= ast_ao->op[i];
        if (operator== 0) // &&
        {
            if (if_value == 0)
                run_ast(ast_ao->children->children[i + 1], argc, argv);
            else
                continue;
        }
        else if (operator== 1) // ||
        {
            if (if_value != 0)
                run_ast(ast_ao->children->children[i + 1], argc, argv);
            else
                continue;
        }
    }
}
static char *expand_for_value(char *word, char **argv, int argc)
{
    char *dup = my_strdup(word);
    return expand_word(dup, argv, argc, if_value);
}

static void ast_run_for(struct ast *ast, int argc, char **argv)
{
    struct ast_for *ast_for = (struct ast_for *)ast;
    char *var_name = ast_for->var_name;
    char **list = ast_for->list;
    struct ast *body = ast_for->body;

    if (!list)
        return;

    for (size_t i = 0; list[i] != NULL; i++)
    {
        char *value = expand_for_value(list[i], argv, argc);
        var_env_set(var_name, value);
        free(value);
        run_ast(body, argc, argv);
    }
}

static int try_execute_builtin(char **argv)
{
    if (!argv || !argv[0])
        return -1;

    if (strcmp(argv[0], "true") == 0)
    {
        return builtins_true();
    }
    else if (strcmp(argv[0], "false") == 0)
    {
        return builtins_false();
    }
    else if (strcmp(argv[0], "echo") == 0)
    {
        return builtins_echo(argv);
    }

    return -1;
}
static void restored_fds(int stdin_cp, int stdout_cp, int stderr_cp)
{
    dup2(stdin_cp, STDIN_FILENO);
    dup2(stdout_cp, STDOUT_FILENO);
    dup2(stderr_cp, STDERR_FILENO);
    close(stdin_cp);
    close(stdout_cp);
    close(stderr_cp);
}
static pid_t fork_executor(struct ast_cmd *cmd, int in, int out,
                           int (*filedes)[2], size_t nb_child,
                           char **argv, int argc)
{
    pid_t pid = fork();
    if (pid != 0)
        return pid;

    if (in != STDIN_FILENO)
        dup2(in, STDIN_FILENO);
    if (out != STDOUT_FILENO)
        dup2(out, STDOUT_FILENO);

    if (filedes)
    {
        for (size_t k = 0; k < nb_child - 1; k++)
        {
            close(filedes[k][0]);
            close(filedes[k][1]);
        }
    }

    if (cmd->redirections)
        run_redirections(cmd->redirections);

    char **words = dup_words(cmd->words);
    expand_words(words, argv, argc, if_value);

    int builtin = try_execute_builtin(words);
    if (builtin != -1)
    {
        fflush(stdout);
        free_words(words);
        exit(builtin);
    }

    execvp(words[0], words);
    errx(1, "Couldn't execvp");
}


void run_pipe(struct ast *enter_pipe, int argc, char **argv)
{
    int stdin_cp = dup(STDIN_FILENO);
    int stdout_cp = dup(STDOUT_FILENO);
    int stderr_cp = dup(STDERR_FILENO);

    struct ast_pipe *pipe_node = (struct ast_pipe *)enter_pipe;
    struct ast_list *pipelines = pipe_node->pipelines;
    size_t nb_child = pipelines->child_count;

    if (nb_child < 2)
        return;

    int (*filedes)[2] = calloc(nb_child - 1, sizeof(int[2]));
    pid_t *pid_list = calloc(nb_child, sizeof(pid_t));

    for (size_t i = 0; i < nb_child - 1; i++)
        pipe(filedes[i]);

    for (size_t j = 0; j < nb_child; j++)
    {
        int in = (j == 0) ? STDIN_FILENO : filedes[j - 1][0];
        int out = (j == nb_child - 1) ? STDOUT_FILENO : filedes[j][1];

        pid_list[j] =
            fork_executor((struct ast_cmd *)pipelines->children[j],
                          in, out, filedes, nb_child, argv, argc);
    }

    for (size_t k = 0; k < nb_child - 1; k++)
    {
        close(filedes[k][0]);
        close(filedes[k][1]);
    }

    int wstatus;
    for (size_t x = 0; x < nb_child; x++)
        waitpid(pid_list[x], &wstatus, 0);

    if (WIFEXITED(wstatus))
        if_value = WEXITSTATUS(wstatus);
    else
        if_value = 1;

    if (pipe_node->negated)
        if_value = !if_value;

    restored_fds(stdin_cp, stdout_cp, stderr_cp);

    free(filedes);
    free(pid_list);
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

static void run_assignment(struct ast_cmd *cmd, int check)
{
    if (check == 1)
    {
        for (size_t i = 0; cmd->assignments[i] != NULL; i++)
        {
            char *eq = strchr(cmd->assignments[i], '=');
            if (eq)
            {
                *eq = '\0';
                char *name = cmd->assignments[i];
                char *value = eq + 1;
                var_env_set(name, value);
            }
        }
    }
    else
    {
        for (size_t i = 0; cmd->assignments[i] != NULL; i++)
        {
            char *eq = strchr(cmd->assignments[i], '=');
            if (eq)
            {
                *eq = '\0';
                char *name = cmd->assignments[i];
                var_env_set(name, "");
            }
        }
    }
}


void run_command(struct ast_cmd *cmd, int argc, char **argv)
{
    int stdin_cp = dup(STDIN_FILENO);
    int stdout_cp = dup(STDOUT_FILENO);
    int stderr_cp = dup(STDERR_FILENO);

    if (!cmd->words || !cmd->words[0])
    {
        if (cmd->redirections)
        {
            run_redirections(cmd->redirections);
            restored_fds(stdin_cp, stdout_cp, stderr_cp);
        }
        else if (cmd->assignments) // we only have variables, expansion is for
                                   // every cmd, so we add to global list
        {
            run_assignment(cmd, 1); //we add to everyone
        }
        return;
    }
    if (cmd->assignments)
        run_assignment(cmd, 0); //we only add "" to everyone
    char **words = dup_words(cmd->words);
    expand_words(words, argv, argc, if_value);
    if (cmd->redirections)
        run_redirections(cmd->redirections);
    // case where var is only for this line
    // run_assignment(cmd);
    int builtin_result = try_execute_builtin(words);
    if (builtin_result != -1)
    {
        if_value = builtin_result;
        fflush(stdout);
        restored_fds(stdin_cp, stdout_cp, stderr_cp);
        if (cmd->negated)
            if_value = !if_value;
        free_words(words);
        return;
    }

    int pid = fork();
    if (pid == 0)
    {
        execvp(words[0], words);
        err(2, "exec dont recognize it");
    }

    int wstatus;
    waitpid(pid, &wstatus, 0);

    if (WIFEXITED(wstatus))
        if_value = WEXITSTATUS(wstatus);
    else
        if_value = 1;
    restored_fds(stdin_cp, stdout_cp, stderr_cp);
    if (cmd->negated)
        if_value = !if_value;

    free_words(words);
}

void run_ast(struct ast *ast, int argc, char **argv)
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
        [AST_AND_OR] = &ast_run_and_or,
        [AST_FOR] = &ast_run_for,
    };
    (*run_functions[ast->type])(ast, argc, argv);
}
//----------------------------------DESTROY AST
// TYPES------------------------------

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
//--------------------------------------redirections-----------------------------

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

