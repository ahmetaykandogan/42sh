#ifndef READ_FILE_H
#define READ_FILE_H

#include <stdio.h>

FILE *open_file(char *file);

char *read_file(FILE *file);

#endif /* ! READ_FILE_H */
