#include "builtins.h"

#include <stdio.h>
#include <string.h>

int builtins_true(void)
{
    return 0;
}

int builtins_false(void)
{
    return 1;
}

// TODO verif arg, then do the two other option depending on it
int builtins_echo(int argc, char *argv[])
{
    int i = 1;
    int newline = 1;

    // -n option
    if (i < argc && strcmp(argv[i], "-n") == 0) {
        newline = 0;
        i++;
    }
    
    // -e enable the interpretation of \n, \t and \\ escapes.
    // -E disable the interpretation of \n, \t and \\ escapes.

    /* Print arguments */
    for (; i < argc; i++) {
        printf("%s", argv[i]);
        if (i + 1 < argc)
            printf(" ");
    }

    if (newline)
        printf("\n");

    return 0;
}