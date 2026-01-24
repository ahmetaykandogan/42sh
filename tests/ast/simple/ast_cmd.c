#include <criterion/criterion.h>
#include <criterion/redirect.h>

#include "../../../src/ast/ast.h"

// TestSuite definition
TestSuite(TestCmd);

Test(TestCmd, init_basic)
{
    struct ast_cmd *cmd = ast_cmd_init(0);
    
    cr_assert_not_null(cmd, "ast_cmd_init should not be NULL");
    cr_assert_eq(cmd->base.type, AST_CMD, "Type should be AST_CMD");
    cr_assert_eq(cmd->negated, 0, "Negated should be 0");
    cr_assert_null(cmd->words, "Words should be NULL initially");
    cr_assert_null(cmd->assignments, "Assignments should be NULL initially");
    cr_assert_null(cmd->redirections, "Redirections should be NULL initially");
    
    ast_cmd_destroy(cmd);
}

Test(TestCmd, init_negated)
{
    struct ast_cmd *cmd = ast_cmd_init(1);
    
    cr_assert_not_null(cmd);
    cr_assert_eq(cmd->negated, 1, "Negated should be 1");
    
    ast_cmd_destroy(cmd);
}

Test(TestCmd, add_single_word)
{
    struct ast_cmd *cmd = ast_cmd_init(0);
    
    ast_cmd_add_word(cmd, strdup("echo"));
    
    cr_assert_not_null(cmd->words, "Words should not be NULL after adding");
    cr_assert_str_eq(cmd->words[0], "echo", "First word should be 'echo'");
    cr_assert_null(cmd->words[1], "Second element should be NULL (terminator)");
    
    ast_cmd_destroy(cmd);
}

Test(TestCmd, add_multiple_words)
{
    struct ast_cmd *cmd = ast_cmd_init(0);
    
    ast_cmd_add_word(cmd, strdup("echo"));
    ast_cmd_add_word(cmd, strdup("hello"));
    ast_cmd_add_word(cmd, strdup("world"));
    
    cr_assert_str_eq(cmd->words[0], "echo");
    cr_assert_str_eq(cmd->words[1], "hello");
    cr_assert_str_eq(cmd->words[2], "world");
    cr_assert_null(cmd->words[3], "Should be NULL terminated");
    
    ast_cmd_destroy(cmd);
}

Test(TestCmd, add_variable)
{
    struct ast_cmd *cmd = ast_cmd_init(0);
    
    add_vars(cmd, strdup("VAR=value"));
    
    cr_assert_not_null(cmd->assignments);
    cr_assert_str_eq(cmd->assignments[0], "VAR=value");
    cr_assert_null(cmd->assignments[1], "Should be NULL terminated");
    
    ast_cmd_destroy(cmd);
}

Test(TestCmd, destroy_null)
{
    ast_cmd_destroy(NULL);
}

Test(TestCmd, destroy_empty)
{
    struct ast_cmd *cmd = ast_cmd_init(0);
    ast_cmd_destroy(cmd);
}