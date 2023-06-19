//
// Created by Tim Holzhey on 29.11.22.
//

#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <time.h>
#include <signal.h>
#include "http_server.h"
#include "common.h"
#include "http_request.h"
#include "http_request_parser.h"
#include "http_response.h"
#include "http_request_handler.h"
#include "http_route.h"
#include "http_response_builder.h"
#include "http_static.h"
#include "websocket_server.h"
#include "websocket_route.h"
#include "http_server_client.h"

#define HTTP_SERVER_MAX_ERROR_DESC_LENGTH		100
#define HTTP_SERVER_IP_ADDRESS_LENGTH			16

typedef enum {
	HTTP_SERVER_STATE_OFF,
	HTTP_SERVER_STATE_IDLE,
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
	http_client_t clients[HTTP_SERVER_MAX_NUM_CLIENTS];
	uint32_t num_clients;
	struct sockaddr_in server_address;
	struct sockaddr_in client_address;
	socklen_t client_address_len;
} m_http_server_internal;

static http_server_state_t http_server_state_off();
static http_server_state_t http_server_state_idle();
static http_server_state_t http_server_state_stop();

static ret_code_t http_server_handle_connection(http_client_t *p_client);

static void http_server_process();

static uint32_t http_server_get_timestamp() {
	struct timespec ts;
	clock_gettime(CLOCK_MONOTONIC, &ts);
	return ts.tv_sec * 1000 + ts.tv_nsec / 1000000;
}

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
	log_debug("Static file server initialized");
}

void http_server_start(void) {
	switch (m_http_server.state) {
		case HTTP_SERVER_STATE_OFF:
			http_server_process();
			break;
		case HTTP_SERVER_STATE_IDLE:
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
			m_http_server.state = HTTP_SERVER_STATE_STOP;
			http_server_process();
			break;
		default:
			unrecoverable_error("Invalid state");
			break;
	}
}

void http_server_route(http_route_t route) {
	if (m_http_server.num_routes >= HTTP_SERVER_MAX_NUM_ROUTES) {
		internal_error("Too many routes");
		return;
	}

	memcpy(&m_http_server.routes[m_http_server.num_routes], &route, sizeof(http_route_t));

	if (route.path[0] != '/') {
		m_http_server.routes[m_http_server.num_routes].path = malloc(strlen(route.path) + 2);
		m_http_server.routes[m_http_server.num_routes].path[0] = '/';
		strcpy(&m_http_server.routes[m_http_server.num_routes].path[1], route.path);
	} else {
		m_http_server.routes[m_http_server.num_routes].path = malloc(strlen(route.path) + 1);
		strcpy(m_http_server.routes[m_http_server.num_routes].path, route.path);
	}
	m_http_server.routes[m_http_server.num_routes].id = INT32_MAX;
	log_debug("Route %s registered", m_http_server.routes[m_http_server.num_routes].path);

	m_http_server.num_routes++;
}

void http_server_routes(http_route_t *routes, uint32_t num_routes) {
	for (uint32_t i = 0; i < num_routes; i++) {
		http_server_route(routes[i]);
	}
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

	while (bind(m_http_server_internal.server_socket_fd, (struct sockaddr *)&m_http_server_internal.server_address, sizeof(m_http_server_internal.server_address)) < 0) {
		internal_error("Failed to bind socket");
		m_http_server_internal.server_address.sin_port = htons(++m_http_server.port);
	}

	// listen for connections
	if (listen(m_http_server_internal.server_socket_fd, 3) < 0) {
		unrecoverable_error("Failed to listen for connections");
		return HTTP_SERVER_STATE_OFF;
	}

	// set to non-blocking mode
	if (fcntl(m_http_server_internal.server_socket_fd, F_SETFL, O_NONBLOCK) < 0) {
		unrecoverable_error("Failed to set socket to non-blocking mode");
		return HTTP_SERVER_STATE_OFF;
	}

	sigaction(SIGPIPE, &(struct sigaction){SIG_IGN}, NULL);

	log_info("Server listening on http://%s:%d", m_http_server.ip_addr, m_http_server.port);

	return HTTP_SERVER_STATE_IDLE;
}

static void http_server_drop_connection(http_client_t *p_client) {
	// find client
	for (uint32_t i = 0; i < m_http_server_internal.num_clients; i++) {
		if (m_http_server_internal.clients[i].socket_fd == p_client->socket_fd) {
			log_debug("Dropping connection %d", p_client->socket_fd);
			close(p_client->socket_fd);
			memset(p_client, 0, sizeof(http_client_t));

			// remove client
			for (uint32_t j = i; j < m_http_server_internal.num_clients - 1; j++) {
				memcpy(&m_http_server_internal.clients[j], &m_http_server_internal.clients[j + 1], sizeof(http_client_t));
			}
			m_http_server_internal.num_clients--;
			return;
		}
	}
}

