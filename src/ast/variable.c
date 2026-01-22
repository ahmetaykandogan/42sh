#include "ast.h"
#include "variable.h"
#include <limits.h>
#include <linux/limits.h>
#include <ctype.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>


//Le fichier est à clang-tidy

static size_t my_strnlen(const char *s, size_t n) 
{
    size_t len = 0;
    while (len < n && s[len] != '\0') 
    {
        len++;
    }
    return len;
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

static char *my_strndup(const char *s, size_t n) 
{
    if (!s)
        return NULL;
    size_t len = my_strnlen(s, n);
    char *dup = malloc(len + 1);
    if (dup) 
    {
        strncpy(dup, s, len);
        dup[len] = '\0';
    }
    return dup;
}

//=====================INIT/DESTROY=========================

struct var_env *global_var_env = NULL;
char **global_argv = NULL;
int global_argc = 0;

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

void var_env_set(struct var_env *env, const char *name, const char *value) 
{
    for (size_t i = 0; i < env->var_count; i++) 
    {
        if (strcmp(env->variables[i]->name, name) == 0) 
        {
            free(env->variables[i]->value);
            env->variables[i]->value = my_strdup(value);
            return;
        }
    }
    
    if (env->var_count >= env->var_capacity) 
    {
        env->var_capacity = env->var_capacity ? env->var_capacity * 2 : 4;
        env->variables = realloc(env->variables, env->var_capacity * sizeof(struct var *));
    }
    
    env->variables[env->var_count++] = var_init(name, value);
}

void var_env_unset(struct var_env *env, const char *name) 
{
    for (size_t i = 0; i < env->var_count; i++) 
    {
        if (strcmp(env->variables[i]->name, name) == 0) 
        {
            var_destroy(env->variables[i]);
            env->variables[i] = env->variables[env->var_count - 1];
            env->var_count--;
            return;
        }
    }
}

char *var_env_get(struct var_env *env, const char *name) 
{
    for (size_t i = 0; i < env->var_count; i++) 
    {
        if (strcmp(env->variables[i]->name, name) == 0) 
        {
            return env->variables[i]->value;
        }
    }
    return NULL;
}

//=====================EXPANSION=========================

char *expand_special_var(const char *var_name, char **argv, 
                        int argc, int last_exit_status) 
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
        snprintf(buffer, sizeof(buffer), "%d", argc > 0);
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
        snprintf(buffer, sizeof(buffer), "%d", rand() % 32768);
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
        return oldpwd;;
    }
    
    if (strcmp(var_name, "IFS") == 0) 
    {
        char *ifs = getenv("IFS");
        return ifs;
    }
    
    return NULL;
}

char **expand_word(const char *word, struct var_env *env, 
                   char **argv, int argc, int last_exit_status) 
{
    if (!word)
        return NULL;
    
    if (!strchr(word, '$')) 
    {
        char **result = calloc(2, sizeof(char *));
        result[0] = my_strdup(word);
        result[1] = NULL;
        return result;
    }
    
    size_t capacity = 256;
    char *buffer = malloc(capacity);
    size_t pos = 0;
    
    char **results = NULL;
    size_t result_count = 0;
    
    for (size_t i = 0; word[i]; i++) 
    {
        if (word[i] == '$') 
        {
            char *var_name = NULL;
            size_t var_len = 0;
            
            if (word[i + 1] == '{') 
            {
                i += 2;
                size_t start = i;
                while (word[i] && word[i] != '}')
                    i++;
                var_len = i - start;
                var_name = my_strndup(&word[start], var_len);
            } 
            else 
            {
                i++;
                if (isdigit(word[i]) || strchr("@*?$#", word[i])) {
                    var_name = my_strndup(&word[i], 1);
                    var_len = 1;
                } else {
                    size_t start = i;
                    while (word[i] && (isalnum(word[i]) || word[i] == '_'))
                        i++;
                    var_len = i - start;
                    var_name = my_strndup(&word[start], var_len);
                    i--;
                }
            }
            
            char *value = expand_special_var(var_name, argv, argc, last_exit_status);
            if (!value)
                value = var_env_get(env, var_name);
            
            if (strcmp(var_name, "@") == 0 && argv) 
            {
                if (pos > 0) {
                    buffer[pos] = '\0';
                    results = realloc(results, sizeof(char *) * (result_count + 1));
                    results[result_count++] = my_strdup(buffer);
                    pos = 0;
                }
                
                for (int j = 1; j < argc; j++) 
                {
                    results = realloc(results, sizeof(char *) * (result_count + 1));
                    results[result_count++] = my_strdup(argv[j]);
                }
                
                free(var_name);
                continue;
            }
            
            if (value) 
            {
                size_t value_len = strlen(value);
                while (pos + value_len >= capacity) 
                {
                    capacity *= 2;
                    buffer = realloc(buffer, capacity);
                }
                strcpy(&buffer[pos], value);
                pos += value_len;
            }
            
            free(var_name);
        } else 
        {
            if (pos >= capacity - 1) 
            {
                capacity *= 2;
                buffer = realloc(buffer, capacity);
            }
            buffer[pos++] = word[i];
        }
    }
    
    buffer[pos] = '\0';
    
    if (result_count == 0) 
    {
        results = calloc(2, sizeof(char *));
        results[0] = buffer;
        results[1] = NULL;
    } else 
    {
        if (pos > 0) 
        {
            results = realloc(results, sizeof(char *) * (result_count + 2));
            results[result_count++] = buffer;
        } 
        else 
        {
            free(buffer);
        }
        results = realloc(results, sizeof(char *) * (result_count + 1));
        results[result_count] = NULL;
    }
    
    return results;
}
