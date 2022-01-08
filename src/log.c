#include "cebus/log.h"

#include <stdarg.h>
#include <stdio.h>
#include <string.h>

static void do_cb_log(cb_log_level level, const char* str)
{
    fputs(str, stdout);
    fputc('\n', stdout);
}

const char* cb_log_file_name(const char* file_path)
{
    const char* slash = strrchr(file_path, '/');
    if (slash == NULL)
        return file_path;

    return slash + 1;
}

void cb_log(cb_log_level level, const char* format_str, ...)
{
    va_list ap;
    char buf[1024];

    va_start(ap, format_str);
    vsnprintf(buf, sizeof(buf), format_str, ap);
    va_end(ap);

    do_cb_log(cb_log_level_debug, buf);
}