static void http_server_client_reset(http_client_t *p_client) {
	// clear request
	http_request_reset(&p_client->request);

	// clear response
	http_response_reset(&p_client->response);

	// free dynamic buffer
	if (p_client->dynamic_buffer_out_allocated) {
		free(p_client->dynamic_buffer_out);
		p_client->dynamic_buffer_out_allocated = false;
	}

	p_client->buffer_in_len = 0;
}

static http_server_state_t http_server_state_idle() {
	for (uint32_t i = 0; i < m_http_server_internal.num_clients; i++) {
		http_client_t *p_client = &m_http_server_internal.clients[i];

		ret_code_t ret = http_server_handle_connection(p_client);

		if (ret == RET_CODE_OK || ret == RET_CODE_ERROR) {
			http_server_client_reset(p_client);
			http_server_drop_connection(p_client);
		}
	}

	http_client_t *p_client = &m_http_server_internal.clients[m_http_server_internal.num_clients];

	// accept connection with client
	p_client->socket_fd = accept(
			m_http_server_internal.server_socket_fd, (struct sockaddr *) &m_http_server_internal.client_address,
			&m_http_server_internal.client_address_len);
	if (p_client->socket_fd < 0) {
		if (errno != EAGAIN && errno != EWOULDBLOCK) {
			unrecoverable_error("Failed to accept connection with client");
			return HTTP_SERVER_STATE_OFF;
		}
		return HTTP_SERVER_STATE_IDLE;
	}

	log_debug("Accepted connection %d", p_client->socket_fd);

	// set non blocking mode
	if (fcntl(p_client->socket_fd, F_SETFL, O_NONBLOCK) < 0) {
		unrecoverable_error("Failed to set socket to non-blocking mode");
		return HTTP_SERVER_STATE_OFF;
	}

	// set keep alive
	int opt = 1;
	if (setsockopt(p_client->socket_fd, SOL_SOCKET, SO_KEEPALIVE, &opt, sizeof(opt))) {
		unrecoverable_error("Failed to set socket options");
		return HTTP_SERVER_STATE_OFF;
	}

	// init client
	p_client->activity_timestamp = http_server_get_timestamp();
	p_client->keep_alive_timeout = HTTP_SERVER_KEEP_ALIVE_TIMEOUT_MS;
	p_client->protocol = HTTP_SERVER_PROTOCOL_HTTP;
	p_client->buffer_in_len = 0;
	p_client->buffer_out_len = 0;
	p_client->dynamic_buffer_out_allocated = false;

	// add client to list of clients
	m_http_server_internal.num_clients++;

	return HTTP_SERVER_STATE_IDLE;
}

static ret_code_t http_server_handle(http_client_t *p_client, uint8_t *p_data_in, uint32_t data_in_len, uint32_t *p_num_bytes_consumed,
									 uint8_t **pp_data_out, uint32_t *p_data_out_len, uint32_t max_data_out_len, http_route_t *p_routes, uint32_t num_routes) {
	bool is_websocket_upgrade = false, is_headless = false;
	if (data_in_len == 0) {
		if (http_route_forward_stream(p_client, p_routes, num_routes) != RET_CODE_OK) {
			return RET_CODE_OK;
		}
		is_headless = true;
	} else {
		// parse request
		ret_code_t ret = http_request_parse(p_data_in, data_in_len, &p_client->request, p_num_bytes_consumed);
		if (ret == RET_CODE_BUSY) {
			return RET_CODE_BUSY;
		}
		if (ret != RET_CODE_OK) {
			internal_error("Failed to parse HTTP request");
			return RET_CODE_ERROR;
		}

		log_debug("Request: ");
		http_request_print(&p_client->request);

		// handle request
		if (http_request_handle(p_client, p_routes, num_routes, &is_websocket_upgrade) != RET_CODE_OK) {
			log_error("Failed to handle HTTP request");
			return RET_CODE_ERROR;
		}
	}

	log_debug("Response: ");
	http_response_print(&p_client->response);

	if (is_websocket_upgrade) {
		p_client->protocol = HTTP_SERVER_PROTOCOL_WEBSOCKET;
		websocket_route_assign(p_client, &p_client->request, p_routes, num_routes);
		websocket_route_forward(p_client, p_routes, num_routes);
		websocket_interface_environment_reset();
	}

	if (p_client->response.payload_length > HTTP_SERVER_STATIC_BUFFER_OUT_SIZE) {
		if (p_client->response.payload_length > HTTP_SERVER_DYNAMIC_BUFFER_OUT_SIZE) {
			internal_error("Response payload too large");
			return RET_CODE_ERROR;
		}
		p_client->dynamic_buffer_out = malloc(HTTP_SERVER_STATIC_BUFFER_OUT_SIZE + p_client->response.payload_length);
		if (p_client->dynamic_buffer_out == NULL) {
			internal_error("Failed to allocate dynamic buffer");
			return RET_CODE_ERROR;
		}
		p_client->dynamic_buffer_out_allocated = true;
		*pp_data_out = p_client->dynamic_buffer_out;
		max_data_out_len = HTTP_SERVER_DYNAMIC_BUFFER_OUT_SIZE;
	}

	// build response
	if (http_response_build(&p_client->response, *pp_data_out, p_data_out_len, max_data_out_len, is_headless) != RET_CODE_OK) {
		internal_error("Failed to build HTTP response");
		return RET_CODE_ERROR;
	}

	return RET_CODE_OK;
}

