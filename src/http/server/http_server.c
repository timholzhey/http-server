//
// Created by Tim Holzhey on 29.11.22.
//

#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include "http_server.h"
#include "common.h"
#include "http_request.h"
#include "http_request_parser.h"
#include "http_response.h"
#include "http_request_handler.h"
#include "http_route.h"
#include "http_response_builder.h"
#include "http_static.h"

#define HTTP_SERVER_MAX_ERROR_DESC_LENGTH		100
#define HTTP_SERVER_IP_ADDRESS_LENGTH			16

typedef enum {
	HTTP_SERVER_STATE_OFF,
	HTTP_SERVER_STATE_IDLE,
	HTTP_SERVER_STATE_RUNNING,
	HTTP_SERVER_STATE_STOP,
} http_server_state_t;

static struct {
	http_route_t routes[HTTP_SERVER_MAX_NUM_ROUTES];
	uint32_t num_routes;
	http_server_state_t state;
	bool internal_error;
	bool unrecoverable_error;
	bool stop_requested;
	char error_desc[HTTP_SERVER_MAX_ERROR_DESC_LENGTH];
	char ip_addr[HTTP_SERVER_IP_ADDRESS_LENGTH];
	uint16_t port;
	uint32_t num_hooks;
	void (*hooks[HTTP_SERVER_MAX_NUM_HOOKS])(void);
	bool serve_static;
} m_http_server;

static struct {
	int server_socket_fd;
	int client_socket_fd;
	struct sockaddr_in server_address;
	struct sockaddr_in client_address;
	socklen_t client_address_len;
	uint8_t buffer_in[HTTP_SERVER_BUFFER_IN_SIZE];
	uint8_t buffer_out[HTTP_SERVER_STATIC_BUFFER_OUT_SIZE];
	uint8_t *dynamic_buffer_out;
	bool dynamic_buffer_out_allocated;
	uint32_t buffer_in_len;
	uint32_t buffer_out_len;
	http_request_t request;
	http_response_t response;
} m_http_server_internal;

static http_server_state_t http_server_state_off();
static http_server_state_t http_server_state_idle();
static http_server_state_t http_server_state_running();
static http_server_state_t http_server_state_stop();

static void internal_error(const char *error_desc) {
	m_http_server.internal_error = true;
	strncpy(m_http_server.error_desc, error_desc, sizeof(m_http_server.error_desc));
	log_error("Error: %s", m_http_server.error_desc);
}

static void unrecoverable_error(const char *error_desc) {
	m_http_server.unrecoverable_error = true;
	strncpy(m_http_server.error_desc, error_desc, sizeof(m_http_server.error_desc));
	log_error("Unrecoverable error: %s", m_http_server.error_desc);
}

void http_server_response(const char *response) {

}

void http_server_hook(void (*hook)(void)) {
	if (m_http_server.num_hooks >= HTTP_SERVER_MAX_NUM_HOOKS) {
		internal_error("Too many hooks");
		return;
	}

	m_http_server.hooks[m_http_server.num_hooks++] = hook;
}

void http_server_serve_static(const char *path) {
	if (http_static_init(path) != RET_CODE_OK) {
		internal_error("Failed to initialize static file server");
		return;
	}

	m_http_server.serve_static = true;
}

void http_server_start(void) {
	switch (m_http_server.state) {
		case HTTP_SERVER_STATE_OFF:
			internal_error("Cannot start server: Server is not initialized");
			break;
		case HTTP_SERVER_STATE_IDLE:
			m_http_server.state = HTTP_SERVER_STATE_RUNNING;
			break;
		case HTTP_SERVER_STATE_RUNNING:
			internal_error("Cannot start server: Server is already running");
			break;
		default:
			unrecoverable_error("Invalid state");
			break;
	}
}

void http_server_stop(void) {
	m_http_server.stop_requested = true;

	switch (m_http_server.state) {
		case HTTP_SERVER_STATE_OFF:
			internal_error("Cannot stop server: Server is not initialized");
			break;
		case HTTP_SERVER_STATE_IDLE:
			internal_error("Cannot stop server: Server is not running");
			break;
		case HTTP_SERVER_STATE_RUNNING:
			m_http_server.state = HTTP_SERVER_STATE_IDLE;
			break;
		default:
			unrecoverable_error("Invalid state");
			break;
	}
}

