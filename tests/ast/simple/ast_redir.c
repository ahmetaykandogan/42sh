#include <criterion/criterion.h>
#include <criterion/redirect.h>

#include "../../../src/ast/ast.h"

// TestSuite definition
TestSuite(TestRedir);

Test(TestRedir, init_basic)
{
    struct redirection *redir = redirection_init(NULL, strdup("file.txt"), strdup(">"));
    
    cr_assert_not_null(redir);
    cr_assert_null(redir->fd);
    cr_assert_str_eq(redir->filename, "file.txt");
    cr_assert_str_eq(redir->operator, ">");
    
    redirection_destroy(redir);
}

Test(TestRedir, init_with_fd)
{
    struct redirection *redir = redirection_init(strdup("2"), strdup("error.log"), strdup(">&"));
    
    cr_assert_str_eq(redir->fd, "2");
    cr_assert_str_eq(redir->filename, "error.log");
    cr_assert_str_eq(redir->operator, ">&");
    
    redirection_destroy(redir);
}

Test(TestRedir, destroy_null)
{
    redirection_destroy(NULL);
}

Test(TestRedir, add_to_cmd)
{
    struct ast_cmd *cmd = ast_cmd_init(0);
    struct redirection *redir = redirection_init(NULL, strdup("out.txt"), strdup(">"));
    
    ast_cmd_add_redirection((struct ast *)cmd, redir);
    
    cr_assert_not_null(cmd->redirections);
    cr_assert_not_null(cmd->redirections[0]);
    cr_assert_null(cmd->redirections[1], "Should be NULL terminated");
    
    ast_cmd_destroy(cmd);
}

Test(TestRedir, add_multiple_to_cmd)
{
    struct ast_cmd *cmd = ast_cmd_init(0);
    struct redirection *redir1 = redirection_init(NULL, strdup("in.txt"), strdup("<"));
    struct redirection *redir2 = redirection_init(NULL, strdup("out.txt"), strdup(">"));
    
    ast_cmd_add_redirection((struct ast *)cmd, redir1);
    ast_cmd_add_redirection((struct ast *)cmd, redir2);
    
    cr_assert_not_null(cmd->redirections[0]);
    cr_assert_not_null(cmd->redirections[1]);
    cr_assert_null(cmd->redirections[2]);
    
    ast_cmd_destroy(cmd);
}

Test(TestRedir, add_to_if)
{
    struct ast_if *if_node = ast_if_init();
    struct redirection *redir = redirection_init(NULL, strdup("file.txt"), strdup(">"));
    
    ast_if_add_redirection((struct ast *)if_node, redir);
    
    cr_assert_not_null(if_node->redirections);
    cr_assert_not_null(if_node->redirections[0]);
    cr_assert_null(if_node->redirections[1]);
    
    ast_if_destroy(if_node);
}