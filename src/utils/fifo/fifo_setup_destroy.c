#include <stdlib.h>

#include "fifo.h"

struct fifo *fifo_init(void)
{
    struct fifo *queue;
    queue = malloc(sizeof(struct fifo));
    if (queue == NULL)
    {
        return NULL;
    }
    queue->head = NULL;
    queue->tail = NULL;
    queue->size = 0;
    return queue;
}

void fifo_clear(struct fifo *fifo)
{
    while (fifo->size != 0)
    {
        fifo_pop(fifo);
    }
}

void fifo_destroy(struct fifo *fifo)
{
    fifo_clear(fifo);
    free(fifo);
}
