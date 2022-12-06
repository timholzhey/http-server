//
// Created by Tim Holzhey on 03.12.22.
//

#ifndef HTTP_SERVER_HTTP_INTERFACE_H
#define HTTP_SERVER_HTTP_INTERFACE_H

#include "common.h"
#include <stdint.h>
#include "http_request.h"
#include "http_response.h"

typedef struct {
	void (*response)(const char *response);
	void (*route)(const char *path, void (*handler)(void));
	void (*config)(const char *ip_addr, uint16_t port);
	void (*run)(void);
	void (*start)(void);
	void (*stop)(void);
	void (*hook)(void (*hook)(void));
	void (*serve_static)(const char *path);
	void (*websocket)(const char *path, void (*handler)(void));
} http_server_interface_t;

typedef struct {
	http_method_t method;
	char *(*param)(const char *name);
	char *(*body)(void);
} http_request_interface_t;

typedef struct {
	void (*html)(const char *html);
	void (*json)(const char *json);
	void (*text)(const char *text);
	void (*status)(uint16_t status);
	void (*append)(const char *text);
} http_response_interface_t;

void http_interface_environment_set(http_request_t *p_request, http_response_t *p_response);
void http_interface_environment_reset();

void http_interface_init_request(http_request_interface_t *p_interface);
void http_interface_init_response(http_response_interface_t *p_interface);

void http_interface_method_not_allowed(http_response_interface_t *p_response);

#endif //HTTP_SERVER_HTTP_INTERFACE_H
