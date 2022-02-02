//! This file defines common utilities to work with strings
#pragma once

#include <stddef.h>

/// Replace the `needle` string inside `src` by `str` and write the resulting output to `out` 
void cb_str_replace(const char* src, const char* needle, const char* str, char* out, size_t size);
