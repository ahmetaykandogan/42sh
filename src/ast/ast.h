#ifndef AST_H
#define AST_H

#include <stdlib.h>

enum ast_type //All the pseudo types of AST nodes
{
    AST_CMD,
    AST_IF,
    AST_LIST,
};

//To understand how this works, see the 42SH Design AST documentation
//(I still dont understand how this works fully)
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
    int nb_word;
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
    // this may be only left and right, but I think it's not?
    size_t child_count;
};

//-------------------------------------INITIALIZE AST TYPES-----------------//


struct ast_cmd *ast_cmd_init();
struct ast_if *ast_if_init();
struct ast_list *ast_list_init();

//--------------------------------ADDINGS------------------------------------


void ast_cmd_add_word(struct ast_cmd *in, char *word);
void ast_list_add_ast(struct ast_list *in, stuct ast_cmd *ast);
#endif /* ! AST_H */
