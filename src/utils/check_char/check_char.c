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
