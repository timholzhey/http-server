//
// Created by Tim Holzhey on 06.12.22.
//

#include <websocket_frame.h>
#include "websocket_server.h"
#include "websocket_route.h"
#include "websocket_interface.h"

static websocket_frame_t m_frame;

#define WEBSOCKET_FRAME_MAX_SIZE		1024

static uint8_t buffer_out[WEBSOCKET_FRAME_MAX_SIZE];

ret_code_t websocket_server_handle(http_client_t *p_client, uint8_t *p_data_in, uint32_t data_in_len,
								   uint32_t *p_num_bytes_consumed,
								   uint8_t **pp_data_out, uint32_t *p_data_out_len, uint32_t max_data_out_len,
								   http_route_t *p_routes, uint32_t num_routes) {
	if (data_in_len > 0) {
		RET_ON_FAIL(websocket_frame_decode(p_data_in, data_in_len, &m_frame, p_num_bytes_consumed));

		websocket_interface_set_frame(&m_frame);

		websocket_route_forward(p_client, p_routes, num_routes);
	} else {
		websocket_interface_environment_set(WEBSOCKET_EVENT_IDLE);
		websocket_route_forward_stream(p_client, p_routes, num_routes);
	}

	if (websocket_interface_get_frame(&m_frame) == RET_CODE_OK) {
		if (websocket_frame_encode(&m_frame, buffer_out, p_data_out_len, max_data_out_len) == RET_CODE_OK) {
			*pp_data_out = buffer_out;
		}
	}

	websocket_interface_environment_reset();

	return RET_CODE_OK;
}

bool websocket_server_is_upgrade(http_request_t *p_request) {
	if (http_headers_contains_key_value_string(p_request->headers, p_request->num_headers, "Upgrade", "websocket") != RET_CODE_OK) {
		return false;
	}
	if (http_headers_contains_key_value_string(p_request->headers, p_request->num_headers, "Connection", "Upgrade") != RET_CODE_OK) {
		return false;
	}
	char sec_websocket_key[HTTP_HEADER_MAX_VALUE_SIZE];
	if (http_headers_get_value_string(p_request->headers, p_request->num_headers, "Sec-WebSocket-Key", sec_websocket_key) != RET_CODE_OK) {
		return false;
	}
	char sec_websocket_version[HTTP_HEADER_MAX_VALUE_SIZE];
	if (http_headers_contains_key_value_string(p_request->headers, p_request->num_headers, "Sec-WebSocket-Version",
											   "13") != RET_CODE_OK) {
		return false;
	}
	return true;
}