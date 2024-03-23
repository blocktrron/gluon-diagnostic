#pragma once

enum log_level {
    LOG_DEBUG,
    LOG_INFO,
    LOG_WARNING,
    LOG_ERROR,
    LOG_FATAL
};

void log_set_level(enum log_level level);

void log_error(const char *fmt, ...);
void log_info(const char *fmt, ...);
void log_debug(const char *fmt, ...);
