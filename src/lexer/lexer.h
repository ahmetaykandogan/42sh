#ifndef LEXER_H
#define LEXER_H

#include "../utils/fifo/fifo.h"

#include <stdio.h>

struct fifo *lexer(char *input);

struct fifo *lexing_from_file(char *path);
struct fifo *lexing_from_stdin(FILE *f);

#endif /* ! LEXER_H */
