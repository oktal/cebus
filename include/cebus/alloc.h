#pragma once

#include <stdlib.h>
#include <stddef.h>

#define cb_alloc(size) cb_alloc_safe(size, __FILE__, __LINE__)
#define cb_new(ty, size) cb_alloc(size * sizeof(ty))

#define CB_MOVE(x) x
#define CB_REF(x) x

void *cb_alloc_safe(size_t size, const char* file, size_t line);
char* cb_strndup(const char* s, size_t n);
char *cb_strdup(const char *s);
