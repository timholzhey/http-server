//
// Created by Tim Holzhey on 03.12.22.
//

#include "http_request_handler.h"
#include "http_status.h"
#include "websocket_handshake.h"
#include <time.h>

static void http_request_handle_404(http_request_t *p_request, http_response_t *p_response) {
	const char *doc_404 = "<!DOCTYPE html>"
						   "<html>"
	                       "<head>"
	                       "<title>404 Not Found</title>"
	                       "</head>"
	                       "<body>"
	                       "<h1>Not Found</h1>"
	                       "<p>The requested URL was not found on this server.</p>"
	                       "</body>"
	                       "</html>";
	p_response->payload_length = strlen(doc_404);
	memcpy(p_response->payload, doc_404, p_response->payload_length);
	p_response->status_code = HTTP_STATUS_CODE_NOT_FOUND;
}

static bool is_websocket_upgrade(http_request_t *p_request) {
	char upgrade[HTTP_HEADER_MAX_VALUE_SIZE];
	if (http_headers_get_value_string(p_request->headers, p_request->num_headers, "Upgrade", upgrade) != RET_CODE_OK) {
		return false;
	}
	if (strcmp(upgrade, "websocket") != 0) {
		return false;
	}
	char connection[HTTP_HEADER_MAX_VALUE_SIZE];
	if (http_headers_get_value_string(p_request->headers, p_request->num_headers, "Connection", connection) != RET_CODE_OK) {
		return false;
	}
	if (strstr(connection, "Upgrade") == NULL) {
		return false;
	}
	char sec_websocket_key[HTTP_HEADER_MAX_VALUE_SIZE];
	if (http_headers_get_value_string(p_request->headers, p_request->num_headers, "Sec-WebSocket-Key", sec_websocket_key) != RET_CODE_OK) {
		return false;
	}
	char sec_websocket_version[HTTP_HEADER_MAX_VALUE_SIZE];
	if (http_headers_get_value_string(p_request->headers, p_request->num_headers, "Sec-WebSocket-Version", sec_websocket_version) != RET_CODE_OK) {
		return false;
	}
	if (strcmp(sec_websocket_version, "13") != 0) {
		return false;
	}
	return true;
}

ret_code_t http_request_handle(http_request_t *p_request, http_response_t *p_response, http_route_t *p_routes, uint32_t num_routes) {
	// check websocket upgrade
	if (is_websocket_upgrade(p_request)) {
		websocket_handshake(p_request, p_response);
		return RET_CODE_OK;
	}

	// set default content type header
	http_headers_set_value_string(p_response->headers, &p_response->num_headers, "Content-Type", "text/html");

	// set default server header
	http_headers_set_value_string(p_response->headers, &p_response->num_headers, "Server", "http-server");

	// route request
	if (http_route_forward(p_request, p_response, p_routes, num_routes) != RET_CODE_OK) {
		http_request_handle_404(p_request, p_response);
	}

	// set content length header
	http_headers_set_value_numeric(p_response->headers, &p_response->num_headers, "Content-Length", p_response->payload_length);

	// set date header
	time_t t = time(NULL);
	struct tm tm = *gmtime(&t);
	char date[64];
	strftime(date, sizeof(date), "%a, %d %b %Y %H:%M:%S %Z", &tm);
	http_headers_set_value_string(p_response->headers, &p_response->num_headers, "Date", date);

	return RET_CODE_OK;
}
