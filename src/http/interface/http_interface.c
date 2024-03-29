//
// Created by Tim Holzhey on 03.12.22.
//

#include <stdlib.h>
#include "http_interface.h"
#include "http_status.h"
#include "http_request_params.h"

static struct {
	http_request_t *p_request;
	http_response_t *p_response;
	void *allocations[1024];
	uint32_t num_allocations;
	bool interface_initialized;
	bool response_pending;
} m_env;

http_request_interface_t request;
http_response_interface_t response;

void http_interface_environment_set(http_request_t *p_request, http_response_t *p_response) {
	if (m_env.interface_initialized) {
		log_error("HTTP interface already initialized");
		return;
	}

	if (p_request->method != HTTP_METHOD_UNKNOWN) {
		m_env.response_pending = true;
	} else {
		m_env.response_pending = false;
	}

	m_env.p_request = p_request;
	m_env.p_response = p_response;
	m_env.num_allocations = 0;
	m_env.p_response->status_code = HTTP_STATUS_CODE_OK;

	http_interface_init_request(&request);
	http_interface_init_response(&response);

	m_env.interface_initialized = true;
}

void http_interface_environment_reset(void) {
	for (uint32_t i = 0; i < m_env.num_allocations; i++) {
		free(m_env.allocations[i]);
	}
	m_env.num_allocations = 0;
	m_env.interface_initialized = false;
}

static void http_response_text(const char *text) {
	if (!m_env.interface_initialized) {
		log_error("HTTP interface not initialized");
		return;
	}

	if (text == NULL) {
		log_error("Response text is NULL");
		return;
	}

	uint32_t len = strlen(text);
	if (len > HTTP_RESPONSE_MAX_STATIC_PAYLOAD_SIZE) {
		len = HTTP_RESPONSE_MAX_STATIC_PAYLOAD_SIZE;
	}

	m_env.p_response->payload_length = len;
	memcpy(m_env.p_response->payload, text, len);
	m_env.p_response->payload_length = len;
	http_headers_set_value_string(m_env.p_response->headers, &m_env.p_response->num_headers, "Content-Type", "text/plain");
	http_headers_set_value_numeric(m_env.p_response->headers, &m_env.p_response->num_headers, "Content-Length", len);
}

static void http_response_html(const char *html) {
	if (!m_env.interface_initialized) {
		log_error("HTTP interface not initialized");
		return;
	}

	if (html == NULL) {
		log_error("Response html is NULL");
		return;
	}

	http_response_text(html);
	http_headers_set_value_string(m_env.p_response->headers, &m_env.p_response->num_headers, "Content-Type", "text/html");
}

static void http_response_json(const char *json) {
	if (!m_env.interface_initialized) {
		log_error("HTTP interface not initialized");
		return;
	}

	if (json == NULL) {
		log_error("Response json is NULL");
		return;
	}

	http_response_text(json);
	http_headers_set_value_string(m_env.p_response->headers, &m_env.p_response->num_headers, "Content-Type", "application/json");
}

static void http_response_append(const uint8_t *p_data, uint32_t data_len) {
	if (!m_env.interface_initialized) {
		log_error("HTTP interface not initialized");
		return;
	}

	if (p_data == NULL) {
		log_error("Response data is NULL");
		return;
	}

	if (data_len > HTTP_RESPONSE_MAX_STATIC_PAYLOAD_SIZE - m_env.p_response->payload_length) {
		data_len = HTTP_RESPONSE_MAX_STATIC_PAYLOAD_SIZE - m_env.p_response->payload_length;
	}

	memcpy(m_env.p_response->payload + m_env.p_response->payload_length, p_data, data_len);
	m_env.p_response->payload_length += data_len;
	http_headers_set_value_numeric(m_env.p_response->headers, &m_env.p_response->num_headers, "Content-Length", m_env.p_response->payload_length);
}

static void http_response_append_text(const char *text) {
	if (!m_env.interface_initialized) {
		log_error("HTTP interface not initialized");
		return;
	}

	if (text == NULL) {
		log_error("Response text is NULL");
		return;
	}

	http_response_append((const uint8_t *) text, strlen(text));
}

static void http_response_status(uint16_t status) {
	if (!m_env.interface_initialized) {
		log_error("HTTP interface not initialized");
		return;
	}

	m_env.p_response->status_code = status;
}

static ret_code_t allocate(void **pp, uint32_t size) {
	if (m_env.num_allocations >= sizeof(m_env.allocations) / sizeof(m_env.allocations[0])) {
		return RET_CODE_ERROR;
	}

	*pp = malloc(size);
	if (*pp == NULL) {
		return RET_CODE_ERROR;
	}

	m_env.allocations[m_env.num_allocations++] = *pp;
	return RET_CODE_OK;
}

static char *http_request_get_param(const char *key) {
	if (!m_env.interface_initialized) {
		log_error("HTTP interface not initialized");
		return NULL;
	}

	char *value = NULL;
	if (allocate((void **) &value, HTTP_REQUEST_MAX_PARAM_VALUE_SIZE) != RET_CODE_OK) {
		return NULL;
	}

	uint32_t value_len = 0;
	if (http_request_params_get(m_env.p_request, key, value, &value_len, HTTP_REQUEST_MAX_PARAM_VALUE_SIZE) != RET_CODE_OK) {
		return NULL;
	}

	if (value_len == HTTP_REQUEST_MAX_PARAM_VALUE_SIZE) {
		return NULL;
	}

	value[value_len] = '\0';
	return value;
}

static char *http_request_get_body(void) {
	if (!m_env.interface_initialized) {
		log_error("HTTP interface not initialized");
		return NULL;
	}

	char *body = NULL;
	if (allocate((void **) &body, m_env.p_request->payload_length + 1) != RET_CODE_OK) {
		return NULL;
	}

	memcpy(body, m_env.p_request->payload, m_env.p_request->payload_length);
	body[m_env.p_request->payload_length] = '\0';

	return body;
}

static void http_response_send(void) {
	m_env.response_pending = true;
}

void http_interface_init_request(http_request_interface_t *p_interface) {
	p_interface->method = m_env.p_request->method;
	p_interface->param = http_request_get_param;
	p_interface->body = http_request_get_body;
	p_interface->p_headers = m_env.p_request->headers;
	p_interface->p_num_headers = &m_env.p_request->num_headers;
}

void http_interface_init_response(http_response_interface_t *p_interface) {
	p_interface->html = http_response_html;
	p_interface->json = http_response_json;
	p_interface->text = http_response_text;
	p_interface->status = http_response_status;
	p_interface->append = http_response_append;
	p_interface->append_text = http_response_append_text;
	p_interface->p_headers = m_env.p_response->headers;
	p_interface->p_num_headers = &m_env.p_response->num_headers;
	p_interface->send = http_response_send;
}

bool http_interface_is_response_pending(void) {
	return m_env.response_pending;
}

void http_interface_method_not_allowed(void) {
	if (!m_env.interface_initialized) {
		log_error("HTTP interface not initialized");
		return;
	}

	http_response_html("<!DOCTYPE html>\n"
					 "<title>405 Method Not Allowed</title>\n"
					 "<h1>Method Not Allowed</h1>\n"
					 "<p>The method is not allowed for the requested URL.</p>");
	http_response_status(HTTP_STATUS_CODE_METHOD_NOT_ALLOWED);
}
