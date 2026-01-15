#include <ctype.h>
#include <stddef.h>
#include <string.h>

// Should add the following characters for the next steps: | & < >
int check_char(char c)
{
    if (c == ';' || c == '\n' || c == '\\' || c == '\0' || c == '!' || c == '&'
        || c == '|' || c == '<' || c == '>' || isspace(c))
        return 0;
    if (c == '\'')
        return 1;
    if (c == '\"')
        return 2;
    return 42;
}

int is_ionumber(char *token)
{
    if (!token || !token[0])
    {
        return 0;
    }

    for (size_t i = 0; i < strlen(token); i++)
    {
        if (!isdigit(token[i]))
        {
            return 0;
        }
    }

    return 1;
}
