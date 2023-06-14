//
// Created by Tim Holzhey on 03.12.22.
//

#ifndef HTTP_SERVER_HTTP_REQUEST_H
#define HTTP_SERVER_HTTP_REQUEST_H

#include "common.h"
#include "http_method.h"
#include "http_headers.h"
#include <stdint.h>

#define HTTP_REQUEST_MAX_URI_SIZE			1024
#define HTTP_REQUEST_MAX_VERSION_SIZE		16
#define HTTP_REQUEST_MAX_PAYLOAD_SIZE		(10 * 1024)

typedef struct {
	http_method_t method;
	char uri[HTTP_REQUEST_MAX_URI_SIZE];
	char version[HTTP_REQUEST_MAX_VERSION_SIZE];
	http_header_t headers[HTTP_MAX_NUM_HEADERS];
	uint32_t num_headers;
	uint8_t payload[HTTP_REQUEST_MAX_PAYLOAD_SIZE];
	uint32_t payload_length;
	uint8_t payload_pending:1;
	uint32_t payload_pending_length;
} http_request_t;

ret_code_t http_request_print(http_request_t *p_request);

void http_request_reset(http_request_t *p_request);

#endif //HTTP_SERVER_HTTP_REQUEST_H
