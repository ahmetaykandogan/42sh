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

void fifo_destroy_failed(struct fifo *fifo)
{
    while (fifo->size != 0)
    {
        struct lextoken *token = fifo_head(fifo);
        if (token->tok == WORDS || token->tok == VAR || token->tok == IONUMBER
            || token->tok == RED_IN || token->tok == RED_OUT
            || token->tok == APPEND_OUT || token->tok == DUP_IN
            || token->tok == DUP_OUT || token->tok == RED_FORCE
            || token->tok == FD_RW)
        {
            free(token->value);
        }
        fifo_pop(fifo);
    }
    free(fifo);
}
