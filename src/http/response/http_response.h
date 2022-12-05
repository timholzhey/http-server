//
// Created by Tim Holzhey on 03.12.22.
//

#ifndef HTTP_SERVER_HTTP_RESPONSE_H
#define HTTP_SERVER_HTTP_RESPONSE_H

#include "common.h"
#include <stdint.h>
#include <stdbool.h>
#include "http_headers.h"

#define HTTP_RESPONSE_MAX_STATIC_PAYLOAD_SIZE		(10 * 1024)
#define HTTP_RESPONSE_MAX_DYNAMIC_PAYLOAD_SIZE		(100 * 1024 * 1024)

typedef struct {
	uint16_t status_code;
	http_header_t headers[HTTP_MAX_NUM_HEADERS];
	uint32_t num_headers;
	uint8_t payload[HTTP_RESPONSE_MAX_STATIC_PAYLOAD_SIZE];
	uint8_t *dynamic_payload;
	bool dynamic_payload_allocated;
	uint32_t payload_length;
} http_response_t;

ret_code_t http_response_print(http_response_t *p_response);

void http_response_reset(http_response_t *p_response);

#endif //HTTP_SERVER_HTTP_RESPONSE_H
