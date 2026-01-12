#define _POSIX_C_SOURCE 200809L

#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../utils/malloc_err.c"

FILE *open_file(char *file)
{
    FILE *f = fopen(file, "r");
    if (!f)
    {
        err(2, "Could not open file");
    }
    return f;
}

char *read_file(FILE *f)
{
    char *line = NULL;

    size_t capacity = 8;
    char *buff = malloc(sizeof(char) * capacity);
    if (!buff)
    {
        fclose(f);
        err(2, "Could not malloc buff");
    }

    size_t n = 0;
    size_t buff_len = 0;

    ssize_t line_size;
    while ((line_size = getline(&line, &n, f)) != -1)
    {
        if (buff_len >= capacity)
        {
            capacity *= 2;
            char *tmp = realloc(buff, capacity * sizeof(char));
            if (!tmp)
            {
                fclose(f);
                err(2, "Could not realloc tmp");
            }
            buff = tmp;
        }

        memcpy(buff + buff_len, line, line_size);
        buff_len += line_size;
    }
    free(line);
    if (fclose(f) == -1)
    {
        err(2, "Could not close file");
    }

    buff = realloc_err(buff, buff_len + 1);
    buff[buff_len] = '\0';
    if (buff_len == 0)
    {
        errx(2, "Empty file");
    }
    return buff;
}
