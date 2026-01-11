#ifndef AST_H
#define AST_H

#include <assert.h>
#include <stdlib.h>

enum ast_type //All the pseudo types of AST nodes
{
    AST_CMD,
    AST_IF,
    AST_LIST,
};

//To understand how this works, see the 42SH Design AST documentation
//But basically this allows that we can cast every struct ast to their specific type during execution
//We can for example do:
// struct ast *node = ...;
// if (node->type == AST_CMD) {
//     struct ast_cmd *cmd_node = (struct ast_cmd *)node;
struct ast
{
    enum ast_type type;
};


//--------------------------------------THE TYPES OF AST NODES--------------------------------------//

struct ast_cmd
{
    struct ast base;
    char **words; // NULL terminated char* list */
};
struct ast_if
{
    struct ast base;
    struct ast *condition; // the condition
    struct ast *then_body; // the body of the then clause
    struct ast *else_body; // the body of the else, may be NULL
};

struct ast_list
{
    struct ast base;
    struct ast **children;
    size_t child_count;
};

//-------------------------------------INITIALIZE AST TYPES-----------------//


struct ast_cmd *ast_cmd_init();
struct ast_if *ast_if_init();
struct ast_list *ast_list_init();

//--------------------------------ADDINGS------------------------------------


void ast_cmd_add_word(struct ast_cmd *in, char *word);
//TODO realloc words array inside ast_cmd toincreaze size by 1
void ast_list_add_ast(struct ast_list *in, struct ast *ast);
//TODO same for this

//---------------------LOOKUP TABLE FOR DESTROY FUNCTIONS-----------------

const char *ast_repr(enum ast_type type);
typedef void (*run_type)(struct ast *ast);
void run_ast(struct ast *ast);
void run_command(char **words);
//----------------------------------DESTROY AST TYPES------------------------------

void struct_ast_destroy(struct ast *node); // generic destroy function for all ast types
//either a lookup table or a switch case inside to call the specific destroy function

void ast_cmd_destroy(struct ast_cmd *node);
void ast_if_destroy(struct ast_if *node);
void ast_list_destroy(struct ast_list *node);


//TODO PRETTY PRINT FUNCTIONS FOR DEBUGGING


#endif /* ! AST_H */
