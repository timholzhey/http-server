//
// Created by Tim Holzhey on 03.12.22.
//

#include "http_request_handler.h"
#include "http_status.h"

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

ret_code_t http_request_handle(http_request_t *p_request, http_response_t *p_response, http_route_t *p_routes, uint32_t num_routes) {
	http_headers_set_value_string(p_response->headers, &p_response->num_headers, "Content-Type", "text/html");
	http_headers_set_value_string(p_response->headers, &p_response->num_headers, "Server", "http-server");

	// route
	if (http_route_forward(p_request, p_response, p_routes, num_routes) != RET_CODE_OK) {
		http_request_handle_404(p_request, p_response);
	}

	http_headers_set_value_numeric(p_response->headers, &p_response->num_headers, "Content-Length", p_response->payload_length);

	return RET_CODE_OK;
}
