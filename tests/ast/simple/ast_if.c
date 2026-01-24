#include <criterion/criterion.h>
#include <criterion/redirect.h>

#include "../../../src/ast/ast.h"

// TestSuite definition
TestSuite(TestIf);

Test(TestIf, init_basic)
{
    struct ast_if *if_node = ast_if_init();
    
    cr_assert_not_null(if_node);
    cr_assert_eq(if_node->base.type, AST_IF);
    cr_assert_null(if_node->condition);
    cr_assert_null(if_node->then_body);
    cr_assert_null(if_node->else_body);
    cr_assert_null(if_node->redirections);
    
    ast_if_destroy(if_node);
}

Test(TestIf, with_condition_and_then)
{
    struct ast_if *if_node = ast_if_init();
    struct ast_cmd *cond = ast_cmd_init(0);
    struct ast_cmd *then = ast_cmd_init(0);
    
    if_node->condition = (struct ast *)cond;
    if_node->then_body = (struct ast *)then;
    
    cr_assert_not_null(if_node->condition);
    cr_assert_not_null(if_node->then_body);
    cr_assert_null(if_node->else_body);
    
    ast_if_destroy(if_node);
}

Test(TestIf, with_else)
{
    struct ast_if *if_node = ast_if_init();
    struct ast_cmd *cond = ast_cmd_init(0);
    struct ast_cmd *then = ast_cmd_init(0);
    struct ast_cmd *else_cmd = ast_cmd_init(0);
    
    if_node->condition = (struct ast *)cond;
    if_node->then_body = (struct ast *)then;
    if_node->else_body = (struct ast *)else_cmd;
    
    cr_assert_not_null(if_node->else_body);
    
    ast_if_destroy(if_node);
}

Test(TestIf, destroy_null)
{
    ast_if_destroy(NULL);
}