#pragma once

typedef enum cb_log_level
{
    cb_log_level_trace = 0,
    cb_log_level_debug,
    cb_log_level_info,
    cb_log_level_warn,
    cb_log_level_error
} cb_log_level;

typedef void (*cb_log_sink)(cb_log_level, const char*, ...);

void cb_log(cb_log_level level, const char* format_str, ...);
void cb_log_debug(const char* format_str, ...);

const char* cb_log_file_name(const char* file_path);

#ifndef CB_VA_ARGS
  #define CB_VA_ARGS(...) , ##__VA_ARGS__
#endif

#define CB_LOG_DBG(log_level, format_str, ...) \
    do { \
        cb_log(log_level, "[%s:%d] " format_str, cb_log_file_name(__FILE__), __LINE__ CB_VA_ARGS(__VA_ARGS__)); \
    } while(0)

#define CB_LOG_LEVEL_TRACE cb_log_level_trace
#define CB_LOG_LEVEL_DEBUG cb_log_level_debug
#define CB_LOG_LEVEL_INFO cb_log_level_info
#define CB_LOG_LEVEL_WARN cb_log_level_warn
#define CB_LOG_LEVEL_ERROR cb_log_level_error
