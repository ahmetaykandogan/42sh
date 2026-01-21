#include "ast.h"
#include <assert.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdio.h>
#include <err.h>
#include <string.h>
#include "variable.h"
#include "variable.c"

#include "../builtins/builtins.h"

int if_value = 0;

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

struct ast_and_or *ast_and_or_init(void)
{
    struct ast_and_or *out = calloc(1, sizeof(struct ast_and_or));
    out->base.type = AST_AND_OR;
    out->children = ast_list_init();
    out->op = NULL;
    out->child_count = 1;
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
        [AST_AND_OR] = "AND_OR",
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

static void ast_run_and_or(struct ast *ast)
{
    struct ast_and_or *ast_ao = (struct ast_and_or *)ast;
    if (ast_ao->child_count == 0)
        return;

    run_ast(ast_ao->children->children[0]);
    for (size_t i = 0; i < ast_ao->child_count - 1; i++)
    {
        int operator = ast_ao->op[i];
        if (operator == 0) // &&
        {
            if (if_value == 0)
                run_ast(ast_ao->children->children[i + 1]);
            else
                continue;
        }
        else if (operator == 1) // ||
        {
            if (if_value != 0)
                run_ast(ast_ao->children->children[i + 1]);
            else
                continue;
        }
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

static pid_t fork_executor(char **argv, int in, int out, int (*filedes)[2], size_t nb_child)
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
        for (size_t k = 0; k < (nb_child ? nb_child - 1 : 0); k++)
        {
            close(filedes[k][0]);
            close(filedes[k][1]);
        }
    }

    int builtin_executed = try_execute_builtin(argv);
    if (builtin_executed != -1)
    {
        fflush(stdout);
        exit(builtin_executed);
    }

    execvp(argv[0], argv);
    errx(1, "Couldn't execvp");
}

void run_pipe(struct ast *enter_pipe)
{
    struct ast_pipe *pipe_node = (struct ast_pipe *)enter_pipe;
    struct ast_list *pipelines = pipe_node->pipelines;

    size_t nb_child = pipelines->child_count;
    if (nb_child < 2)
        return;

    int (*filedes)[2] = calloc(nb_child - 1, sizeof(int[2]));

    pid_t *pid_list = calloc(nb_child, sizeof(pid_t));

    for (size_t i = 0; i < nb_child - 1; i++)
        if (pipe(filedes[i]) != 0)
            errx(1, "Couldn't pipe!");

    for (size_t j = 0; j < nb_child; j++)
    {
        struct ast_cmd *cmd = (struct ast_cmd *)pipelines->children[j];
        int in;
        int out;

        if (j == 0)
            in = STDIN_FILENO;
        else
            in = filedes[j - 1][0];

        if (j == nb_child - 1)
            out = STDOUT_FILENO;
        else
            out = filedes[j][1];

        pid_list[j] = fork_executor(cmd->words, in, out, filedes, nb_child);
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
static void restored_fds(int stdin_cp, int stdout_cp, int stderr_cp)
{
    dup2(stdin_cp, STDIN_FILENO);
    dup2(stdout_cp, STDOUT_FILENO);
    dup2(stderr_cp, STDERR_FILENO);
    close(stdin_cp);
    close(stdout_cp);
    close(stderr_cp);
}

static char **expand_command_words(char **words, struct var_env *env, char **argv, int argc)
{
    if (!words)
        return NULL;
    size_t count = 0;

    for (size_t i = 0; words[i] != NULL; i++)
    {
        char **expanded = expand_word(words[i], env, argv, argc, if_value);
        size_t j = 0;
        while (expanded[j] && expanded)
        {
            count++;
            j++;
        }
        
        for (size_t k = 0; expanded && expanded[k]; k++)
            free(expanded[k]);
        free(expanded);
    }

    char **out = calloc(count + 1, sizeof(char *));
    size_t index = 0;

    for (size_t i = 0; words[i] != NULL; i++)
    {
        char **expanded = expand_word(words[i], env, argv, argc, if_value);
        for(size_t j = 0; expanded && expanded[j]; j++)
        {
            out[index++] = expanded[j];
        }
        free(expanded);
    }

    return out;
}

void run_command(struct ast_cmd *cmd)
{
    char **words = cmd->words;
    int stdin_cp = dup(STDIN_FILENO);
    int stdout_cp = dup(STDOUT_FILENO);
    int stderr_cp = dup(STDERR_FILENO);
    if (!words || !words[0])
    {
        if (cmd->redirections)
        {
            run_redirections(cmd->redirections);
            restored_fds(stdin_cp, stdout_cp, stderr_cp);
        }
        else if (cmd->assignments) // we only have variables, expansion is for every cmd, so we add to global list
        {

        }
        return;
    }
    //we save the fds to restore them later

    int argc = 0;
    while (words[argc] != NULL)
        argc++;

    if (cmd->assignments)
    {
        for (size_t i = 0; cmd->assignments[i] != NULL; i++) {
            char *eq = strchr(cmd->assignments[i], '=');
            if (eq) {
                *eq = '\0';
                char *name = cmd->assignments[i];
                char *value = eq + 1;
                
                char **expanded_value = expand_word(value, global_var_env, 
                                                    global_argv, global_argc, if_value);
                if (expanded_value && expanded_value[0]) {
                    var_env_set(global_var_env, name, expanded_value[0]);
                }
                
                for (size_t j = 0; expanded_value && expanded_value[j]; j++)
                    free(expanded_value[j]);
                free(expanded_value);
                
                *eq = '=';
            }
        }
    }

    char **expanded_words = expand_command_words(words, global_var_env, global_argv, global_argc);

    int builtin_result = try_execute_builtin(expanded_words);
    if (builtin_result != -1) {
        run_redirections(cmd->redirections);
        if_value = builtin_result;
        fflush(stdout);
        
        if (cmd->negated)
            if_value = !if_value;
        
        for (size_t i = 0; expanded_words && expanded_words[i]; i++)
            free(expanded_words[i]);
        free(expanded_words);
        return;
    }

    if (cmd->redirections)
        run_redirections(cmd->redirections);
    builtin_result = try_execute_builtin(words);
    if (builtin_result != -1)
    {
        if_value = builtin_result;
        fflush(stdout);
        restored_fds(stdin_cp, stdout_cp, stderr_cp);
        if (cmd->negated)
            if_value = !if_value;
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

    for (size_t i = 0; expanded_words[i] && expanded_words; i++)
        free(expanded_words[i]);
    free(expanded_words);

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
        [AST_AND_OR] = &ast_run_and_or,
    };
    (*run_functions[ast->type])(ast);
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
        [AST_AND_OR] = (run_type)&ast_and_or_destroy,
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
    free(redir->fd);
    free(redir->filename);
    free(redir->operator);
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
