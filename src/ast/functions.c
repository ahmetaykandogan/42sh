// This is just so we can have a global function

#include "functions.h"

#include <err.h>
#include <stdlib.h>

#include "ast.h"

struct func_env *global_func_env = NULL;

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

struct func_env *func_env_init(void)
{
    struct func_env *env = calloc(1, sizeof(struct func_env));
    env->functions = NULL;
    env->func_count = 0;
    env->func_capacity = 0;
    return env;
}

struct func *func_init(const char *name, struct ast *body)
{
    struct func *new_func = malloc(sizeof(struct func));
    new_func->name = my_strdup(name);
    new_func->body = body;
    return new_func;
}

void func_env_set(const char *name, struct ast *body)
{
    if (global_func_env == NULL)
    {
        global_func_env = func_env_init();
    }
    if (global_func_env->func_count
        >= global_func_env->func_capacity) // resize if needed
    {
        if (global_func_env->func_capacity == 0) // initial capacity
            global_func_env->func_capacity = 4;
        else
            global_func_env->func_capacity *= 2; // double if needed
        global_func_env->functions =
            realloc(global_func_env->functions,
                    global_func_env->func_capacity * sizeof(struct func *));
    }
    for (size_t i = 0; i < global_func_env->func_count; i++)
    {
        if (strcmp(global_func_env->functions[i]->name, name)
            == 0) // check if the variable already exists
        {
            free(global_func_env->functions[i]->name);
            global_func_env->functions[i]->name = my_strdup(name);
            return;
        }
    }

    global_func_env->functions[global_func_env->func_count++] =
        func_init(name, body);
}

struct ast *func_env_get(const char *name)
{
    if (global_func_env == NULL)
    {
        return NULL;
    }
    for (size_t i = 0; i < global_func_env->func_count; i++)
    {
        if (strcmp(global_func_env->functions[i]->name, name) == 0)
        {
            return global_func_env->functions[i]->body;
        }
    }
    return NULL;
}
