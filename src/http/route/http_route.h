//
// Created by Tim Holzhey on 03.12.22.
//

#ifndef HTTP_SERVER_HTTP_ROUTE_H
#define HTTP_SERVER_HTTP_ROUTE_H

#include "common.h"
#include "http_request.h"
#include "http_response.h"
#include "http_server.h"

#define HTTP_SERVER_MAX_ROUTE_PATH_LENGTH	100

typedef struct {
	char path[HTTP_SERVER_MAX_ROUTE_PATH_LENGTH];
	void (*request_handler)(void);
	http_server_protocol_t protocol;
	int id;
	bool is_streaming;
} http_route_t;

ret_code_t http_route_forward(http_request_t *p_request, http_response_t *p_response, http_route_t *p_routes, uint32_t num_routes);

#endif //HTTP_SERVER_HTTP_ROUTE_H
