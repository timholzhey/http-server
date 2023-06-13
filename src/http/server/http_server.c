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
	uint8_t buffer_in[HTTP_SERVER_BUFFER_IN_SIZE];
	uint8_t buffer_out[HTTP_SERVER_STATIC_BUFFER_OUT_SIZE];
	uint8_t *dynamic_buffer_out;
	bool dynamic_buffer_out_allocated;
	uint32_t buffer_in_len;
	uint32_t buffer_out_len;
	http_request_t request;
	http_response_t response;
	http_client_t *p_busy_client;
	bool client_is_busy;
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

	m_http_server.routes[m_http_server.num_routes].request_handler = handler;
	m_http_server.routes[m_http_server.num_routes].protocol = HTTP_SERVER_PROTOCOL_HTTP;
	m_http_server.routes[m_http_server.num_routes].id = INT32_MAX;
	m_http_server.num_routes++;
}

void http_server_websocket(const char *path, void (*handler)(void)) {
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

	m_http_server.routes[m_http_server.num_routes].request_handler = handler;
	m_http_server.routes[m_http_server.num_routes].protocol = HTTP_SERVER_PROTOCOL_WEBSOCKET;
	m_http_server.routes[m_http_server.num_routes].id = INT32_MAX;
	m_http_server.num_routes++;
}

void http_server_websocket_streaming(const char *path, void (*handler)(void)) {
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

	m_http_server.routes[m_http_server.num_routes].request_handler = handler;
	m_http_server.routes[m_http_server.num_routes].protocol = HTTP_SERVER_PROTOCOL_WEBSOCKET;
	m_http_server.routes[m_http_server.num_routes].id = INT32_MAX;
	m_http_server.routes[m_http_server.num_routes].is_streaming = true;
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
			// remove client
			for (uint32_t j = i; j < m_http_server_internal.num_clients - 1; j++) {
				memcpy(&m_http_server_internal.clients[j], &m_http_server_internal.clients[j + 1], sizeof(http_client_t));
			}
			m_http_server_internal.num_clients--;
			m_http_server_internal.buffer_in_len = 0;
			m_http_server_internal.buffer_out_len = 0;
			return;
		}
	}
}

static http_server_state_t http_server_state_idle() {
	// TODO: Fix multiple connections
	if (m_http_server_internal.client_is_busy) {
		ret_code_t ret = http_server_handle_connection(m_http_server_internal.p_busy_client);

		if (ret == RET_CODE_OK || ret == RET_CODE_ERROR) {
			http_server_drop_connection(m_http_server_internal.p_busy_client);
			m_http_server_internal.client_is_busy = false;
		}
	}

	for (uint32_t i = 0; i < m_http_server_internal.num_clients && !m_http_server_internal.client_is_busy; i++) {
		http_client_t *p_client = &m_http_server_internal.clients[i];

		ret_code_t ret = http_server_handle_connection(p_client);

		if (ret == RET_CODE_BUSY) {
			m_http_server_internal.p_busy_client = p_client;
			m_http_server_internal.client_is_busy = true;
			break;
		}
		if (ret == RET_CODE_OK || ret == RET_CODE_ERROR) {
			http_server_drop_connection(p_client);
		}
	}

	// accept connection with client
	m_http_server_internal.clients[m_http_server_internal.num_clients].socket_fd = accept(
			m_http_server_internal.server_socket_fd, (struct sockaddr *) &m_http_server_internal.client_address,
			&m_http_server_internal.client_address_len);
	if (m_http_server_internal.clients[m_http_server_internal.num_clients].socket_fd < 0) {
		if (errno != EAGAIN && errno != EWOULDBLOCK) {
			unrecoverable_error("Failed to accept connection with client");
			return HTTP_SERVER_STATE_OFF;
		}
		return HTTP_SERVER_STATE_IDLE;
	}

	log_debug("Accepted connection %d", m_http_server_internal.clients[m_http_server_internal.num_clients].socket_fd);

	// set non blocking mode
	if (fcntl(m_http_server_internal.clients[m_http_server_internal.num_clients].socket_fd, F_SETFL, O_NONBLOCK) < 0) {
		unrecoverable_error("Failed to set socket to non-blocking mode");
		return HTTP_SERVER_STATE_OFF;
	}

	// set keep alive
	int opt = 1;
	if (setsockopt(m_http_server_internal.clients[m_http_server_internal.num_clients].socket_fd, SOL_SOCKET, SO_KEEPALIVE, &opt, sizeof(opt))) {
		unrecoverable_error("Failed to set socket options");
		return HTTP_SERVER_STATE_OFF;
	}

	// init client
	m_http_server_internal.clients[m_http_server_internal.num_clients].activity_timestamp = http_server_get_timestamp();
	m_http_server_internal.clients[m_http_server_internal.num_clients].keep_alive_timeout = HTTP_SERVER_KEEP_ALIVE_TIMEOUT_MS;
	m_http_server_internal.clients[m_http_server_internal.num_clients].protocol = HTTP_SERVER_PROTOCOL_HTTP;

	// add client to list of clients
	m_http_server_internal.num_clients++;

	return HTTP_SERVER_STATE_IDLE;
}