void http_server_route(const char *path, void (*handler)(void)) {
	if (m_http_server.num_routes >= HTTP_SERVER_MAX_NUM_ROUTES) {
		internal_error("Too many routes");
		return;
	}

	if (strlen(path) >= HTTP_SERVER_MAX_ROUTE_PATH_LENGTH) {
		internal_error("Route path too long");
		return;
	}

	if (path[0] != '/') {
		m_http_server.routes[m_http_server.num_routes].path[0] = '/';
		strncpy(&m_http_server.routes[m_http_server.num_routes].path[1], path, sizeof(m_http_server.routes[m_http_server.num_routes].path) - 1);
	} else {
		strncpy(m_http_server.routes[m_http_server.num_routes].path, path, sizeof(m_http_server.routes[m_http_server.num_routes].path));
	}

	m_http_server.routes[m_http_server.num_routes].handler = handler;
	m_http_server.num_routes++;
}

static void http_server_process() {
	if (m_http_server.unrecoverable_error) {
		return;
	}

	switch (m_http_server.state) {
		case HTTP_SERVER_STATE_OFF:
			m_http_server.state = http_server_state_off();
			break;
		case HTTP_SERVER_STATE_IDLE:
			m_http_server.state = http_server_state_idle();
			break;
		case HTTP_SERVER_STATE_RUNNING:
			m_http_server.state = http_server_state_running();
			break;
		case HTTP_SERVER_STATE_STOP:
			m_http_server.state = http_server_state_stop();
			break;
		default:
			unrecoverable_error("Invalid state");
			break;
	}

	for (uint32_t i = 0; i < m_http_server.num_hooks; i++) {
		m_http_server.hooks[i]();
	}
}

void http_server_config(const char *ip_addr, uint16_t port) {
	if (m_http_server.state != HTTP_SERVER_STATE_OFF) {
		internal_error("Cannot configure server: Server is already initialized");
		return;
	}

	if (strlen(ip_addr) >= HTTP_SERVER_IP_ADDRESS_LENGTH) {
		internal_error("IP address too long");
		return;
	}

	strncpy(m_http_server.ip_addr, ip_addr, HTTP_SERVER_IP_ADDRESS_LENGTH);
	m_http_server.port = port;
}

void http_server_run() {
	while (!m_http_server.unrecoverable_error) {
		if (m_http_server.stop_requested) {
			m_http_server.stop_requested = false;
			break;
		}
		http_server_process();
	}
}

static http_server_state_t http_server_state_off() {
	if (m_http_server.ip_addr[0] == '\0') {
		memcpy(m_http_server.ip_addr, HTTP_SERVER_DEFAULT_IP_ADDRESS, strlen(HTTP_SERVER_DEFAULT_IP_ADDRESS));
	}

	if (m_http_server.port == 0) {
		m_http_server.port = HTTP_SERVER_DEFAULT_PORT;
	}

	// create socket
	m_http_server_internal.server_socket_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (m_http_server_internal.server_socket_fd < 0) {
		unrecoverable_error("Failed to create socket");
		return HTTP_SERVER_STATE_OFF;
	}

	// set socket options
	int opt = 1;
	if (setsockopt(m_http_server_internal.server_socket_fd, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt))) {
		unrecoverable_error("Failed to set socket options");
		return HTTP_SERVER_STATE_OFF;
	}

	// bind socket to m_http_server.ip_addr string and port
	m_http_server_internal.server_address.sin_family = AF_INET;
	m_http_server_internal.server_address.sin_addr.s_addr = inet_addr(m_http_server.ip_addr);
	m_http_server_internal.server_address.sin_port = htons(m_http_server.port);

	if (bind(m_http_server_internal.server_socket_fd, (struct sockaddr *)&m_http_server_internal.server_address, sizeof(m_http_server_internal.server_address)) < 0) {
		unrecoverable_error("Failed to bind socket");
		return HTTP_SERVER_STATE_OFF;
	}

	// listen for connections
	if (listen(m_http_server_internal.server_socket_fd, 3) < 0) {
		unrecoverable_error("Failed to listen for connections");
		return HTTP_SERVER_STATE_OFF;
	}

	log_debug("Server listening on http://%s:%d", m_http_server.ip_addr, m_http_server.port);

	return HTTP_SERVER_STATE_IDLE;
}

static http_server_state_t http_server_state_idle() {
	// accept connection with client
	m_http_server_internal.client_socket_fd = accept(m_http_server_internal.server_socket_fd, (struct sockaddr *)&m_http_server_internal.client_address, (socklen_t *)&m_http_server_internal.client_address_len);
	if (m_http_server_internal.client_socket_fd < 0) {
		unrecoverable_error("Failed to accept connection with client");
		return HTTP_SERVER_STATE_OFF;
	}

	log_debug("Accepted connection");

	return HTTP_SERVER_STATE_RUNNING;
}

