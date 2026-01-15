#include <criterion/criterion.h>
#include <criterion/redirect.h>
#include <stddef.h>
#include <stdio.h>

#include "../../../src/pretty_printer/pretty_printer.c"

TestSuite(TestPrettyPrinterError);

Test(TestPrettyPrinterError, null_ast_test, .init = cr_redirect_stdout)
{
    struct ast *node = NULL;
    pretty_print_ast(node);
    fflush(stdout);
    cr_assert_stdout_eq_str("");
}

Test(TestPrettyPrinterError, null_ast_ast_cmd_test, .init = cr_redirect_stdout)
{
    struct ast_cmd *node = NULL;
    pretty_print_ast((struct ast *)node);
    fflush(stdout);
    cr_assert_stdout_eq_str("");
}

Test(TestPrettyPrinterError, null_ast_ast_if_test, .init = cr_redirect_stdout)
{
    struct ast_if *node = NULL;
    pretty_print_ast((struct ast *)node);
    fflush(stdout);
    cr_assert_stdout_eq_str("");
}

Test(TestPrettyPrinterError, null_ast_ast_list_test, .init = cr_redirect_stdout)
{
    struct ast_list *node = NULL;
    pretty_print_ast((struct ast *)node);
    fflush(stdout);
    cr_assert_stdout_eq_str("");
}

Test(TestPrettyPrinterError, null_ast_cmd_test, .init = cr_redirect_stdout)
{
    struct ast_cmd *node = NULL;
    pretty_print_ast_cmd(node);
    fflush(stdout);
    cr_assert_stdout_eq_str("");
}

Test(TestPrettyPrinterError, null_ast_if_test, .init = cr_redirect_stdout)
{
    struct ast_if *node = NULL;
    pretty_print_ast_if(node);
    fflush(stdout);
    cr_assert_stdout_eq_str("");
}

Test(TestPrettyPrinterError, null_ast_list_test, .init = cr_redirect_stdout)
{
    struct ast_list *node = NULL;
    pretty_print_ast_list(node);
    fflush(stdout);
    cr_assert_stdout_eq_str("");
}