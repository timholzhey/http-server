//
// Created by Tim Holzhey on 03.12.22.
//

#ifndef HTTP_SERVER_HTTP_ROUTE_H
#define HTTP_SERVER_HTTP_ROUTE_H

#include "common.h"
#include "http_request.h"
#include "http_response.h"
#include "http_server_protocol.h"
#include "http_server_client.h"

typedef struct {
	char *path;
	void (*handler)(void);
	bool streaming;
	http_server_protocol_t protocol;
	http_method_t method;
	int id;
} http_route_t;

ret_code_t http_route_forward(http_client_t *p_client, http_route_t *p_routes, uint32_t num_routes);

ret_code_t http_route_forward_stream(http_client_t *p_client, http_route_t *p_routes, uint32_t num_routes);

#endif //HTTP_SERVER_HTTP_ROUTE_H
