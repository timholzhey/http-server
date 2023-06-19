//
// Created by tholz on 06.12.2022.
//

#include "websocket_frame.h"
#include "websocket_route.h"

bool websocket_route_exists(http_request_t *p_request, http_route_t *p_routes, uint32_t num_routes) {
	for (uint32_t i = 0; i < num_routes; i++) {
		if (strcmp(p_request->uri, p_routes[i].path) == 0 && p_routes[i].protocol == HTTP_SERVER_PROTOCOL_WEBSOCKET) {
			return true;
		}
	}

	return false;
}

ret_code_t websocket_route_assign(http_client_t *p_client, http_request_t *p_request, http_route_t *p_routes, uint32_t num_routes) {
	for (uint32_t i = 0; i < num_routes; i++) {
		if (strcmp(p_request->uri, p_routes[i].path) == 0) {
			p_routes[i].id = p_client->socket_fd;
			return RET_CODE_OK;
		}
	}

	return RET_CODE_ERROR;
}

ret_code_t websocket_route_forward(http_client_t *p_client, http_route_t *p_routes, uint32_t num_routes) {
	for (uint32_t i = 0; i < num_routes; i++) {
		if (p_routes[i].id == p_client->socket_fd) {
			p_routes[i].handler();
			return RET_CODE_OK;
		}
	}

	return RET_CODE_ERROR;
}

ret_code_t websocket_route_forward_stream(http_client_t *p_client, http_route_t *p_routes, uint32_t num_routes) {
	for (uint32_t i = 0; i < num_routes; i++) {
		if (p_routes[i].id == p_client->socket_fd && p_routes[i].streaming) {
			p_routes[i].handler();
			return RET_CODE_OK;
		}
	}

	return RET_CODE_ERROR;
}
