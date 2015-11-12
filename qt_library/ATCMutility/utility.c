#include <string.h>
#include <stdio.h>
#include "common.h"
#include "app_logprint.h"
#include "defines.h"

/**
 * @brief transform a string in hex format into int
 */
int atoh(const char * string)
{
    int value;
    sscanf (string, "%x", &value);
    return value;
}

