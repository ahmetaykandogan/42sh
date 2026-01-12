#include <err.h>
#include <stdlib.h>

void *malloc_err(size_t size)
{
    void *ptr = malloc(size);
    if (!ptr)
    {
        err(2, "Could not malloc");
    }
    return ptr;
}

void *realloc_err(void *ptr, size_t size)
{
    void *r = realloc(ptr, size);
    if (!r)
    {
        err(2, "Could not realloc");
    }
    return r;
}
