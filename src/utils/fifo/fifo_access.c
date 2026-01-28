#include <stdio.h>
#include <stdlib.h>

#include "fifo.h"

size_t fifo_size(struct fifo *fifo)
{
    return fifo->size;
}

void fifo_push(struct fifo *fifo, struct lextoken *elt)
{
    struct list *new_val = malloc(sizeof(struct fifo));
    if (new_val == NULL)
        return;
    new_val->data = elt;
    new_val->next = NULL;
    if (fifo->head == 0)
    {
        fifo->head = new_val;
        fifo->tail = new_val;
    }
    else
    {
        fifo->tail->next = new_val;
        fifo->tail = new_val;
    }
    fifo->size++;
}

struct lextoken *fifo_head(struct fifo *fifo)
{
    if (!fifo->head)
    {
        return NULL;
    }
    return fifo->head->data;
}

void fifo_pop(struct fifo *fifo)
{
    if (fifo->size == 0)
        return;
    struct list *tmp = fifo->head;
    fifo->head = fifo->head->next;
    if (fifo->head == NULL)
        fifo->tail = NULL;
    free(tmp->data);
    free(tmp);
    fifo->size--;
}


struct lextoken *fifo_access_i(const struct fifo *fifo, int i)
{
    if (fifo == NULL)
        return NULL;
    struct list *current = fifo->head;
    for (int j = 0; j < i; j++)
    {
        if (current == NULL)
            return NULL;
        current = current->next;
    }
    if (current == NULL)
        return NULL;
    return current->data;
}

