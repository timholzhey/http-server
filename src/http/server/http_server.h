//
// Created by Tim Holzhey on 29.11.22.
//

#ifndef HTTP_SERVER_HTTP_SERVER_H
#define HTTP_SERVER_HTTP_SERVER_H

#include <stdint.h>
#include "http_interface.h"
#include "websocket_interface.h"
#include "http_method.h"
#include "http_route.h"
#include "http_server_protocol.h"
#include "http_status.h"

#define HTTP_SERVER_MAX_NUM_ROUTES				100
#define HTTP_SERVER_MAX_NUM_HOOKS				100
#define HTTP_SERVER_DYNAMIC_BUFFER_OUT_SIZE		(100 * 1024 * 1024)
#define HTTP_SERVER_DEFAULT_IP_ADDRESS			"127.0.0.1"
#define HTTP_SERVER_DEFAULT_PORT				5123
#define HTTP_SERVER_MAX_NUM_CLIENTS				100
#define HTTP_SERVER_KEEP_ALIVE_TIMEOUT_MS		10000

void http_server_config(const char *ip_addr, uint16_t port);
void http_server_run();
void http_server_start(void);
void http_server_stop(void);
void http_server_hook(void (*hook)(void));
void http_server_serve_static(const char *path);
void http_server_route(http_route_t route);
void http_server_routes(http_route_t *p_routes, uint32_t num_routes);

#define HTTP_SERVER(name) \
	static http_server_interface_t name = { \
        .config = http_server_config, \
		.run = http_server_run,     \
		.start = http_server_start, \
		.stop = http_server_stop,   \
		.hook = http_server_hook,   \
        .serve_static = http_server_serve_static, \
		.route = http_server_route,            \
        .routes = http_server_routes,          \
	};

#define HTTP_ROUTE(_path, name) \
	void name ## _handler(void); \
	http_route_t name = { \
		.path = (_path), \
		.handler = name ## _handler, \
        .streaming = false,       \
        .protocol = HTTP_SERVER_PROTOCOL_HTTP, \
        .method = HTTP_METHOD_UNKNOWN, \
	};                            \
	void name ## _handler(void)

#define HTTP_ROUTE_METHOD(_path, name, _method) \
	void name ## _handler(void); \
	http_route_t name = { \
		.path = (_path), \
		.handler = name ## _handler, \
		.streaming = false,       \
		.protocol = HTTP_SERVER_PROTOCOL_HTTP, \
		.method = (_method), \
	};                            \
	void name ## _handler(void)

#define WEBSOCKET_ROUTE(_path, name) \
	void name ## _handler(void); \
	http_route_t name = { \
		.path = (_path), \
		.handler = name ## _handler, \
		.streaming = false,       \
		.protocol = HTTP_SERVER_PROTOCOL_WEBSOCKET, \
		.method = HTTP_METHOD_UNKNOWN, \
	};                            \
	void name ## _handler(void)

#endif //HTTP_SERVER_HTTP_SERVER_H
