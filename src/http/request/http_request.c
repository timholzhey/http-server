//
// Created by Tim Holzhey on 03.12.22.
//

#include "http_request.h"

ret_code_t http_request_print(http_request_t *p_request) {
	VERIFY_ARGS_NOT_NULL(p_request);

	log_debug("{");
	log_debug("  \"method\": \"%s\",", http_method_strings[p_request->method]);
	log_debug("  \"uri\": \"%s\",", p_request->uri);
	log_debug("  \"version\": \"%s\",", p_request->version);
	log_debug("  \"headers\": {");
	for (uint32_t i = 0; i < p_request->num_headers; i++) {
		log_debug("    \"%s\": \"%s\",", p_request->headers[i].key, p_request->headers[i].value);
	}
	log_debug("  }");
	log_debug("}");

	return RET_CODE_OK;
}

void http_request_reset(http_request_t *p_request) {
	http_headers_free(p_request->headers, p_request->num_headers);
	memset(p_request, 0, sizeof(http_request_t));
}
