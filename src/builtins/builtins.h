#ifndef BUILTINS_H
#define BUILTINS_H

// Don't forget to fflush(stdout) after running builtins.
int builtins_true(void);
int builtins_false(void);

int builtins_echo(char** words);

int builtins_cd(char** argv);

int builtins_export(int argc, char** argv);

#endif // BUILTINS_H
