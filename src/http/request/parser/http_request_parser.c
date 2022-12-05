//
// Created by Tim Holzhey on 03.12.22.
//

#include "http_request_parser.h"
#include "http_request.h"
#include <stdbool.h>

static ret_code_t read_line_crlf(const uint8_t *p_data, uint32_t data_len, uint32_t *p_line_len) {
	for (uint32_t i = 0; i < data_len; i++) {
		if (p_data[i] == '\r' && p_data[i + 1] == '\n') {
			*p_line_len = i;
			return RET_CODE_OK;
		}
	}

	return RET_CODE_BUSY;
}

static ret_code_t read_token_delim(const uint8_t *p_data, uint32_t data_len, uint32_t *p_token_len, char delim) {
	for (uint32_t i = 0; i < data_len; i++) {
		if (p_data[i] == delim) {
			*p_token_len = i;
			return RET_CODE_OK;
		}
	}

	return RET_CODE_BUSY;
}

#define READ_LINE() \
	if (read_line_crlf(p_parse, data_len - *p_bytes_consumed, &line_length) != RET_CODE_OK) { \
		return RET_CODE_BUSY; \
	} \
	p_line = p_parse; \

#define READ_TOKEN_DELIM(delim) \
	if (read_token_delim(p_parse, line_length - line_consumed, &token_length, delim) != RET_CODE_OK) { \
		return RET_CODE_ERROR; \
	} \
	if (token_length == 0) { \
		return RET_CODE_ERROR; \
	}

#define READ_TOKEN_REST() \
	token_length = line_length - line_consumed;

#define CONSUME_TOKEN() \
	p_parse += token_length + 1; \
	line_consumed += token_length + 1;

#define CONSUME_LINE() \
	p_parse = p_line + line_length + 2; \
	*p_bytes_consumed += line_length + 2; \
	line_consumed = 0;

#define COPY_TOKEN(dest, max_len) \
	if (token_length > (max_len)) { \
		return RET_CODE_ERROR; \
	} \
	memcpy(dest, p_parse, token_length); \
	(dest)[token_length] = '\0';

ret_code_t http_request_parse(uint8_t *p_data, uint32_t data_len, http_request_t *p_request, uint32_t *p_bytes_consumed) {
	VERIFY_ARGS_NOT_NULL(p_data, p_request, p_bytes_consumed);

	if (p_request->payload_pending == 1) {
		if (p_request->payload_pending_length > 0 && data_len > 0) {
			uint32_t bytes_available = MIN(data_len, p_request->payload_pending_length);
			memcpy(p_request->payload, p_data, bytes_available);
			*p_bytes_consumed = bytes_available;
			p_request->payload_pending_length -= bytes_available;
		}
		p_request->payload_pending = 0;

		return RET_CODE_OK;
	}

	uint8_t *p_parse = p_data;
	uint8_t *p_line = p_data;
	uint32_t line_length = 0;
	uint32_t line_consumed = 0;
	uint32_t token_length = 0;

	READ_LINE();
	READ_TOKEN_DELIM(' ');

	p_request->method = HTTP_METHOD_UNKNOWN;
	for (uint32_t i = 0; i < HTTP_METHOD_COUNT; i++) {
		if (strncmp((char *) p_line, http_method_strings[i], token_length) == 0) {
			p_request->method = i;
			break;
		}
	}
	if (p_request->method == HTTP_METHOD_UNKNOWN) {
		log_error("Unknown HTTP method: %.*s", token_length, p_line);
		return RET_CODE_ERROR;
	}

	CONSUME_TOKEN();
	READ_TOKEN_DELIM(' ');

	COPY_TOKEN(p_request->uri, HTTP_REQUEST_MAX_URI_SIZE);

	CONSUME_TOKEN();
	READ_TOKEN_REST();

	COPY_TOKEN(p_request->version, HTTP_REQUEST_MAX_VERSION_SIZE);

	CONSUME_TOKEN();
	CONSUME_LINE();

	while (true) {
		if (p_request->num_headers >= HTTP_MAX_NUM_HEADERS) {
			log_error("Too many headers");
			return RET_CODE_ERROR;
		}

		READ_LINE();
		if (line_length == 0) {
			CONSUME_LINE();
			break;
		}

		READ_TOKEN_DELIM(':');

		COPY_TOKEN(p_request->headers[p_request->num_headers].key, HTTP_PARAM_MAX_KEY_SIZE);

		CONSUME_TOKEN();
		READ_TOKEN_REST();

		COPY_TOKEN(p_request->headers[p_request->num_headers].value, HTTP_PARAM_MAX_VALUE_SIZE);

		CONSUME_TOKEN();
		CONSUME_LINE();

		p_request->num_headers++;
	}

	if (http_headers_get_value_numeric(p_request->headers, p_request->num_headers, "Content-Length", &p_request->payload_length) == RET_CODE_OK) {
		if (p_request->payload_length > 0) {
			p_request->payload_pending = 1;
			p_request->payload_pending_length = p_request->payload_length;
			return RET_CODE_BUSY;
		}
	}

	return RET_CODE_OK;
}
