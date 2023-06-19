//
// Created by Tim Holzhey on 03.12.22.
//

#include "http_request_handler.h"
#include "http_status.h"
#include "websocket_handshake.h"
#include "websocket_interface.h"
#include <time.h>
#include <websocket_route.h>
#include <websocket_server.h>

static void http_request_handle_route_does_not_exist(http_request_t *p_request, http_response_t *p_response) {
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

static void http_request_handle_refuse_websocket_upgrade(http_request_t *p_request, http_response_t *p_response) {
	const char *doc_400 = "<!DOCTYPE html>"
						   "<html>"
	                       "<head>"
	                       "<title>400 Bad Request</title>"
	                       "</head>"
	                       "<body>"
	                       "<h1>Bad Request</h1>"
	                       "<p>The server cannot process the request due to a client error.</p>"
	                       "</body>"
	                       "</html>";
	p_response->payload_length = strlen(doc_400);
	memcpy(p_response->payload, doc_400, p_response->payload_length);
	p_response->status_code = HTTP_STATUS_CODE_BAD_REQUEST;
}

ret_code_t http_request_handle(http_client_t *p_client, http_route_t *p_routes, uint32_t num_routes, bool *p_upgrade_websocket) {
	http_request_t *p_request = &p_client->request;
	http_response_t *p_response = &p_client->response;

	// check websocket upgrade
	if (websocket_server_is_upgrade(p_request)) {
		if (websocket_route_exists(p_request, p_routes, num_routes)) {
			if (websocket_handshake(p_request, p_response) == RET_CODE_OK) {
				*p_upgrade_websocket = true;
				websocket_interface_environment_set(WEBSOCKET_EVENT_CONNECTED);
				return RET_CODE_OK;
			}
			return RET_CODE_ERROR;
		} else {
			http_request_handle_refuse_websocket_upgrade(p_request, p_response);
		}
	}

	// set default content type header
	http_headers_set_value_string(p_response->headers, &p_response->num_headers, "Content-Type", "text/html");

	// set default server header
	http_headers_set_value_string(p_response->headers, &p_response->num_headers, "Server", "http-server");

	// route request
	if (http_route_forward(p_client, p_routes, num_routes) != RET_CODE_OK) {
		http_request_handle_route_does_not_exist(p_request, p_response);
	}

	// set date header
	time_t t = time(NULL);
	struct tm tm = *gmtime(&t);
	char date[64];
	strftime(date, sizeof(date), "%a, %d %b %Y %H:%M:%S %Z", &tm);
	http_headers_set_value_string(p_response->headers, &p_response->num_headers, "Date", date);

	return RET_CODE_OK;
}
