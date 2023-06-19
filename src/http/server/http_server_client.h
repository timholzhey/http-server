//
// Created by Tim Holzhey on 19.06.23.
//

#ifndef HTTP_SERVER_HTTP_SERVER_CLIENT_H
#define HTTP_SERVER_HTTP_SERVER_CLIENT_H

#include <stdint.h>
#include <stdbool.h>
#include "http_request.h"
#include "http_response.h"
#include "http_server_protocol.h"

#define HTTP_SERVER_BUFFER_IN_SIZE				(10 * 1024)
#define HTTP_SERVER_STATIC_BUFFER_OUT_SIZE		(10 * 1024)

typedef struct {
	int socket_fd;
	uint32_t activity_timestamp;
	uint32_t keep_alive_timeout;
	http_server_protocol_t protocol;
	uint8_t buffer_in[HTTP_SERVER_BUFFER_IN_SIZE];
	uint8_t buffer_out[HTTP_SERVER_STATIC_BUFFER_OUT_SIZE];
	uint8_t *dynamic_buffer_out;
	bool dynamic_buffer_out_allocated;
	uint32_t buffer_in_len;
	uint32_t buffer_out_len;
	http_request_t request;
	http_response_t response;
	bool is_streaming;
	uint32_t streaming_route_idx;
} http_client_t;

#endif //HTTP_SERVER_HTTP_SERVER_CLIENT_H
