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

struct var_env *var_env_init();
void var_env_destroy(struct var_env *env);

#endif /* ! VARIABLE_H */
