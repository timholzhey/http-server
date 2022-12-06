//
// Created by Tim Holzhey on 29.11.22.
//

#ifndef HTTP_SERVER_HTTP_SERVER_H
#define HTTP_SERVER_HTTP_SERVER_H

#include <stdint.h>
#include "http_interface.h"
#include "websocket_interface.h"
#include "http_method.h"

#define HTTP_SERVER_MAX_NUM_ROUTES				100
#define HTTP_SERVER_MAX_NUM_HOOKS				100
#define HTTP_SERVER_BUFFER_IN_SIZE				(10 * 1024)
#define HTTP_SERVER_STATIC_BUFFER_OUT_SIZE		(10 * 1024)
#define HTTP_SERVER_DYNAMIC_BUFFER_OUT_SIZE		(100 * 1024 * 1024)
#define HTTP_SERVER_DEFAULT_IP_ADDRESS			"127.0.0.1"
#define HTTP_SERVER_DEFAULT_PORT				5123
#define HTTP_SERVER_MAX_NUM_CLIENTS				1000
#define HTTP_SERVER_KEEP_ALIVE_TIMEOUT_MS		10000

typedef enum {
	HTTP_SERVER_PROTOCOL_HTTP,
	HTTP_SERVER_PROTOCOL_WEBSOCKET,
} http_server_protocol_t;

typedef struct {
	int socket_fd;
	uint32_t activity_timestamp;
	uint32_t keep_alive_timeout;
	http_server_protocol_t protocol;
} http_client_t;

void http_server_response(const char *response);
void http_server_route(const char *path, void (*handler)(void));
void http_server_config(const char *ip_addr, uint16_t port);
void http_server_run();
void http_server_start(void);
void http_server_stop(void);
void http_server_hook(void (*hook)(void));
void http_server_serve_static(const char *path);
void http_server_websocket(const char *path, void (*handler)(void));

#define HTTP_SERVER(name) \
	static http_server_interface_t name = { \
    	.response = http_server_response, \
		.route = http_server_route, \
        .config = http_server_config, \
		.run = http_server_run,     \
		.start = http_server_start, \
		.stop = http_server_stop,   \
		.hook = http_server_hook,   \
        .serve_static = http_server_serve_static, \
        .websocket = http_server_websocket,     \
	};

#define HTTP_ROUTE(name, body) \
	void name(void) { \
        http_request_interface_t request; \
		http_response_interface_t response; \
		http_interface_init_request(&request);  \
		http_interface_init_response(&response); \
        {body}                         \
	}

#define HTTP_ROUTE_METHOD(name, _method, body) \
	void name(void) { \
        http_request_interface_t request; \
        http_response_interface_t response; \
        http_interface_init_request(&request);  \
        http_interface_init_response(&response); \
        if (request.method != (_method)) {     \
            http_interface_method_not_allowed(&response);	   \
            return; \
        }                                           \
        {body}                                      \
	}

#define WEBSOCKET_ROUTE(name, body) \
	void name(void) {                  \
    	websocket_interface_t websocket;     \
    	websocket_interface_init(&websocket); \
        {body}                                      \
	}

#endif //HTTP_SERVER_HTTP_SERVER_H
