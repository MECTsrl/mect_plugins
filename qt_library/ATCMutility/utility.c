#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include "common.h"
#include "app_logprint.h"
#include "defines.h"

/**
 * @brief transform a string in hex format into int
 */
int atoh(const char * string)
{
    int value = 0;
    sscanf (string, "%x", &value);
    return value;
}

char *strtok_csv(char *string, const char *separators, char **savedptr)
{
    char *p, *s;

    if (separators == NULL || savedptr == NULL) {
        return NULL;
    }
    if (string == NULL) {
        p = *savedptr;
        if (p == NULL) {
            return NULL;
        }
    } else {
        p = string;
    }

    s = strstr(p, separators);
    if (s == NULL) {
        *savedptr = NULL;
        s = p + strlen(p);
    }
    else
    {
        *s = 0;
        *savedptr = s + 1;
    }

    // remove spaces at head
    while (p < s && isspace(*p)) {
        ++p;
    }
    // remove spaces at tail
    --s;
    while (s > p && isspace(*s)) {
        *s = 0;
        --s;
    }
    return p;
}
