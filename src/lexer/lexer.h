#ifndef LEXER_H
#define LEXER_H

#include <stdio.h>

#include "../utils/fifo/fifo.h"

struct fifo *lexer(char *input);

struct fifo *lexing_from_file(char *path);
struct fifo *lexing_from_stdin(FILE *f);

#endif /* ! LEXER_H */
