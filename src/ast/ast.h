#ifndef AST_H
#define AST_H

#include <assert.h>
#include <unistd.h>
#include <sys/types.h>
#include <string.h>

enum ast_type // All the pseudo types of AST nodes
{
    AST_CMD,
    AST_IF,
    AST_LIST,
    AST_WHILE,
    AST_UNTIL,
    AST_PIPE,
    AST_AND_OR
};

// To understand how this works, see the 42SH Design AST documentation
// But basically this allows that we can cast every struct ast to their specific
// type during execution We can for example do:
//  struct ast *node = ...;
//  if (node->type == AST_CMD) {
//      struct ast_cmd *cmd_node = (struct ast_cmd *)node;
struct ast
{
    enum ast_type type;
};

struct redirection
{
    char *fd;//if there is an IONUMBER (IF not NULL)
    //int flags;//O_RDONLY, O_WRONLY, etc may be needed for future steps?
    char *filename;//the file to redirect to/from
    char *operator;//the redirection operator (>, <, >>, etc)
};

//--------------------------------------THE TYPES OF AST
//NODES--------------------------------------//

struct ast_cmd
{
    struct ast base;
    int negated; // 1 if the command is negated with '!'
    char **words; // NULL terminated char* list */

    struct redirection **redirections; // NULL terminated redirection list 
    //if there are none, this is NULL
};

struct ast_if
{
    struct ast base;
    struct ast *condition; // the condition
    struct ast *then_body; // the body of the then clause
    struct ast *else_body; // the body of the else, may be NULL
    
    struct redirection **redirections;
    //NULL if none
};

struct ast_list
{
    struct ast base;
    struct ast **children;
    size_t child_count;
};

struct ast_while
{
    struct ast base;
    struct ast *condition;
    struct ast *body;

    struct redirection **redirections;
};

struct ast_until
{
    struct ast base;
    struct ast *condition;
    struct ast *body;

    struct redirection **redirections;
};

struct ast_pipe
{
    struct ast base;
    struct ast_list *pipelines;
    int negated;

    struct redirection **redirections; // Can a pipe have redirections?
};

struct ast_and_or // 
{
    struct ast base;
    struct ast_list *children;
    int *op; // 0 for &&, 1 for || 
    //the way this will function is op[i] is the operator between
    //children[i] and children[i+1]
    //for example, if we have a || b && c, then op[0] = 1, op[1] = 0
    //This implementation dont work for case with parantheses, there is no and or priority
    size_t child_count;
};

//-------------------------------------INITIALIZE AST TYPES-----------------//

struct ast_cmd *ast_cmd_init(int neg);
struct ast_if *ast_if_init();
struct ast_list *ast_list_init();
struct ast_while *ast_while_init();
struct ast_until *ast_until_init();
struct ast_pipe *ast_pipeline_init(int neg);
struct ast_and_or *ast_and_or_init();

//--------------------------------ADDINGS------------------------------------//

void ast_cmd_add_word(struct ast_cmd *in, char *word);
// TODO realloc words array inside ast_cmd toincreaze size by 1
void ast_list_add_ast(struct ast_list *in, struct ast *ast);
// TODO same for this
void ast_and_or_add_ast(struct ast_and_or *in, struct ast *ast, int type);
// type is 0 for &&, 1 for ||

//---------------------LOOKUP TABLE FOR DESTROY FUNCTIONS-----------------//

const char *ast_repr(enum ast_type type);

typedef void (*run_type)(struct ast *ast);
/* FIX: separate function pointer type for redirections */
typedef void (*redir_type)(struct ast *ast, struct redirection *redir);

void run_ast(struct ast *ast);
void run_pipe(struct ast *enter_pipe);
void run_command(struct ast_cmd *cmd);
void run_redirections(struct redirection **redir);

//----------------------------------DESTROY AST
//TYPES------------------------------//

void struct_ast_destroy(struct ast *node); // generic destroy function for all ast types

void ast_cmd_destroy(struct ast_cmd *node);
void ast_if_destroy(struct ast_if *node);
void ast_list_destroy(struct ast_list *node);
void ast_while_destroy(struct ast_while *node);
void ast_until_destroy(struct ast_until *node);
void ast_pipe_destroy(struct ast_pipe *node);
void ast_and_or_destroy(struct ast_and_or *node);

//--------------------------------------redirections-----------------------------


struct redirection *redirection_init(char *fd, char *filename, char *operator);
void redirection_destroy(struct redirection *redir);
void ast_add_redirection(struct ast *node, struct redirection *redir);


void ast_cmd_add_redirection(struct ast *ast_node, struct redirection *redir);
void ast_if_add_redirection(struct ast *ast_node, struct redirection *redir);
void ast_while_add_redirection(struct ast *ast_node, struct redirection *redir);
void ast_until_add_redirection(struct ast *ast_node, struct redirection *redir);

/* FIX: pointer level corrected */
void add_redirection(struct redirection ***ast_redir,
                     struct redirection *new_redir);

    
#endif /* ! AST_H */