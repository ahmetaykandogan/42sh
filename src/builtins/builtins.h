#ifndef BUILTINS_H
#define BUILTINS_H

// Don't forget to fflush(stdout) after running builtins.
int builtins_true(void);
int builtins_false(void);

// How do we call it ?
// For now with argc argv
int builtins_echo(int argc, char *argv[]);

#endif // BUILTINS_H
