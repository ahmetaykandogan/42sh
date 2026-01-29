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
#include "functions.h"
#include "variable.h"

struct shell_status shell = { 0, 0, 0 };
// last status: what was the last command exit status
// should_exit: whether the shell should exit (For exit builtin)
// exit_status: the exit status to use when exiting the shell
static char* my_strdup(const char* s)
{
    if (!s)
        return NULL;
    size_t len = strlen(s);
    char* dup = malloc(len + 1);
    if (dup)
        strcpy(dup, s);
    return dup;
}

int return_value_to_main(void)
{
    return shell.last_status;
}
static char** dup_words(char** src)
{
    size_t n = 0;
    while (src && src[n])
        n++;

    char** dst = calloc(n + 1, sizeof(char*));
    for (size_t i = 0; i < n; i++)
        dst[i] = my_strdup(src[i]);
    dst[n] = NULL;
    return dst;
}

static void expand_words(char** words, char** argv, int argc,
                         struct shell_status shell)
{
    for (size_t i = 0; words && words[i]; i++)
        words[i] = expand_word(words[i], argv, argc, shell.last_status);
}

static void free_words(char** words)
{
    if (!words)
        return;
    for (size_t i = 0; words[i]; i++)
        free(words[i]);
    free(words);
}

//-----------------LOOKUP TABLE FOR RUN FUNCTIONS-----------------

static void ast_run_cmd(struct ast* ast, int argc, char** argv)
{
    struct ast_cmd* ast_cmd = (struct ast_cmd*)ast;
    run_command(ast_cmd, argc, argv);
}

static void ast_run_if(struct ast* ast, int argc, char** argv)
{
    struct ast_if* ast_if = (struct ast_if*)ast;

    run_ast(ast_if->condition, argc, argv);

    if (shell.last_status == 0)
        run_ast(ast_if->then_body, argc, argv);
    else if (ast_if->else_body)
        run_ast(ast_if->else_body, argc, argv);
}

static void ast_run_list(struct ast* ast, int argc, char** argv)
{
    struct ast_list* ast_list = (struct ast_list*)ast;
    for (size_t i = 0; i < ast_list->child_count; i++)
    {
        if (shell.should_exit == 3)
        {
            break;
        }
        run_ast(ast_list->children[i], argc, argv);
    }
    shell.should_exit = 0;
}

const char* ast_repr(enum ast_type type)
{
    static const char* const names[] = {
        [AST_CMD] = "CMD",       [AST_IF] = "IF",       [AST_LIST] = "LIST",
        [AST_WHILE] = "WHILE",   [AST_UNTIL] = "UNTIL", [AST_PIPE] = "PIPE",
        [AST_AND_OR] = "AND_OR",
    };
    return names[type];
}

static void ast_run_while(struct ast* ast, int argc, char** argv)
{
    struct ast_while* ast_w = (struct ast_while*)ast;
    while (1)
    {
        if (shell.should_exit == 2)
        {
            shell.should_exit = 0;
            break;
        }
        run_ast(ast_w->condition, argc, argv);
        if (shell.last_status != 0)
            break;
        run_ast(ast_w->body, argc, argv);
    }
}

static void ast_run_until(struct ast* ast, int argc, char** argv)
{
    struct ast_until* ast_u = (struct ast_until*)ast;
    while (1)
    {
        if (shell.should_exit == 2)
        {
            shell.should_exit = 0;
            break;
        }
        run_ast(ast_u->condition, argc, argv);
        if (shell.last_status == 0)
            break;
        run_ast(ast_u->body, argc, argv);
    }
}

static void ast_run_and_or(struct ast* ast, int argc, char** argv)
{
    struct ast_and_or* ast_ao = (struct ast_and_or*)ast;
    if (ast_ao->child_count == 0)
        return;

    run_ast(ast_ao->children->children[0], argc, argv);
    for (size_t i = 0; i < ast_ao->child_count - 1; i++)
    {
        int operator= ast_ao->op[i];
        if (operator== 0) // &&
        {
            if (shell.last_status == 0)
                run_ast(ast_ao->children->children[i + 1], argc, argv);
            else
                continue;
        }
        else if (operator== 1) // ||
        {
            if (shell.last_status != 0)
                run_ast(ast_ao->children->children[i + 1], argc, argv);
            else
                continue;
        }
    }
}
static char* expand_for_value(char* word, char** argv, int argc,
                              struct shell_status shell)
{
    char* dup = my_strdup(word);
    return expand_word(dup, argv, argc, shell.last_status);
}

static void ast_run_for(struct ast* ast, int argc, char** argv)
{
    struct ast_for* ast_for = (struct ast_for*)ast;
    char* var_name = ast_for->var_name;
    char** list = ast_for->list;
    struct ast* body = ast_for->body;

    if (!list)
        return;

    for (size_t i = 0; list[i] != NULL; i++)
    {
        if (shell.should_exit == 2)
        {
            shell.should_exit = 0;
            break;
        }
        char* value = expand_for_value(list[i], argv, argc, shell);
        var_env_set(var_name, value);
        free(value);
        run_ast(body, argc, argv);
    }
}

static void ast_run_func(struct ast* ast, int argc, char** argv)
{
    (void)argc;
    (void)argv;
    struct ast_function* ast_func = (struct ast_function*)ast;
    // We add the function to the global list
    func_env_set(ast_func->name, ast_func->body);
}
static int try_execute_builtin(char** argv, int argc)
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
    else if (strcmp(argv[0], "exit") == 0)
    {
        shell.should_exit = 1;
        shell.exit_status = builtins_exit(argv);
        return shell.exit_status;
    }
    else if (strcmp(argv[0], "break") == 0)
    {
        shell.should_exit = 2; // Indicate a break
        // There's no need for a builtin actually
        return 0;
    }
    else if (strcmp(argv[0], "continue") == 0)
    {
        shell.should_exit = 3; // Indicate a continue
        return 0;
    }
    else if (strcmp(argv[0], "cd") == 0)
    {
        return builtins_cd(argv);
    }
    else if (strcmp(argv[0], "export") == 0)
    {
        return builtins_export(argc, argv);
    }
    return -1;
}