static ret_code_t http_server_handle(http_client_t *p_client, uint8_t *p_data_in, uint32_t data_in_len, uint32_t *p_num_bytes_consumed,
									 uint8_t **pp_data_out, uint32_t *p_data_out_len, uint32_t max_data_out_len, http_route_t *p_routes, uint32_t num_routes) {
	if (data_in_len == 0) {
		// TODO: HTTP Streaming
		return RET_CODE_OK;
	}

	// parse request
	ret_code_t ret = http_request_parse(p_data_in, data_in_len, &m_http_server_internal.request, p_num_bytes_consumed);
	if (ret == RET_CODE_BUSY) {
		return RET_CODE_BUSY;
	}
	if (ret != RET_CODE_OK) {
		internal_error("Failed to parse HTTP request");
		return RET_CODE_ERROR;
	}

	http_request_print(&m_http_server_internal.request);

	// handle request
	bool is_websocket_upgrade = false;
	http_request_handle(&m_http_server_internal.request, &m_http_server_internal.response,
						p_routes, num_routes, &is_websocket_upgrade);
	if (is_websocket_upgrade) {
		p_client->protocol = HTTP_SERVER_PROTOCOL_WEBSOCKET;
		websocket_route_assign(p_client, &m_http_server_internal.request, p_routes, num_routes);
		websocket_route_forward(p_client, p_routes, num_routes);
	}

	if (m_http_server_internal.response.payload_length > HTTP_SERVER_STATIC_BUFFER_OUT_SIZE) {
		if (m_http_server_internal.response.payload_length > HTTP_SERVER_DYNAMIC_BUFFER_OUT_SIZE) {
			internal_error("Response payload too large");
			return RET_CODE_ERROR;
		}
		m_http_server_internal.dynamic_buffer_out = malloc(HTTP_SERVER_STATIC_BUFFER_OUT_SIZE + m_http_server_internal.response.payload_length);
		if (m_http_server_internal.dynamic_buffer_out == NULL) {
			internal_error("Failed to allocate dynamic buffer");
			return RET_CODE_ERROR;
		}
		m_http_server_internal.dynamic_buffer_out_allocated = true;
		*pp_data_out = m_http_server_internal.dynamic_buffer_out;
		max_data_out_len = HTTP_SERVER_DYNAMIC_BUFFER_OUT_SIZE;
	}

	// build response
	if (http_response_build(&m_http_server_internal.response, *pp_data_out, p_data_out_len, max_data_out_len) != RET_CODE_OK) {
		internal_error("Failed to build HTTP response");
		return RET_CODE_ERROR;
	}

	return RET_CODE_OK;
}

static ret_code_t http_server_handle_connection(http_client_t *p_client) {
	if (!m_http_server_internal.request.payload_pending || m_http_server_internal.buffer_in_len == 0) {
		// receive
		int32_t bytes_received = (int32_t) recv(p_client->socket_fd, m_http_server_internal.buffer_in + m_http_server_internal.buffer_in_len, HTTP_SERVER_BUFFER_IN_SIZE - m_http_server_internal.buffer_in_len, 0);
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

			m_http_server_internal.buffer_in_len += bytes_received;

			log_debug("Received %d bytes from client %d", bytes_received, p_client->socket_fd);
		}
	}

	ret_code_t ret;
	uint32_t bytes_consumed = 0;
	uint8_t *p_buffer_out = m_http_server_internal.buffer_out;
	m_http_server_internal.buffer_out_len = 0;

	switch (p_client->protocol) {
		case HTTP_SERVER_PROTOCOL_HTTP:
			ret = http_server_handle(p_client, m_http_server_internal.buffer_in, m_http_server_internal.buffer_in_len, &bytes_consumed,
									 &p_buffer_out, &m_http_server_internal.buffer_out_len, HTTP_SERVER_STATIC_BUFFER_OUT_SIZE,
									 m_http_server.routes, m_http_server.num_routes);
			break;
		case HTTP_SERVER_PROTOCOL_WEBSOCKET:
			ret = websocket_server_handle(p_client, m_http_server_internal.buffer_in, m_http_server_internal.buffer_in_len, &bytes_consumed,
										  &p_buffer_out, &m_http_server_internal.buffer_out_len, HTTP_SERVER_STATIC_BUFFER_OUT_SIZE,
										  m_http_server.routes, m_http_server.num_routes);
			break;
		default:
			internal_error("Invalid protocol");
			return RET_CODE_ERROR;
	}
	if (ret != RET_CODE_OK && ret != RET_CODE_BUSY) {
		bytes_consumed = m_http_server_internal.buffer_in_len;
	}

	if (m_http_server_internal.buffer_in_len > 0) {
		// consume buffer
		m_http_server_internal.buffer_in_len -= bytes_consumed;
		memmove(m_http_server_internal.buffer_in, m_http_server_internal.buffer_in + bytes_consumed, m_http_server_internal.buffer_in_len);
	}

	if (ret == RET_CODE_BUSY) {
		return RET_CODE_BUSY;
	}

	if (m_http_server_internal.buffer_out_len > 0) {
		// send response
		int32_t bytes_sent = (int32_t) send(p_client->socket_fd, p_buffer_out, m_http_server_internal.buffer_out_len, 0);
		if (bytes_sent < 0) {
			internal_error("Failed to send response to client");
			return RET_CODE_ERROR;
		}

		log_debug("Sent %d bytes to client %d", bytes_sent, p_client->socket_fd);
	}

	// clear request
	memset(&m_http_server_internal.request, 0, sizeof(m_http_server_internal.request));

	// clear response
	http_response_reset(&m_http_server_internal.response);
	memset(&m_http_server_internal.response, 0, sizeof(m_http_server_internal.response));

	if (m_http_server_internal.dynamic_buffer_out_allocated) {
		free(m_http_server_internal.dynamic_buffer_out);
		m_http_server_internal.dynamic_buffer_out_allocated = false;
	}

	m_http_server_internal.buffer_in_len = 0;

	return RET_CODE_READY;
}

static http_server_state_t http_server_state_stop() {
	return HTTP_SERVER_STATE_IDLE;
}
