//
// Created by Tim Holzhey on 03.12.22.
//

#ifndef HTTP_SERVER_HTTP_HEADERS_H
#define HTTP_SERVER_HTTP_HEADERS_H

#include "common.h"
#include <stdint.h>

#define HTTP_HEADER_MAX_KEY_SIZE			1024
#define HTTP_HEADER_MAX_VALUE_SIZE			1024
#define HTTP_MAX_NUM_HEADERS				100

typedef struct {
	char key[HTTP_HEADER_MAX_KEY_SIZE];
	char value[HTTP_HEADER_MAX_VALUE_SIZE];
} http_header_t;

ret_code_t http_headers_set_value_string(http_header_t *p_headers, uint32_t *p_num_headers, const char *key, const char *value);

ret_code_t http_headers_set_value_numeric(http_header_t *p_headers, uint32_t *p_num_headers, const char *key, uint32_t value);

ret_code_t http_headers_get_value_string(http_header_t *p_headers, uint32_t num_headers, const char *key, char *value);

ret_code_t http_headers_get_value_numeric(http_header_t *p_headers, uint32_t num_headers, const char *key, uint32_t *value);

#endif //HTTP_SERVER_HTTP_HEADERS_H
