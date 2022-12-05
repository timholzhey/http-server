//
// Created by Tim Holzhey on 04.12.22.
//

#include "http_response_builder.h"
#include "http_status.h"

ret_code_t http_response_build(http_response_t *p_response, uint8_t *p_data, uint32_t *p_data_len, uint32_t max_data_len) {
	VERIFY_ARGS_NOT_NULL(p_response, p_data, p_data_len);

	*p_data_len = 0;
	const char *status = http_status_code_strings[p_response->status_code];

	*p_data_len += snprintf((char *) p_data + *p_data_len, max_data_len - *p_data_len, "HTTP/1.1 %d %s\r\n", p_response->status_code, status);

	for (uint32_t i = 0; i < p_response->num_headers; i++) {
		if (*p_data_len + strlen(p_response->headers[i].key) + strlen(p_response->headers[i].value) + 4 > max_data_len) {
			log_error("Response header too large");
			return RET_CODE_ERROR;
		}
		*p_data_len += snprintf((char *) p_data + *p_data_len, max_data_len - *p_data_len, "%s: %s\r\n",
								p_response->headers[i].key, p_response->headers[i].value);
	}

	*p_data_len += snprintf((char *) p_data + *p_data_len, max_data_len - *p_data_len, "\r\n");

	if (p_response->payload_length > 0) {
		if (*p_data_len + p_response->payload_length + 4 > max_data_len) {
			log_error("Payload too large");
			return RET_CODE_ERROR;
		}
		if (p_response->dynamic_payload_allocated) {
			memcpy(p_data + *p_data_len, p_response->dynamic_payload, p_response->payload_length);
		} else {
			memcpy(p_data + *p_data_len, p_response->payload, p_response->payload_length);
		}
		*p_data_len += p_response->payload_length;
	}

	return RET_CODE_OK;
}
