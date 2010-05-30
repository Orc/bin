/*
 * a more-reentrant basename function
 */
#include "config.h"
#include <string.h>

char *
basename(const char *string)
{
    char *p = strrchr((char*)string, '/');

    return p ? (1+p) : (char*)string;
}