static http_server_state_t http_server_state_running() {
	if (!m_http_server_internal.request.payload_pending) {
		// receive
		int32_t bytes_received = (int32_t) recv(m_http_server_internal.client_socket_fd, m_http_server_internal.buffer_in + m_http_server_internal.buffer_in_len, HTTP_SERVER_BUFFER_IN_SIZE - m_http_server_internal.buffer_in_len, 0);
		if (bytes_received == 0) {
			log_debug("Client disconnected");
			return HTTP_SERVER_STATE_IDLE;
		} else if (bytes_received < 0) {
			unrecoverable_error("Failed to receive data from client");
			return HTTP_SERVER_STATE_OFF;
		}

		m_http_server_internal.buffer_in_len += bytes_received;
	}

	// parse request
	uint32_t bytes_parsed = 0;
	ret_code_t ret = http_request_parse(m_http_server_internal.buffer_in, m_http_server_internal.buffer_in_len, &m_http_server_internal.request, &bytes_parsed);
	if (ret == RET_CODE_BUSY) {
		// consume buffer
		m_http_server_internal.buffer_in_len -= bytes_parsed;
		memmove(m_http_server_internal.buffer_in, m_http_server_internal.buffer_in + bytes_parsed, m_http_server_internal.buffer_in_len);
		return HTTP_SERVER_STATE_RUNNING;
	}
	if (ret != RET_CODE_OK) {
		internal_error("Failed to parse HTTP request");
		return HTTP_SERVER_STATE_IDLE;
	}

	http_request_print(&m_http_server_internal.request);

	// handle request
	http_request_handle(&m_http_server_internal.request, &m_http_server_internal.response,
						m_http_server.routes, m_http_server.num_routes);

	uint8_t *buffer_out = m_http_server_internal.buffer_out;
	uint32_t max_buffer_out_len = HTTP_SERVER_STATIC_BUFFER_OUT_SIZE;
	if (m_http_server_internal.response.payload_length > HTTP_SERVER_STATIC_BUFFER_OUT_SIZE) {
		if (m_http_server_internal.response.payload_length > HTTP_SERVER_DYNAMIC_BUFFER_OUT_SIZE) {
			internal_error("Response payload too large");
			return HTTP_SERVER_STATE_IDLE;
		}
		m_http_server_internal.dynamic_buffer_out = malloc(m_http_server_internal.response.payload_length);
		if (m_http_server_internal.dynamic_buffer_out == NULL) {
			internal_error("Failed to allocate dynamic buffer");
			return HTTP_SERVER_STATE_IDLE;
		}
		m_http_server_internal.dynamic_buffer_out_allocated = true;
		buffer_out = m_http_server_internal.dynamic_buffer_out;
		max_buffer_out_len = HTTP_SERVER_DYNAMIC_BUFFER_OUT_SIZE;
	}

	// build response
	http_response_build(&m_http_server_internal.response, buffer_out, &m_http_server_internal.buffer_out_len, max_buffer_out_len);

	// send response
	int32_t bytes_sent = (int32_t) send(m_http_server_internal.client_socket_fd, buffer_out, m_http_server_internal.buffer_out_len, 0);
	if (bytes_sent < 0) {
		unrecoverable_error("Failed to send response to client");
		return HTTP_SERVER_STATE_OFF;
	}

	log_debug("Sent response %d/%d", bytes_sent, m_http_server_internal.buffer_out_len);

	// consume buffer
	m_http_server_internal.buffer_in_len -= bytes_parsed;
	memmove(m_http_server_internal.buffer_in, m_http_server_internal.buffer_in + bytes_parsed, m_http_server_internal.buffer_in_len);

	// clear
	memset(&m_http_server_internal.request, 0, sizeof(m_http_server_internal.request));
	http_response_reset(&m_http_server_internal.response);
	memset(&m_http_server_internal.response, 0, sizeof(m_http_server_internal.response));

	if (m_http_server_internal.dynamic_buffer_out_allocated) {
		free(m_http_server_internal.dynamic_buffer_out);
		m_http_server_internal.dynamic_buffer_out_allocated = false;
	}

	return HTTP_SERVER_STATE_RUNNING;
}

static http_server_state_t http_server_state_stop() {
	return HTTP_SERVER_STATE_IDLE;
}
