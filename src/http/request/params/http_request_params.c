//
// Created by Tim Holzhey on 04.12.22.
//

#include "http_request_params.h"
#include <stdbool.h>

ret_code_t http_request_params_get(http_request_t *p_request, const char *key, char *p_value, uint32_t *p_value_len, uint32_t max_value_len) {
	VERIFY_ARGS_NOT_NULL(p_request, key, p_value);

	*p_value_len = 0;

	// find first occurrence of ? in uri
	char *p_query = strchr(p_request->uri, '?');
	if (p_query == NULL) {
		return RET_CODE_ERROR;
	}

	// split by & and =, copy value into p_value
	while (true) {
		char *p_key = p_query + 1;
		char *p_value_end = strchr(p_key, '=');
		if (p_value_end == NULL) {
			return RET_CODE_ERROR;
		}

		// check if key matches
		if (strncmp(p_key, key, p_value_end - p_key) == 0) {
			// copy value
			p_query = p_value_end + 1;
			p_value_end = strchr(p_query, '&');
			if (p_value_end == NULL) {
				p_value_end = strchr(p_query, '\0');
			}

			*p_value_len = p_value_end - p_query;
			if (*p_value_len > max_value_len) {
				return RET_CODE_ERROR;
			}

			memcpy(p_value, p_query, *p_value_len);
			p_value[*p_value_len] = '\0';

			return RET_CODE_OK;
		}

		// find next occurrence of &
		p_query = strchr(p_value_end, '&');
		if (p_query == NULL) {
			return RET_CODE_ERROR;
		}
	}
}
