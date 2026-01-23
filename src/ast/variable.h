#ifndef VARIABLE_H
#define VARIABLE_H

#include <stdlib.h>
#include <string.h>

#include "ast.h"

struct var
{
    char *name;
    char *value;
};

struct var_env
{
    struct var **variables;
    size_t var_count;
    size_t var_capacity;
};

//===========================INIT/DESTROY=============================
struct var_env *var_env_init();
void var_env_destroy(struct var_env *env);

//============================SET=GET/UNSET=============================
void var_env_set(const char *name, const char *value); // add a new variable
char *var_env_get(const char *name);
void var_env_unset(const char *name);

//===========================EXPANSION=============================
char *expand_variable_spec(const char *var_name, struct var_env *env,
                           char **argv, int argc, int last_exit);
char *expand_word(char *word, char **argv, int argc, int if_value);

#endif /* ! VARIABLE_H */
