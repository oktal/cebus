#include "cebus/log.h"

#include <stdarg.h>
#include <stdio.h>
#include <string.h>

static const char* cb_log_level_str(cb_log_level level)
{
    switch (level)
    {
        case cb_log_level_trace:
            return "trace";
        case cb_log_level_debug:
            return "debug";
        case cb_log_level_info:
            return "info";
        case cb_log_level_warn:
            return "warn";
        case cb_log_level_error:
            return "error";
    }

    return "";
}

static void do_cb_log(cb_log_level level, const char* str)
{
    fprintf(stdout, "(%s) ", cb_log_level_str(level));
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
