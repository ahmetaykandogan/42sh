#include <criterion/criterion.h>
#include <criterion/redirect.h>
#include <stddef.h>
#include <stdio.h>

#include "../../../src/pretty_printer/pretty_printer.c"

TestSuite(TestPrettyPrinterSimple);

Test(TestPrettyPrinterSimple, ast_ast_cmd_test, .init = cr_redirect_stdout)
{
    char *words1 = malloc(sizeof(char *));
    words1 = "echo";
    char *words2 = malloc(sizeof(char *));
    words2 = "banane";
    char *words3 = malloc(sizeof(char *));
    words3 = NULL;
    char *words[3];
    words[0] = words1;
    words[1] = words2;
    words[2] = words3;
    struct ast type;
    type.type = AST_CMD;
    struct ast_cmd *node = calloc(1, sizeof(struct ast_cmd));
    node->base = type;
    node->words = words;
    pretty_print_ast((struct ast *)node);
    fflush(stdout);
    cr_assert_stdout_eq_str("CMD: echo banane \n");
}

Test(TestPrettyPrinterSimple, ast_ast_if_test, .init = cr_redirect_stdout)
{
    struct ast_if *node = calloc(1, sizeof(struct ast_if));
    struct ast type;
    type.type = AST_IF;
    node->base = type;
    pretty_print_ast((struct ast *)node);
    fflush(stdout);
    cr_assert_stdout_eq_str("Condition: If \nBody: Then\n");
}

Test(TestPrettyPrinterSimple, ast_ast_list_test, .init = cr_redirect_stdout)
{
    struct ast_list *node = calloc(1, sizeof(struct ast_list));
    struct ast type;
    type.type = AST_LIST;
    node->base = type;
    pretty_print_ast((struct ast *)node);
    fflush(stdout);
    cr_assert_stdout_eq_str("LIST:\n");
}


Test(TestPrettyPrinterSimple, basic_ast_cmd_test, .init = cr_redirect_stdout)
{
    char *words1 = malloc(sizeof(char *));
    words1 = "echo";
    char *words2 = malloc(sizeof(char *));
    words2 = "banane";
    char *words3 = malloc(sizeof(char *));
    words3 = NULL;
    char *words[3];
    words[0] = words1;
    words[1] = words2;
    words[2] = words3;
    struct ast_cmd *node = calloc(1, sizeof(struct ast_cmd));
    node->words = words;

    pretty_print_ast_cmd(node);
    fflush(stdout);
    cr_assert_stdout_eq_str("CMD: echo banane \n");
}

Test(TestPrettyPrinterSimple, basic_ast_if_test, .init = cr_redirect_stdout)
{
    struct ast_if *node = calloc(1, sizeof(struct ast_if));
    pretty_print_ast_if(node);
    fflush(stdout);
    cr_assert_stdout_eq_str("Condition: If \nBody: Then\n");
}

Test(TestPrettyPrinterSimple, basic_ast_list_test, .init = cr_redirect_stdout)
{
    struct ast_list *node = calloc(1, sizeof(struct ast_list));
    pretty_print_ast_list(node);
    fflush(stdout);
    cr_assert_stdout_eq_str("LIST:\n");
}
