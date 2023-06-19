//
// Created by Tim Holzhey on 03.12.22.
//

#include "http_route.h"
#include "http_interface.h"
#include "http_static.h"
#include <stdbool.h>

ret_code_t http_route_forward(http_client_t *p_client, http_route_t *p_routes, uint32_t num_routes) {
	http_request_t *p_request = &p_client->request;
	http_response_t *p_response = &p_client->response;

	http_interface_environment_set(p_request, p_response);

	bool route_exists = false, was_handled = false;
	for (uint32_t i = 0; i < num_routes; i++) {
		char *p_delim1 = strchr(p_request->uri, '?');
		char *p_delim2 = strchr(p_request->uri, '#');
		char *p_delim = NULL;
		if (p_delim1 != NULL && p_delim2 != NULL) {
			p_delim = p_delim1 < p_delim2 ? p_delim1 : p_delim2;
		} else if (p_delim1 != NULL) {
			p_delim = p_delim1;
		} else if (p_delim2 != NULL) {
			p_delim = p_delim2;
		}

		uint32_t length = p_delim == NULL ? strlen(p_request->uri) : p_delim - p_request->uri;
		if (p_routes[i].protocol == HTTP_SERVER_PROTOCOL_HTTP &&
			strlen(p_routes[i].path) == length &&
			strncmp(p_request->uri, p_routes[i].path, length) == 0) {
			route_exists = true;
			if (p_routes[i].method != HTTP_METHOD_UNKNOWN && p_routes[i].method != p_request->method) {
				continue;
			}
			if (p_routes[i].streaming) {
				if (p_client->is_streaming) {
					log_error("Client is already streaming");
				}
				log_debug("Client %u is now streaming on route %s", p_client->socket_fd, p_routes[i].path);
				p_client->is_streaming = true;
				p_client->streaming_route_idx = i;
			}
			p_routes[i].handler();
			was_handled = true;
			break;
		}
	}

	if (route_exists && !was_handled) {
		http_interface_method_not_allowed();
	}

	http_interface_environment_reset();

	if (was_handled) {
		return RET_CODE_OK;
	}

	if (http_static_route(p_request, p_response) == RET_CODE_OK) {
		return RET_CODE_OK;
	}

	return RET_CODE_ERROR;
}

ret_code_t http_route_forward_stream(http_client_t *p_client, http_route_t *p_routes, uint32_t num_routes) {
	http_request_t *p_request = &p_client->request;
	http_response_t *p_response = &p_client->response;

	if (!p_client->is_streaming || p_client->streaming_route_idx > num_routes) {
		return RET_CODE_ERROR;
	}

	p_request->method = HTTP_METHOD_UNKNOWN;
	http_interface_environment_set(p_request, p_response);

	p_routes[p_client->streaming_route_idx].handler();

	bool has_response = http_interface_is_response_pending();

	http_interface_environment_reset();

	return has_response ? RET_CODE_OK : RET_CODE_ERROR;
}
