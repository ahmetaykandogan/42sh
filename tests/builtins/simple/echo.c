#include <criterion/criterion.h>
#include <criterion/redirect.h>
#include <stdio.h>

#include "../../../src/builtins/builtins.h"

// TestSuite definition
TestSuite(TestEchoSimple);

Test(TestEchoSimple, basic_echo_test, .init = cr_redirect_stdout)
{
    char *words[3];
    char *word1 = "echo";
    char *word2 = "Hello World!";
    char *word3 = NULL;

    words[0] = word1;
    words[1] = word2;
    words[2] = word3;

    builtins_echo(words);
    fflush(stdout);
    cr_assert_stdout_eq_str("Hello World!\n");
}

Test(TestEchoSimple, basic_n_option_echo_test, .init = cr_redirect_stdout)
{
    char *words[4];
    char *word1 = "echo";
    char *option = "-n";
    char *word2 = "Hello World!";
    char *word3 = NULL;

    words[0] = word1;
    words[1] = option;
    words[2] = word2;
    words[3] = word3;

    builtins_echo(words);
    fflush(stdout);
    cr_assert_stdout_eq_str("Hello World!");
}

Test(TestEchoSimple, n_option_inside_echo_test, .init = cr_redirect_stdout)
{
    char *words[4];
    char *word1 = "echo";
    char *option = "-n";
    char *word2 = "Hello \n World!";
    char *word3 = NULL;

    words[0] = word1;
    words[1] = option;
    words[2] = word2;
    words[3] = word3;

    builtins_echo(words);
    fflush(stdout);
    cr_assert_stdout_eq_str("Hello \n World!");
}

Test(TestEchoSimple, n_option_after_echo_test, .init = cr_redirect_stdout)
{
    char *words[5];
    char *word1 = "echo";
    char *option = "-n";
    char *word2 = "Hello World!";
    char *word3 = "\n";
    char *word4 = NULL;

    words[0] = word1;
    words[1] = option;
    words[2] = word2;
    words[3] = word3;
    words[4] = word4;

    builtins_echo(words);
    fflush(stdout);
    cr_assert_stdout_eq_str("Hello World!\n");
}

Test(TestEchoSimple, nothing_e_option_echo_test, .init = cr_redirect_stdout)
{
    char *words[4];
    char *word1 = "echo";
    char *option = "-e";
    char *word2 = "Hello World!";
    char *word3 = NULL;

    words[0] = word1;
    words[1] = option;
    words[2] = word2;
    words[3] = word3;

    builtins_echo(words);
    fflush(stdout);
    cr_assert_stdout_eq_str("Hello World!\n");
}

Test(TestEchoSimple, e_option_inside_echo_test, .init = cr_redirect_stdout)
{
    char *words[4];
    char *word1 = "echo";
    char *option = "-n";
    char *word2 = "Hello \n World!";
    char *word3 = NULL;

    words[0] = word1;
    words[1] = option;
    words[2] = word2;
    words[3] = word3;

    builtins_echo(words);
    fflush(stdout);
    cr_assert_stdout_eq_str("Hello \n World!\n");
}

Test(TestEchoSimple, e_option_outside_echo_test, .init = cr_redirect_stdout)
{
    char *words[5];
    char *word1 = "echo";
    char *option = "-n";
    char *word2 = "Hello World!";
    char *word3 = "\n";
    char *word4 = NULL;

    words[0] = word1;
    words[1] = option;
    words[2] = word2;
    words[3] = word3;
    words[4] = word4;

    builtins_echo(words);
    fflush(stdout);
    cr_assert_stdout_eq_str("Hello World!\n\n");
}

Test(TestEchoSimple, e_option_tab_outside_echo_test, .init = cr_redirect_stdout)
{
    char *words[5];
    char *word1 = "echo";
    char *option = "-n";
    char *word2 = "Hello World!";
    char *word3 = "\t";
    char *word4 = NULL;

    words[0] = word1;
    words[1] = option;
    words[2] = word2;
    words[3] = word3;
    words[4] = word4;

    builtins_echo(words);
    fflush(stdout);
    cr_assert_stdout_eq_str("Hello World!\t\n");
}

Test(TestEchoSimple, e_option_escape_outside_echo_test, .init = cr_redirect_stdout)
{
    char *words[5];
    char *word1 = "echo";
    char *option = "-n";
    char *word2 = "Hello World!";
    char *word3 = "\\";
    char *word4 = NULL;

    words[0] = word1;
    words[1] = option;
    words[2] = word2;
    words[3] = word3;
    words[4] = word4;

    builtins_echo(words);
    fflush(stdout);
    cr_assert_stdout_eq_str("Hello World!\\\n");
}

Test(TestEchoSimple, nothing_E_option_echo_test, .init = cr_redirect_stdout)
{
    char *words[4];
    char *word1 = "echo";
    char *option = "-E";
    char *word2 = "Hello World!";
    char *word3 = NULL;

    words[0] = word1;
    words[1] = option;
    words[2] = word2;
    words[3] = word3;

    builtins_echo(words);
    fflush(stdout);
    cr_assert_stdout_eq_str("Hello World!\n");
}

Test(TestEchoSimple, E_option_inside_echo_test, .init = cr_redirect_stdout)
{
    char *words[4];
    char *word1 = "echo";
    char *option = "-n";
    char *word2 = "Hello \n World!";
    char *word3 = NULL;

    words[0] = word1;
    words[1] = option;
    words[2] = word2;
    words[3] = word3;

    builtins_echo(words);
    fflush(stdout);
    cr_assert_stdout_eq_str("Hello \\n World!\n");
}

Test(TestEchoSimple, E_option_outside_echo_test, .init = cr_redirect_stdout)
{
    char *words[5];
    char *word1 = "echo";
    char *option = "-n";
    char *word2 = "Hello World!";
    char *word3 = "\n";
    char *word4 = NULL;

    words[0] = word1;
    words[1] = option;
    words[2] = word2;
    words[3] = word3;
    words[4] = word4;

    builtins_echo(words);
    fflush(stdout);
    cr_assert_stdout_eq_str("Hello World!\\n\n");
}

Test(TestEchoSimple, E_option_tab_outside_echo_test, .init = cr_redirect_stdout)
{
    char *words[5];
    char *word1 = "echo";
    char *option = "-n";
    char *word2 = "Hello World!";
    char *word3 = "\t";
    char *word4 = NULL;

    words[0] = word1;
    words[1] = option;
    words[2] = word2;
    words[3] = word3;
    words[4] = word4;

    builtins_echo(words);
    fflush(stdout);
    cr_assert_stdout_eq_str("Hello World!\\t\n");
}

Test(TestEchoSimple, E_option_escape_outside_echo_test, .init = cr_redirect_stdout)
{
    char *words[5];
    char *word1 = "echo";
    char *option = "-n";
    char *word2 = "Hello World!";
    char *word3 = "\\";
    char *word4 = NULL;

    words[0] = word1;
    words[1] = option;
    words[2] = word2;
    words[3] = word3;
    words[4] = word4;

    builtins_echo(words);
    fflush(stdout);
    cr_assert_stdout_eq_str("Hello World!\\\\\n");
}
