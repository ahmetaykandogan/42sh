#include <criterion/criterion.h>
#include <criterion/redirect.h>
#include <stddef.h>
#include <stdio.h>

#include "../../../src/pretty_printer/pretty_printer.c"

TestSuite(TestPrettyPrinterHard);

Test(TestPrettyPrinterHard, ast_full_ast_cmd_test, .init = cr_redirect_stdout)
{
    struct ast_if *node = calloc(1, sizeof(struct ast_if));
    struct ast type_if;
    type_if.type = AST_IF;
    node->base = type_if;

    struct ast_list *node_list = calloc(1, sizeof(struct ast_list));
    struct ast type_list;
    type_list.type = AST_LIST;
    node_list->base = type_list;
    node_list->child_count = 2;
    node_list->children = calloc(2, sizeof(struct ast_cmd));

    struct ast_cmd *node_cmd1 = calloc(1, sizeof(struct ast_cmd));
    struct ast type_cmd1;
    type_cmd1.type = AST_CMD;
    node_cmd1->base = type_cmd1;
    char *words1_cmd1 = malloc(sizeof(char *));
    words1_cmd1 = "true";
    char *words2_cmd1 = malloc(sizeof(char *));
    words2_cmd1 = NULL;
    char *words_cmd1[2];
    words_cmd1[0] = words1_cmd1;
    words_cmd1[1] = words2_cmd1;
    node_cmd1->words = words_cmd1;

    node_list->children[0] = (struct ast *)node_cmd1;

    struct ast_cmd *node_cmd2 = calloc(1, sizeof(struct ast_cmd));
    struct ast type_cmd2;
    type_cmd2.type = AST_CMD;
    node_cmd2->base = type_cmd2;
    char *words1_cmd2 = malloc(sizeof(char *));
    words1_cmd2 = "false";
    char *words2_cmd2 = malloc(sizeof(char *));
    words2_cmd2 = NULL;
    char *words_cmd2[2];
    words_cmd2[0] = words1_cmd2;
    words_cmd2[1] = words2_cmd2;
    node_cmd2->words = words_cmd2;

    node_list->children[1] = (struct ast *)node_cmd2;

    struct ast_cmd *node_cmd3 = calloc(1, sizeof(struct ast_cmd));
    struct ast type_cmd3;
    type_cmd3.type = AST_CMD;
    node_cmd3->base = type_cmd3;
    char *words1_cmd3 = malloc(sizeof(char *));
    words1_cmd3 = "echo";
    char *words2_cmd3 = malloc(sizeof(char *));
    words2_cmd3 = "banane";
    char *words3_cmd3 = malloc(sizeof(char *));
    words3_cmd3 = NULL;
    char *words_cmd3[3];
    words_cmd3[0] = words1_cmd3;
    words_cmd3[1] = words2_cmd3;
    words_cmd3[2] = words3_cmd3;
    node_cmd3->words = words_cmd3;

    node->then_body = (struct ast *)node_cmd3;

    struct ast_cmd *node_cmd4 = calloc(1, sizeof(struct ast_cmd));
    struct ast type_cmd4;
    type_cmd4.type = AST_CMD;
    node_cmd4->base = type_cmd4;
    char *words1_cmd4 = malloc(sizeof(char *));
    words1_cmd4 = "echo";
    char *words2_cmd4 = malloc(sizeof(char *));
    words2_cmd4 = "abricot";
    char *words3_cmd4 = malloc(sizeof(char *));
    words3_cmd4 = NULL;
    char *words_cmd4[3];
    words_cmd4[0] = words1_cmd4;
    words_cmd4[1] = words2_cmd4;
    words_cmd4[2] = words3_cmd4;
    node_cmd4->words = words_cmd4;
    
    node->else_body = (struct ast *)node_cmd4;

    pretty_print_ast((struct ast *)node);
    fflush(stdout);
    cr_assert_stdout_eq_str("Condition: If \nLIST:\nCMD: true \nCMD: false \nBody: Then\nCMD: echo banane \nBody: Else\nCMD: echo abricot \n");
    pretty_print_ast((struct ast *)node);
}

Test(TestPrettyPrinterHard, ast_ast_if_test, .init = cr_redirect_stdout)
{
    struct ast_if *node = calloc(1, sizeof(struct ast_if));
    struct ast type;
    type.type = AST_IF;
    node->base = type;
    pretty_print_ast((struct ast *)node);
    fflush(stdout);
    cr_assert_stdout_eq_str("Condition: If \nBody: Then\n");
}

Test(TestPrettyPrinterHard, ast_ast_list_test, .init = cr_redirect_stdout)
{
    struct ast_list *node = calloc(1, sizeof(struct ast_list));
    struct ast type;
    type.type = AST_LIST;
    node->base = type;
    pretty_print_ast((struct ast *)node);
    fflush(stdout);
    cr_assert_stdout_eq_str("LIST:\n");
}

Test(TestPrettyPrinterHard, basic_ast_cmd_test, .init = cr_redirect_stdout)
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

Test(TestPrettyPrinterHard, basic_ast_if_test, .init = cr_redirect_stdout)
{
    struct ast_if *node = calloc(1, sizeof(struct ast_if));
    pretty_print_ast_if(node);
    fflush(stdout);
    cr_assert_stdout_eq_str("Condition: If \nBody: Then\n");
}

Test(TestPrettyPrinterHard, basic_ast_list_test, .init = cr_redirect_stdout)
{
    struct ast_list *node = calloc(1, sizeof(struct ast_list));
    pretty_print_ast_list(node);
    fflush(stdout);
    cr_assert_stdout_eq_str("LIST:\n");
}
