#include "builtins.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <limits.h>

int builtins_true(void)
{
    return 0;
}

int builtins_false(void)
{
    return 1;
}

/* ----------------------------- */
/* -- ECHO BUILTINS FUNCTIONS -- */
/* ----------------------------- */

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

/* ----------------------------- */
/* --  CD  BUILTINS FUNCTIONS -- */
/* ----------------------------- */

static char *get_pwd(void)
{
    char *pwd = getenv("PWD");
    if (pwd && pwd[0])
    {
        return strdup(pwd);
    }
    
    char *cwd = getcwd(NULL, 0);
    if (!cwd)
    {
        perror("getcwd");
        return NULL;
    }
    
    return cwd;
}

static int update_pwd(const char *pwd, const char *new)
{
    if (pwd)
    {
        if (setenv("OLDPWD", pwd, 1) != 0)
        {
            err(-1, "cd : setenv OLDPWD");
        }
    }
    
    if (new)
    {
        if (setenv("PWD", new, 1) != 0)
        {
            err(-1, "cd : setenv PWD");
        }
    }
    
    return 0;
}

int builtins_cd(char **words)
{
    const char *target_dir = NULL;
    char *old_pwd = NULL;
    char *new_pwd = NULL;
    int ret = 0;

    old_pwd = get_pwd();
    if (!old_pwd)
    {
        err(1, "cd : failed to get pwd");
    }

    if (!words[1] || words[1][0] == '\0')
    {
        target_dir = getenv("HOME");
        if (!target_dir || target_dir[0] == '\0')
        {
            free(old_pwd);
            err(1, "cd : HOME not set");
        }
    }
    else if (strcmp(words[1], "-") == 0)
    {
        target_dir = getenv("OLDPWD");
        if (!target_dir || target_dir[0] == '\0')
        {
            free(old_pwd);
            err(1, "cd : OLDPWD not set");
        }
        
        printf("%s\n", target_dir);
        fflush(stdout);
    }
    else
    {
        target_dir = words[1];
    }
    
    if (chdir(target_dir) != 0)
    {
        free(old_pwd);
        err(1, "cd : failed");
    }
    
    if (target_dir[0] == '/')
    {
        new_pwd = strdup(target_dir);
    }
    else if (strcmp(words[1], "-") == 0)
    {
        new_pwd = strdup(target_dir);
    }
    else
    {
        size_t old_len = strlen(old_pwd);
        size_t target_len = strlen(target_dir);
        
        int needs_slash = (old_len > 0 && old_pwd[old_len - 1] != '/');
        
        new_pwd = malloc(old_len + needs_slash + target_len + 1);
        if (!new_pwd)
        {
            free(old_pwd);
            err(1, "cd : malloc failed");
        }
        
        strcpy(new_pwd, old_pwd);
        if (needs_slash)
        {
            strcat(new_pwd, "/");
        }
        strcat(new_pwd, target_dir);
    }
    
    if (!new_pwd)
    {
        free(old_pwd);
        err(1, "cd : failed to allocate new PWD");
    }
    
    ret = update_pwd_vars(old_pwd, new_pwd);
    
    free(old_pwd);
    free(new_pwd);
    
    return ret;
}
