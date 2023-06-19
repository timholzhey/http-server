//
// Created by Tim Holzhey on 03.12.22.
//

#ifndef HTTP_SERVER_HTTP_INTERFACE_H
#define HTTP_SERVER_HTTP_INTERFACE_H

#include "common.h"
#include <stdint.h>
#include "http_request.h"
#include "http_response.h"
#include "http_route.h"

typedef struct {
	void (*config)(const char *ip_addr, uint16_t port);
	void (*run)(void);
	void (*start)(void);
	void (*stop)(void);
	void (*hook)(void (*hook)(void));
	void (*serve_static)(const char *path);
	void (*route)(http_route_t route);
	void (*routes)(http_route_t *p_routes, uint32_t num_routes);
} http_server_interface_t;

typedef struct {
	http_method_t method;
	char *(*param)(const char *name);
	char *(*body)(void);
	http_header_t *p_headers;
	uint32_t *p_num_headers;
} http_request_interface_t;

typedef struct {
	void (*html)(const char *html);
	void (*json)(const char *json);
	void (*text)(const char *text);
	void (*status)(uint16_t status);
	void (*append)(const uint8_t *p_data, uint32_t data_len);
	void (*append_text)(const char *text);
	void (*send)(void);
	http_header_t *p_headers;
	uint32_t *p_num_headers;
} http_response_interface_t;

extern http_request_interface_t request;
extern http_response_interface_t response;

void http_interface_environment_set(http_request_t *p_request, http_response_t *p_response);
void http_interface_environment_reset(void);

void http_interface_init_request(http_request_interface_t *p_interface);
void http_interface_init_response(http_response_interface_t *p_interface);

bool http_interface_is_response_pending(void);

void http_interface_method_not_allowed(void);

#endif //HTTP_SERVER_HTTP_INTERFACE_H