static int try_execute_funcs(char** argv)
{
    if (!argv || !argv[0])
        return -1;

    struct ast* node = func_env_get(argv[0]);
    if (node == NULL)
        return -1;
    struct ast* body = (struct ast*)node;
    run_ast(body, 0, NULL);
    return shell.last_status;
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
static pid_t fork_executor(struct ast_cmd* cmd, int in, int out,
                           int (*filedes)[2], size_t nb_child, char** argv,
                           int argc)
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

    char** words = dup_words(cmd->words);
    expand_words(words, argv, argc, shell);

    int builtin = try_execute_builtin(words, argc);
    if (builtin != -1)
    {
        fflush(stdout);
        free_words(words);
        exit(builtin);
    }

    execvp(words[0], words);
    errx(1, "Couldn't execvp");
}

void run_pipe(struct ast* enter_pipe, int argc, char** argv)
{
    int stdin_cp = dup(STDIN_FILENO);
    int stdout_cp = dup(STDOUT_FILENO);
    int stderr_cp = dup(STDERR_FILENO);

    struct ast_pipe* pipe_node = (struct ast_pipe*)enter_pipe;
    struct ast_list* pipelines = pipe_node->pipelines;
    size_t nb_child = pipelines->child_count;

    if (nb_child < 2)
        return;

    int(*filedes)[2] = calloc(nb_child - 1, sizeof(int[2]));
    pid_t* pid_list = calloc(nb_child, sizeof(pid_t));

    for (size_t i = 0; i < nb_child - 1; i++)
        if (pipe(filedes[i]) < 0)
            err(2, "pipe failed");

    for (size_t j = 0; j < nb_child; j++)
    {
        int in = (j == 0) ? STDIN_FILENO : filedes[j - 1][0];
        int out = (j == nb_child - 1) ? STDOUT_FILENO : filedes[j][1];

        pid_list[j] = fork_executor((struct ast_cmd*)pipelines->children[j], in,
                                    out, filedes, nb_child, argv, argc);
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
        shell.last_status = WEXITSTATUS(wstatus);
    else
        shell.last_status = 1;

    if (pipe_node->negated)
        shell.last_status = !shell.last_status;

    restored_fds(stdin_cp, stdout_cp, stderr_cp);

    free(filedes);
    free(pid_list);
}

void run_redirections(struct redirection** redir)
{
    if (!redir)
        return;
    for (size_t i = 0; redir[i] != NULL; i++)
    {
        struct redirection* r = redir[i];
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

static void run_assignment(struct ast_cmd* cmd, int check)
{
    if (check == 1)
    {
        for (size_t i = 0; cmd->assignments[i] != NULL; i++)
        {
            char* eq = strchr(cmd->assignments[i], '=');
            if (eq)
            {
                *eq = '\0';
                char* name = cmd->assignments[i];
                char* value = eq + 1;
                var_env_set(name, value);
            }
        }
    }
    else
    {
        for (size_t i = 0; cmd->assignments[i] != NULL; i++)
        {
            char* eq = strchr(cmd->assignments[i], '=');
            if (eq)
            {
                *eq = '\0';
                char* name = cmd->assignments[i];
                var_env_set(name, "");
            }
        }
    }
}

void run_command(struct ast_cmd* cmd, int argc, char** argv)
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
            run_assignment(cmd, 1); // we add to everyone
        }
        return;
    }
    if (cmd->assignments)
        run_assignment(cmd, 0); // we only add "" to everyone
    char** words = dup_words(cmd->words);
    expand_words(words, argv, argc, shell);
    if (cmd->redirections)
        run_redirections(cmd->redirections);
    // case where var is only for this line
    // run_assignment(cmd);
    int builtin_result = try_execute_builtin(words);
    if (builtin_result == -1)
    {
        builtin_result = try_execute_funcs(words);
    }
    if (builtin_result != -1)
    {
        if (builtin_result != -1)
        {
            shell.last_status = builtin_result;
            fflush(stdout);
            restored_fds(stdin_cp, stdout_cp, stderr_cp);
            if (cmd->negated)
                shell.last_status = !shell.last_status;
            free_words(words);
            return;
        }
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
        shell.last_status = WEXITSTATUS(wstatus);
    else
        shell.last_status = 1;
    restored_fds(stdin_cp, stdout_cp, stderr_cp);
    if (cmd->negated)
        shell.last_status = !shell.last_status;

    free_words(words);
}

void run_ast(struct ast* ast, int argc, char** argv)
{
    if (shell.should_exit == 1)
        return;
    if (shell.should_exit == 2)
    {
        shell.should_exit = 0;
        // the break happened, the rest may continue
        return;
    }
    if (!ast)
        return;
    static run_type run_functions[] = {
        [AST_CMD] = &ast_run_cmd,       [AST_IF] = &ast_run_if,
        [AST_LIST] = &ast_run_list,     [AST_WHILE] = &ast_run_while,
        [AST_UNTIL] = &ast_run_until,   [AST_PIPE] = &run_pipe,
        [AST_AND_OR] = &ast_run_and_or, [AST_FOR] = &ast_run_for,
        [AST_FUNCTION] = &ast_run_func,
    };
    (*run_functions[ast->type])(ast, argc, argv);
}
