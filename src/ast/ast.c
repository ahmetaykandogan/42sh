#include "ast.h"


struct ast_cmd *ast_cmd_init()
{
    struct ast_cmd *out = calloc(1, sizeof(struct ast_cmd));
    out->base.type = AST_CMD;
    out->words = calloc(1,sizeof(char *));
    return out;
}

struct ast_if *ast_if_init()
{
    struct ast_if *out = calloc(1, sizeof(struct ast_if));
    out->base.type = AST_IF;
    out->condition = NULL;
    out->then_body = NULL;
    out->else_body = NULL;
    return out;
}

struct ast_list *ast_list_init()
{
    struct ast_list *out = calloc(1, sizeof(struct ast_list));
    out->base.type = AST_LIST;
    out->children = calloc(1, sizeof(struct ast *));
    out->child_count = 0;
    return out;
}


//--------------------------------ADDINGS------------------------------------

void ast_cmd_add_word(struct ast_cmd *in, char *word)
{
    size_t count = 0;
    while (in->words[count] != NULL)
        count++;
    char **out = realloc(in->words, sizeof(char *) * (count + 2));
    out[count] = word;
    out[count + 1] = NULL;
    in->words = out;
};

void ast_list_add_ast(struct ast_list *in, struct ast *ast)
{
    size_t count = in->child_count;
    struct ast **out = realloc(in->children, sizeof(struct ast *) * (count + 1));
    out[count] = ast;
    in->children = out;
    in->child_count += 1;
};

//-----------------LOOKUP TABLE FOR DESTROY FUNCTIONS-----------------

const char *ast_repr(enum ast_type type)
{
    static const char *const names[] = {
        [AST_CMD] = "CMD",
        [AST_IF] = "IF",
        [AST_LIST] = "LIST",
    };
    return names[type];  
}

void run_ast(struct ast *ast)
{
    static run_type run_functions[] = {
        [AST_CMD] = &ast_run_cmd,
        [AST_IF] = &ast_run_if,
        [AST_LIST] = &ast_run_list,
    };
    (*run_functions[ast->type])(ast);
}

static void ast_run_cmd(struct ast *ast)
{
    assert(ast->type == AST_CMD);
    struct ast_cmd *ast_cmd = (struct ast_cmd *)ast;
    run_command(ast_cmd->words);
}

static void ast_run_if(struct ast *ast)
{
    assert(ast->type == AST_IF);
    struct ast_if *ast_if = (struct ast_if *)ast;
    run_ast(ast_if->condition);
    run_ast(ast_if->then_body);
    if (ast_if->else_body)
        run_ast(ast_if->else_body);
}

static void ast_run_list(struct ast *ast)
{
    assert(ast->type == AST_LIST);
    struct ast_list *ast_list = (struct ast_list *)ast;
    for (size_t i = 0; i < ast_list->child_count; i++)
    {
        run_ast(ast_list->children[i]);
    }
}

//----------------------------------DESTROY AST TYPES------------------------------
