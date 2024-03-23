#include <stdarg.h>
#include <stdio.h>
#include "log.h"

static enum log_level log_level = LOG_INFO;

static void log_vprintf(enum log_level level, const char *fmt, va_list args) {
	const char *level_str;

	if (level < log_level) {
		return;
	}

	switch (level) {
	case LOG_DEBUG:
		level_str = "DEBUG";
		break;
	case LOG_INFO:
		level_str = "INFO";
		break;
	case LOG_ERROR:
		level_str = "ERROR";
		break;
	default:
		level_str = "UNKNOWN";
		break;
	}
	fprintf(stderr, "[%s] ", level_str);
	vfprintf(stderr, fmt, args);
	fprintf(stderr, "\n");
}

void log_set_level(enum log_level level) {
	log_level = level;
}

void log_error(const char *fmt, ...) {
	va_list args;
	va_start(args, fmt);
	log_vprintf(LOG_ERROR, fmt, args);
	va_end(args);
}

void log_info(const char *fmt, ...) {
	va_list args;
	va_start(args, fmt);
	log_vprintf(LOG_INFO, fmt, args);
	va_end(args);
}

void log_debug(const char *fmt, ...) {
	va_list args;
	va_start(args, fmt);
	log_vprintf(LOG_DEBUG, fmt, args);
	va_end(args);
}
