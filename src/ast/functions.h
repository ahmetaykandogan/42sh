#ifndef FUNCTIONS_H
#define FUNCTIONS_H


struct func 
{
    char *name;
    struct ast *body;
    //when name is called, run body
};

struct func_env 
{
    struct func **functions;
    size_t func_count;
    size_t func_capacity;
};

void func_env_set(const char *name, struct ast *body);
struct ast *func_env_get(const char *name);
void func_env_unset(const char *name);


#endif /* ! FUNCTIONS_H */
