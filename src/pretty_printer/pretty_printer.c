#include "pretty_printer.h"

void pretty_print_ast(struct ast *node)
{
    if (!node)
        return;
    switch (node->type)
    {
    case AST_CMD: {
        pretty_print_ast_cmd((struct ast_cmd *)node);
        break;
    }
    case AST_IF: {
        pretty_print_ast_if((struct ast_if *)node);
        break;
    }
    case AST_LIST: {
        pretty_print_ast_list((struct ast_list *)node);
        break;
    }
    default:
        printf("Unknown AST node type\n");
        break;
    }
}

void pretty_print_ast_cmd(struct ast_cmd *node)
{
    if (!node)
        return;
    printf("CMD: ");
    for (size_t i = 0; node->words[i] != NULL; i++)
    {
        printf("%s ", node->words[i]);
    }
    printf("\n");
}

void pretty_print_ast_if(struct ast_if *node)
{
    if (!node)
        return;
    printf("Condition: If \n");
    pretty_print_ast(node->condition);
    printf("Body: Then\n");
    pretty_print_ast(node->then_body);
    if (node->else_body)
    {
        printf("Body: Else\n");
        pretty_print_ast(node->else_body);
    }
}

void pretty_print_ast_list(struct ast_list *node)
{
    if (!node)
        return;
    printf("LIST:\n");
    for (size_t i = 0; i < node->child_count; i++)
    {
        pretty_print_ast(node->children[i]);
    }
}
