//
// Created by Tim Holzhey on 02.12.22.
//

#ifndef HTTP_SERVER_COMMON_H
#define HTTP_SERVER_COMMON_H

#include <stdio.h>
#include <string.h>

typedef enum {
	RET_CODE_OK,
	RET_CODE_BUSY,
	RET_CODE_ERROR,
	RET_CODE_READY,
} ret_code_t;

#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))

#define FILENAME (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)

#define LOG_COLOR_WHITE		"\033[0m"
#define LOG_COLOR_RED		"\033[31m"
#define LOG_COLOR_BLUE		"\033[34m"

#define LOG_LEVEL_ERROR			0
#define LOG_LEVEL_INFO			1
#define LOG_LEVEL_HIGHLIGHT		2
#define LOG_LEVEL_DEBUG			3

#ifndef LOG_LEVEL
#define LOG_LEVEL LOG_LEVEL_GLOBAL
#endif

#if defined(DEBUG_GLOBAL) && DEBUG_GLOBAL == 1

#define log_internal(color, level, ...) \
	if (level <= LOG_LEVEL) { \
		printf(color "%-20s: %-4d: ", FILENAME, __LINE__); \
		printf(__VA_ARGS__); \
		printf(LOG_COLOR_WHITE "\n"); \
	}

#define log_internal_raw(color, level, ...) \
	if (level <= LOG_LEVEL) { \
		printf(color); \
		printf(__VA_ARGS__); \
		printf(LOG_COLOR_WHITE); \
	}

#else

#define log_internal(color, ...)

#define log_internal_raw(color, ...)

#endif

#define log_error(...)			log_internal(LOG_COLOR_RED, LOG_LEVEL_ERROR, __VA_ARGS__)
#define log_info(...)           log_internal(LOG_COLOR_WHITE, LOG_LEVEL_INFO, __VA_ARGS__)
#define log_highlight(...)		log_internal(LOG_COLOR_BLUE, LOG_LEVEL_HIGHLIGHT, __VA_ARGS__)
#define log_debug(...)			log_internal(LOG_COLOR_WHITE, LOG_LEVEL_DEBUG, __VA_ARGS__)

#define log_raw_error(...)		log_internal_raw(LOG_COLOR_RED, LOG_LEVEL_ERROR, __VA_ARGS__)
#define log_raw_info(...)       log_internal_raw(LOG_COLOR_WHITE, LOG_LEVEL_INFO, __VA_ARGS__)
#define log_raw_highlight(...)	log_internal_raw(LOG_COLOR_BLUE, LOG_LEVEL_HIGHLIGHT, __VA_ARGS__)
#define log_raw_debug(...)		log_internal_raw(LOG_COLOR_WHITE, LOG_LEVEL_DEBUG, __VA_ARGS__)

#define RET_ON_FAIL(x) \
	if ((x) != 0) { \
		return RET_CODE_ERROR; \
	}

#define EXIT_ON_FAIL(x) \
	if ((x) != 0) { \
		exit(1); \
	}

#define VERIFY_ARGS_NOT_NULL(...) \
	{ \
		const void *args[] = {__VA_ARGS__}; \
		for (uint32_t i = 0; i < sizeof(args) / sizeof(void *); i++) { \
			if (args[i] == NULL) { \
				log_error("Argument %d provided to function `%s` is NULL", i, __func__); \
				return RET_CODE_ERROR; \
			} \
		} \
	}

#endif //HTTP_SERVER_COMMON_H
