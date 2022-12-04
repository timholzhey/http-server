//
// Created by Tim Holzhey on 03.12.22.
//

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
