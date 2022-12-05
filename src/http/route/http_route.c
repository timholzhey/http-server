//
// Created by Tim Holzhey on 03.12.22.
//

#include "http_route.h"
#include "http_interface.h"
#include "http_static.h"
#include <stdbool.h>

ret_code_t http_route_forward(http_request_t *p_request, http_response_t *p_response, http_route_t *p_routes, uint32_t num_routes) {
	http_interface_environment_set(p_request, p_response);

	bool was_handled = false;
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
		if (strncmp(p_request->uri, p_routes[i].path, length) == 0 && strlen(p_routes[i].path) == length) {
			p_routes[i].handler();
			was_handled = true;
			break;
		}
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
