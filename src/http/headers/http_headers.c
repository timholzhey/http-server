//
// Created by Tim Holzhey on 03.12.22.
//

#include "http_headers.h"
#include <stdlib.h>

ret_code_t http_headers_set_value_string(http_header_t *p_headers, uint32_t *p_num_headers, const char *key, const char *value) {
	VERIFY_ARGS_NOT_NULL(p_headers, p_num_headers, (char *) key, (char *) value);

	size_t key_length = strlen(key);
	size_t value_length = strlen(value);

	if (key_length > HTTP_HEADER_MAX_KEY_SIZE || value_length > HTTP_HEADER_MAX_VALUE_SIZE) {
		log_error("Invalid key or value");
		return RET_CODE_ERROR;
	}

	// Key already exists, change value
	for (uint32_t i = 0; i < *p_num_headers; i++) {
		if (strncmp(p_headers[i].key, key, key_length) == 0) {
			if ((p_headers[i].value = realloc(p_headers[i].value, value_length + 1)) == NULL) {
				log_error("Could not allocate memory");
				return RET_CODE_ERROR;
			}
			strcpy(p_headers[i].value, value);
			return RET_CODE_OK;
		}
	}

	if (*p_num_headers >= HTTP_MAX_NUM_HEADERS) {
		log_error("Too many response headers");
		return RET_CODE_ERROR;
	}

	if ((p_headers[*p_num_headers].key = malloc(key_length + 1)) == NULL) {
		log_error("Could not allocate memory");
		return RET_CODE_ERROR;
	}
	strcpy(p_headers[*p_num_headers].key, key);

	if ((p_headers[*p_num_headers].value = malloc(value_length + 1)) == NULL) {
		log_error("Could not allocate memory");
		return RET_CODE_ERROR;
	}
	strcpy(p_headers[*p_num_headers].value, value);

	(*p_num_headers)++;

	return RET_CODE_OK;
}

ret_code_t http_headers_set_value_numeric(http_header_t *p_headers, uint32_t *p_num_headers, const char *key, uint32_t value) {
	VERIFY_ARGS_NOT_NULL(p_headers, p_num_headers, key);

	char num_buf[16];
	snprintf(num_buf, sizeof(num_buf), "%d", value);

	return http_headers_set_value_string(p_headers, p_num_headers, key, num_buf);
}

ret_code_t http_headers_get_value_string(http_header_t *p_headers, uint32_t num_headers, const char *key, char *value) {
	VERIFY_ARGS_NOT_NULL(p_headers, (char *) key, value);

	if (strlen(key) > HTTP_HEADER_MAX_KEY_SIZE) {
		log_error("Invalid key");
		return RET_CODE_ERROR;
	}

	for (uint32_t i = 0; i < num_headers; i++) {
		if (strlen(p_headers[i].key) == strlen(key) && strncmp(p_headers[i].key, key, strlen(key)) == 0) {
			strcpy(value, p_headers[i].value);
			return RET_CODE_OK;
		}
	}

	return RET_CODE_ERROR;
}

ret_code_t http_headers_contains_key_value_string(http_header_t *p_headers, uint32_t num_headers, const char *key, const char *value) {
	VERIFY_ARGS_NOT_NULL(p_headers, (char *) key, value);

	if (strlen(key) > HTTP_HEADER_MAX_KEY_SIZE) {
		log_error("Invalid key");
		return RET_CODE_ERROR;
	}

	for (uint32_t i = 0; i < num_headers; i++) {
		if (strlen(p_headers[i].key) == strlen(key) &&
			strncmp(p_headers[i].key, key, strlen(key)) == 0 &&
			strlen(p_headers[i].value) == strlen(value) &&
			strncmp(p_headers[i].value, value, strlen(value)) == 0) {
			return RET_CODE_OK;
		}
	}

	return RET_CODE_ERROR;
}

ret_code_t http_headers_get_value_numeric(http_header_t *p_headers, uint32_t num_headers, const char *key, uint32_t *value) {
	VERIFY_ARGS_NOT_NULL(p_headers, key, value);

	char buf[HTTP_HEADER_MAX_VALUE_SIZE];
	if (http_headers_get_value_string(p_headers, num_headers, key, buf) != RET_CODE_OK) {
		return RET_CODE_ERROR;
	}
	*value = strtol(buf, NULL, 10);

	return RET_CODE_OK;
}

ret_code_t http_headers_unset(http_header_t *p_headers, uint32_t *p_num_headers, const char *key) {
	VERIFY_ARGS_NOT_NULL(p_headers, p_num_headers, key);

	if (strlen(key) > HTTP_HEADER_MAX_KEY_SIZE) {
		log_error("Invalid key");
		return RET_CODE_ERROR;
	}

	for (uint32_t i = 0; i < *p_num_headers; i++) {
		if (strlen(p_headers[i].key) == strlen(key) && strncmp(p_headers[i].key, key, strlen(key)) == 0) {
			free(p_headers[i].key);
			free(p_headers[i].value);
			for (uint32_t j = i; j < *p_num_headers - 1; j++) {
				p_headers[j] = p_headers[j + 1];
			}
			(*p_num_headers)--;
			return RET_CODE_OK;
		}
	}

	return RET_CODE_OK;
}

void http_headers_free(http_header_t *p_headers, uint32_t num_headers) {
	for (uint32_t i = 0; i < num_headers; i++) {
		free(p_headers[i].key);
		free(p_headers[i].value);
	}
}
