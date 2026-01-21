#include "ast.h"
#include "variable.h"
#include <limits.h>
#include <linux/limits.h>

//Le fichier est à clang-tidy

struct var_env *global_var_env = NULL;
char **global_argv = NULL;
int global_argc = 0;

char **expand_word(const char *word, struct var_env *env, 
                   char **argv, int argc, int last_exit_status) 
{
    if (!word)
        return NULL;
    
    if (!strchr(word, '$')) 
    {
        char **result = calloc(2, sizeof(char *));
        result[0] = strdup(word);
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
                var_name = strndup(&word[start], var_len);
            } 
            else 
            {
                i++;
                if (isdigit(word[i]) || strchr("@*?$#", word[i])) {
                    var_name = strndup(&word[i], 1);
                    var_len = 1;
                } else {
                    size_t start = i;
                    while (word[i] && (isalnum(word[i]) || word[i] == '_'))
                        i++;
                    var_len = i - start;
                    var_name = strndup(&word[start], var_len);
                    i--;
                }
            }
            
            char *value = expand_special_var(var_name, argv, argc, last_exit_status);
            if (!value)
                value = var_get(env, var_name);
            
            if (strcmp(var_name, "@") == 0 && argv) 
            {
                if (pos > 0) {
                    buffer[pos] = '\0';
                    results = realloc(results, sizeof(char *) * (result_count + 1));
                    results[result_count++] = strdup(buffer);
                    pos = 0;
                }
                
                for (int j = 1; j < argc; j++) 
                {
                    results = realloc(results, sizeof(char *) * (result_count + 1));
                    results[result_count++] = strdup(argv[j]);
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
        return strdup(buffer);
    }
    
    // $$
    if (strcmp(var_name, "$") == 0) 
    {
        snprintf(buffer, sizeof(buffer), "%d", getpid());
        return strdup(buffer);
    }
    
    // $#
    if (strcmp(var_name, "#") == 0) 
    {
        snprintf(buffer, sizeof(buffer), "%d", argc > 0 ? argc - 1 : 0);
        return strdup(buffer);
    }
    
    // $0, $1, ..., $n
    if (isdigit(var_name[0]) && var_name[1] == '\0') 
    {
        int idx = var_name[0] - '0';
        if (idx < argc)
            return strdup(argv[idx]);
        return strdup("");
    }
    
    // $* - tous les args séparés par des espaces
    if (strcmp(var_name, "*") == 0) 
    {
        if (argc <= 1)
            return strdup("");
        
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
        return strdup(buffer);
    }
    
    // $UID
    if (strcmp(var_name, "UID") == 0) 
    {
        snprintf(buffer, sizeof(buffer), "%d", getuid());
        return strdup(buffer);
    }
    
    // Variables d'environnement
    if (strcmp(var_name, "PWD") == 0) 
    {
        char cwd[PATH_MAX];
        if (getcwd(cwd, sizeof(cwd)))
            return strdup(cwd);
        return strdup("");
    }
    
    if (strcmp(var_name, "OLDPWD") == 0) 
    {
        char *oldpwd = getenv("OLDPWD");
        return oldpwd ? strdup(oldpwd) : strdup("");
    }
    
    if (strcmp(var_name, "IFS") == 0) 
    {
        char *ifs = getenv("IFS");
        return ifs ? strdup(ifs) : strdup(" \t\n");
    }
    
    return NULL;
}
