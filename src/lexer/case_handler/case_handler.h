#ifndef CASE_HANDLER_H
#define CASE_HANDLER_H

#include <stddef.h>

#include "utils/fifo/fifo.h"

size_t handle_whitespace(char *input, size_t i);

size_t handle_comment(char *input, size_t i, struct fifo *queue);

size_t handle_number(char *input, size_t i, size_t input_len, struct fifo *queue);

size_t handle_word(char *input, size_t i, struct fifo *queue);

size_t handle_single_quote(char *input, size_t i, struct fifo *queue);

size_t handle_double_quote(char *input, size_t i, struct fifo *queue);

size_t handle_redirection(char *input, size_t i, struct fifo *queue);

size_t handle_operator(char *input, size_t i, struct fifo *queue);

size_t handle_variable(char *input, size_t i, size_t input_len, struct fifo *queue);

size_t handle_single_char(char *input, size_t i, struct fifo *queue);

#endif /* ! CASE_HANDLER_H */
