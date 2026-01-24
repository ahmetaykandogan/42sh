#include <criterion/criterion.h>
#include <criterion/redirect.h>

#include <stdlib.h>
#include <string.h>

#include "../../../src/ast/ast.h"

static char *my_strdup(const char *s)
{
    if (!s)
        return NULL;
    size_t len = strlen(s);
    char *dup = malloc(len + 1);
    if (dup)
        strcpy(dup, s);
    return dup;
}

// TestSuite definition
TestSuite(TestRedir);

Test(TestRedir, init_basic)
{
    struct redirection *redir = redirection_init(NULL, my_strdup("file.txt"), my_strdup(">"));
    
    cr_assert_not_null(redir);
    cr_assert_null(redir->fd);
    cr_assert_str_eq(redir->filename, "file.txt");
    cr_assert_str_eq(redir->operator, ">");
    
    redirection_destroy(redir);
}

Test(TestRedir, init_with_fd)
{
    struct redirection *redir = redirection_init(my_strdup("2"), my_strdup("error.log"), my_strdup(">&"));
    
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
    struct redirection *redir = redirection_init(NULL, my_strdup("out.txt"), my_strdup(">"));
    
    ast_cmd_add_redirection((struct ast *)cmd, redir);
    
    cr_assert_not_null(cmd->redirections);
    cr_assert_not_null(cmd->redirections[0]);
    cr_assert_null(cmd->redirections[1], "Should be NULL terminated");
    
    ast_cmd_destroy(cmd);
}

Test(TestRedir, add_multiple_to_cmd)
{
    struct ast_cmd *cmd = ast_cmd_init(0);
    struct redirection *redir1 = redirection_init(NULL, my_strdup("in.txt"), my_strdup("<"));
    struct redirection *redir2 = redirection_init(NULL, my_strdup("out.txt"), my_strdup(">"));
    
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
    struct redirection *redir = redirection_init(NULL, my_strdup("file.txt"), my_strdup(">"));
    
    ast_if_add_redirection((struct ast *)if_node, redir);
    
    cr_assert_not_null(if_node->redirections);
    cr_assert_not_null(if_node->redirections[0]);
    cr_assert_null(if_node->redirections[1]);
    
    ast_if_destroy(if_node);
}