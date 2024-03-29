//
// Created by Tim Holzhey on 03.12.22.
//

#include <stdlib.h>
#include "http_response.h"

ret_code_t http_response_print(http_response_t *p_response) {
	VERIFY_ARGS_NOT_NULL(p_response);

	log_debug("{");
	log_debug("  \"status_code\": \"%d\",", p_response->status_code);
	log_debug("  \"headers\": {");
	for (uint32_t i = 0; i < p_response->num_headers; i++) {
		log_debug("    \"%s\": \"%s\",", p_response->headers[i].key, p_response->headers[i].value);
	}
	log_debug("  }");
	log_debug("}");

	return RET_CODE_OK;
}

void http_response_reset(http_response_t *p_response) {
	http_headers_free(p_response->headers, p_response->num_headers);

	if (p_response->dynamic_payload_allocated) {
		free(p_response->dynamic_payload);
		p_response->dynamic_payload_allocated = false;
	}

	memset(p_response, 0, sizeof(http_response_t));
}
