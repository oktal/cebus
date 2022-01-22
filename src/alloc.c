#include "cebus/alloc.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void* cb_alloc_safe(size_t size, const char* file, size_t line)
{
    void* p = malloc(size);
    if (p == NULL)
    {
        fprintf(stderr, "memory allocation of size %zu failed at %s:%zu",
                size, file, line);
        abort();
    }

    return p;
}

char* cb_strndup(const char* s, size_t n)
{
    char *dup = cb_alloc(n);
    strncpy(dup, s, n);
    return dup;
}

char *cb_strdup(const char* s)
{
    return cb_strndup(s, strlen(s));
}
