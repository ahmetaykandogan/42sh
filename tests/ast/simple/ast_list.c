#include <criterion/criterion.h>
#include <criterion/redirect.h>

#include "../../../src/ast/ast.h"

// TestSuite definition
TestSuite(TestList);

Test(TestList, init_basic)
{
    struct ast_list *list = ast_list_init();
    
    cr_assert_not_null(list);
    cr_assert_eq(list->base.type, AST_LIST);
    cr_assert_null(list->children);
    cr_assert_eq(list->child_count, 0);
    cr_assert_null(list->redirections);
    
    ast_list_destroy(list);
}

Test(TestList, add_single_ast)
{
    struct ast_list *list = ast_list_init();
    struct ast_cmd *cmd = ast_cmd_init(0);
    
    ast_list_add_ast(list, (struct ast *)cmd);
    
    cr_assert_not_null(list->children);
    cr_assert_eq(list->child_count, 1);
    cr_assert_eq(list->children[0]->type, AST_CMD);
    
    ast_list_destroy(list);
}

Test(TestList, add_multiple_ast)
{
    struct ast_list *list = ast_list_init();
    struct ast_cmd *cmd1 = ast_cmd_init(0);
    struct ast_cmd *cmd2 = ast_cmd_init(0);
    struct ast_cmd *cmd3 = ast_cmd_init(0);
    
    ast_list_add_ast(list, (struct ast *)cmd1);
    ast_list_add_ast(list, (struct ast *)cmd2);
    ast_list_add_ast(list, (struct ast *)cmd3);
    
    cr_assert_eq(list->child_count, 3);
    cr_assert_not_null(list->children[0]);
    cr_assert_not_null(list->children[1]);
    cr_assert_not_null(list->children[2]);
    
    ast_list_destroy(list);
}

Test(TestList, destroy_null)
{
    ast_list_destroy(NULL);
}

Test(TestList, destroy_empty)
{
    struct ast_list *list = ast_list_init();
    ast_list_destroy(list);
}