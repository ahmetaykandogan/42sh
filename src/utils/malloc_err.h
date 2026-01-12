#ifndef MALLOC_ERR_H
#define MALLOC_ERR_H

#include <stddef.h>

void *malloc_err(size_t size);

void *realloc_err(void *m, size_t size);

#endif /* ! MALLOC_ERR_H */
