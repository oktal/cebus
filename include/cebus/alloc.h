#pragma once

#include <stddef.h>

#define cebus_alloc(size) cebus_alloc_safe(size, __FILE__, __LINE__)
#define cb_alloc(ty, size) cebus_alloc(size * sizeof(ty))

void *cebus_alloc_safe(size_t size, const char* file, size_t line);
char* cebus_strndup(const char* s, size_t n);
char *cebus_strdup(const char *s);