static ret_code_t http_server_handle_connection(http_client_t *p_client) {
	if (!p_client->request.payload_pending || p_client->buffer_in_len == 0) {
		// receive
		int32_t bytes_received = (int32_t) recv(p_client->socket_fd, p_client->buffer_in + p_client->buffer_in_len, HTTP_SERVER_BUFFER_IN_SIZE - p_client->buffer_in_len, 0);
		if (bytes_received == 0) {
			log_debug("Client %d closed connection", p_client->socket_fd);
			return RET_CODE_OK;
		} else if (bytes_received < 0) {
			if (errno != EAGAIN && errno != EWOULDBLOCK) {
				internal_error("Failed to receive data from client");
				log_error("%s", strerror(errno));
				return RET_CODE_ERROR;
			}
			// check timeout
			if (p_client->protocol == HTTP_SERVER_PROTOCOL_HTTP && http_server_get_timestamp() - p_client->activity_timestamp > p_client->keep_alive_timeout) {
				log_debug("Client %d timed out", p_client->socket_fd);
				return RET_CODE_OK;
			}
		} else {
			p_client->activity_timestamp = http_server_get_timestamp();

			p_client->buffer_in_len += bytes_received;

			log_debug("Received %d bytes from client %d", bytes_received, p_client->socket_fd);
		}
	}

	ret_code_t ret;
	uint32_t bytes_consumed = 0;
	uint8_t *p_buffer_out = p_client->buffer_out;
	p_client->buffer_out_len = 0;

	switch (p_client->protocol) {
		case HTTP_SERVER_PROTOCOL_HTTP:
			ret = http_server_handle(p_client, p_client->buffer_in, p_client->buffer_in_len, &bytes_consumed,
									 &p_buffer_out, &p_client->buffer_out_len, HTTP_SERVER_STATIC_BUFFER_OUT_SIZE,
									 m_http_server.routes, m_http_server.num_routes);
			break;
		case HTTP_SERVER_PROTOCOL_WEBSOCKET:
			ret = websocket_server_handle(p_client, p_client->buffer_in, p_client->buffer_in_len, &bytes_consumed,
										  &p_buffer_out, &p_client->buffer_out_len, HTTP_SERVER_STATIC_BUFFER_OUT_SIZE,
										  m_http_server.routes, m_http_server.num_routes);
			break;
		default:
			internal_error("Invalid protocol");
			return RET_CODE_ERROR;
	}
	if (ret != RET_CODE_OK && ret != RET_CODE_BUSY) {
		bytes_consumed = p_client->buffer_in_len;
	}

	if (p_client->buffer_in_len > 0) {
		// consume buffer
		p_client->buffer_in_len -= bytes_consumed;
		memmove(p_client->buffer_in, p_client->buffer_in + bytes_consumed, p_client->buffer_in_len);
	}

	if (ret == RET_CODE_BUSY) {
		return RET_CODE_BUSY;
	}

	if (p_client->buffer_out_len > 0) {
		// send response
		int32_t bytes_sent = (int32_t) send(p_client->socket_fd, p_buffer_out, p_client->buffer_out_len, 0);
		if (bytes_sent < 0) {
			internal_error("Failed to send response to client");
			return RET_CODE_ERROR;
		}

		log_debug("Sent %d bytes to client %d", bytes_sent, p_client->socket_fd);
		p_client->activity_timestamp = http_server_get_timestamp();
	}

	http_server_client_reset(p_client);

	return RET_CODE_READY;
}

static http_server_state_t http_server_state_stop() {
	return HTTP_SERVER_STATE_IDLE;
}
