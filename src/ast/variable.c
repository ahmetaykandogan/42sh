#include "variable.h"

#include <ctype.h>
#include <err.h>
#include <limits.h>
#include <linux/limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "ast.h"

// Le fichier est à clang-tidy
static char *remove_char(char *line, int index)
{
    size_t i = index;
    while (i < strlen(line) - 1)
    {
        line[i] = line[i + 1];
        i++;
    }
    line[strlen(line) - 1] = '\0';
    // line = realloc(line, strlen(line)-1);
    return line;
}

static char *my_strdup(const char *s)
{
    if (!s)
        return NULL;
    size_t len = strlen(s);
    char *dup = malloc(len + 1);
    if (dup)
        strcpy(dup, s);
    return dup;
}

//=====================INIT/DESTROY=========================
struct var_env *var_env_init()
{
    struct var_env *env = calloc(1, sizeof(struct var_env));
    env->variables = NULL;
    env->var_count = 0;
    env->var_capacity = 0;
    return env;
}
struct var_env *global_var_env = NULL;

struct var *var_init(const char *name, const char *value)
{
    struct var *new_var = malloc(sizeof(struct var));
    new_var->name = my_strdup(name);
    new_var->value = my_strdup(value);
    return new_var;
}

void var_destroy(struct var *var)
{
    if (!var)
        return;
    free(var->name);
    free(var->value);
    free(var);
}

//=====================SET/GET/UNSET=========================

void var_env_set(const char *name, const char *value) // add a new variable
{
    if (global_var_env == NULL)
    {
        global_var_env = var_env_init();
    }
    if (global_var_env->var_count
        >= global_var_env->var_capacity) // resize if needed
    {
        if (global_var_env->var_capacity == 0) // initial capacity
            global_var_env->var_capacity = 4;
        else
            global_var_env->var_capacity *= 2; // double if needed
        global_var_env->variables =
            realloc(global_var_env->variables,
                    global_var_env->var_capacity * sizeof(struct var *));
    }
    for (size_t i = 0; i < global_var_env->var_count; i++)
    {
        if (strcmp(global_var_env->variables[i]->name, name)
            == 0) // check if the variable already exists
        {
            free(global_var_env->variables[i]->value);
            global_var_env->variables[i]->value = my_strdup(value);
            return;
        }
    }

    global_var_env->variables[global_var_env->var_count++] =
        var_init(name, value);
}

void var_env_unset(const char *name)
{
    for (size_t i = 0; i < global_var_env->var_count; i++)
    {
        if (strcmp(global_var_env->variables[i]->name, name) == 0)
        {
            var_destroy(global_var_env->variables[i]);
            global_var_env->variables[i] =
                global_var_env->variables[global_var_env->var_count - 1];
            global_var_env->var_count--;
            return;
        }
    }
}

char *var_env_get(const char *name)
{
    if (global_var_env == NULL)
    {
        err(2, "Variable dont exist");
    }
    for (size_t i = 0; i < global_var_env->var_count; i++)
    {
        if (strcmp(global_var_env->variables[i]->name, name) == 0)
        {
            return global_var_env->variables[i]->value;
        }
    }
    return NULL;
}

//=====================EXPANSION=========================

char *expand_special_var(const char *var_name, char **argv, int argc,
                         int last_exit_status)
{
    if (!var_name)
        return NULL;

    char buffer[64];

    // $?
    if (strcmp(var_name, "?") == 0)
    {
        snprintf(buffer, sizeof(buffer), "%d", last_exit_status);
        return my_strdup(buffer);
    }

    // $$
    if (strcmp(var_name, "$") == 0)
    {
        snprintf(buffer, sizeof(buffer), "%d", getpid());
        return my_strdup(buffer);
    }

    // $#
    if (strcmp(var_name, "#") == 0)
    {
        snprintf(buffer, sizeof(buffer), "%d", argc - 1);
        return my_strdup(buffer);
    }

    // $0, $1, ..., $n
    if (isdigit(var_name[0]) && var_name[1] == '\0')
    {
        int idx = var_name[0] - '0';
        if (idx < argc)
            return my_strdup(argv[idx]);
        return my_strdup("");
    }

    // $* - tous les args séparés par des espaces
    if (strcmp(var_name, "*") == 0)
    {
        if (argc <= 1)
            return my_strdup("");

        size_t total_len = 0;
        for (int i = 1; i < argc; i++)
            total_len += strlen(argv[i]) + 1;

        char *result = malloc(total_len);
        result[0] = '\0';
        for (int i = 1; i < argc; i++)
        {
            strcat(result, argv[i]);
            if (i < argc - 1)
                strcat(result, " ");
        }
        return result;
    }

    // $@ - géré dans expand_word directement
    if (strcmp(var_name, "@") == 0)
    {
        return expand_special_var("*", argv, argc, last_exit_status);
    }

    // $RANDOM
    if (strcmp(var_name, "RANDOM") == 0)
    {
        snprintf(buffer, sizeof(buffer), "%d", rand() % 67);
        return my_strdup(buffer);
    }

    // $UID
    if (strcmp(var_name, "UID") == 0)
    {
        snprintf(buffer, sizeof(buffer), "%d", getuid());
        return my_strdup(buffer);
    }

    // Variables d'environnement
    if (strcmp(var_name, "PWD") == 0)
    {
        char cwd[PATH_MAX];
        if (getcwd(cwd, sizeof(cwd)))
            return my_strdup(cwd);
        return my_strdup("");
    }

    if (strcmp(var_name, "OLDPWD") == 0)
    {
        char *oldpwd = getenv("OLDPWD");
        return my_strdup(oldpwd);
    }

    if (strcmp(var_name, "IFS") == 0)
    {
        char *ifs = getenv("IFS");
        return my_strdup(ifs);
    }

    return NULL;
}

static char *clean_word(char *word)
{
    // erase $
    word = remove_char(word, 0);
    // check for {}
    if (word[0] == '{')
    {
        word = remove_char(word, 0);
        size_t len = strlen(word);
        if (word[len - 1] == '}')
        {
            word = remove_char(word, len - 1);
        }
        else
        {
            // syntax error, missing } //error should be done here?
            err(2, "syntax error: missing }");
        }
    }
    return word;
}
char *expand_word(char *word, char **argv, int argc, int if_value)
{
    if (!word)
        return NULL;
    if (word[0] != '$')
        return word;
    word = clean_word(word); // turn ${VAR} to VAR
    // check for special variables
    char *special = expand_special_var(word, argv, argc, if_value);
    if (special)
    {
        free(word);
        return special;
    }
    // normal variable
    char *value = var_env_get(word);
    if (value)
    {
        free(word);
        return my_strdup(value);
    }
    free(word);
    return my_strdup("");
}
