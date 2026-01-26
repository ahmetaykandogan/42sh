#include "builtins.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../ast/variable.h"

int builtins_true(void)
{
    return 0;
}

int builtins_false(void)
{
    return 1;
}

/* Print depending on interpret whether it is a -e or a -E */
static void print_word(const char *s, int interpret)
{
    int i = 0;

    while (s[i])
    {
        if (interpret && s[i] == '\\' && s[i + 1])
        {
            i++;
            if (s[i] == 'n')
                putchar('\n');
            else if (s[i] == 't')
                putchar('\t');
            else if (s[i] == '\\')
                putchar('\\');
            else
            {
                putchar('\\');
                if (s[i])
                    putchar(s[i]);
            }
        }
        else
            putchar(s[i]);
        if (s[i])
            i++;
    }
}

int builtins_echo(char **words)
{
    int i = 1;
    int newline = 1;
    int interpret = 0;

    /* Parse options */
    while (words[i] && words[i][0] == '-' && words[i][1])
    {
        if (strcmp(words[i], "-n") == 0)
            newline = 0;
        else if (strcmp(words[i], "-e") == 0)
            interpret = 1;
        else if (strcmp(words[i], "-E") == 0)
            interpret = 0;
        else if (strcmp(words[i], "-ne") == 0)
        {
            newline = 0;
            interpret = 1;
        }
        else if (strcmp(words[i], "-nE") == 0)
        {
            newline = 0;
            interpret = 0;
        }
        else if (strcmp(words[i], "-en") == 0)
        {
            newline = 0;
            interpret = 1;
        }
        else if (strcmp(words[i], "-En") == 0)
        {
            newline = 0;
            interpret = 0;
        }
        else
            break;
        i++;
    }

    /* Print arguments */
    while (words[i])
    {
        print_word(words[i], interpret);
        if (words[i + 1])
            putchar(' ');
        i++;
    }

    if (newline)
        putchar('\n');

    fflush(stdout);
    return 0;
}

int builtins_exit(char **argv)
{
    int exit_status = 0;
    if (argv[1])
    {
        exit_status = atoi(argv[1]);
    }
    return exit_status;
}

int builtins_cd(char **argv)
{
    const char *target = NULL;
    // cd
    if (!argv[1])
    {
        target = getenv("HOME");
    }
    // cd -
    else if (strcmp(argv[1], "-") == 0)
    {
        target = var_env_get("OLDPWD");
        if (!target)
        {
            return 1;
        }
        printf("%s\n", target);
    }
    // cd path
    else
    {
        target = argv[1];
    }
    char oldpwd[64];
    if (!getcwd(oldpwd, sizeof(oldpwd)))
        oldpwd[0] = '\0';

    if (chdir(target) != 0)
    {
        return 1;
    }

    if (oldpwd[0])
        var_env_set("OLDPWD", oldpwd);
    char newpwd[64];
    if (getcwd(newpwd, sizeof(newpwd)))
        var_env_set("PWD", newpwd);
    return 0;
}
