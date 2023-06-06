//
// Created by tholz on 06.12.2022.
//

#include "websocket_interface.h"
#include <stdlib.h>
#include <stdbool.h>

static struct {
	websocket_event_t event;
	uint8_t *p_data;
	websocket_frame_t frame;
	bool send_pending;
} m_env;

void websocket_interface_set(websocket_event_t event) {
	m_env.event = event;
}

void websocket_interface_set_frame(websocket_frame_t *p_frame) {
	m_env.p_data = malloc(p_frame->payload_len);
	if (m_env.p_data == NULL) {
		log_error("Failed to allocate memory for websocket frame");
		return;
	}
	memcpy(m_env.p_data, p_frame->p_payload, p_frame->payload_len + 1);
	m_env.p_data[p_frame->payload_len] = '\0';

	// check disconnected
	if (p_frame->header.opcode == WEBSOCKET_OPCODE_CLOSE) {
		m_env.event = WEBSOCKET_EVENT_DISCONNECTED;
	} else {
		m_env.event = WEBSOCKET_EVENT_DATA;
	}
}

void websocket_interface_reset() {
	m_env.event = WEBSOCKET_EVENT_NONE;
	m_env.p_data = NULL;
	m_env.send_pending = false;
}

void websocket_text(const char *data) {
	websocket_frame_build((uint8_t *) data, strlen(data), &m_env.frame, WEBSOCKET_OPCODE_TEXT);
	m_env.send_pending = true;
}

void websocket_send(uint8_t *p_data, uint32_t len) {
	websocket_frame_build(p_data, len, &m_env.frame, WEBSOCKET_OPCODE_BINARY);
	m_env.send_pending = true;
}

void websocket_interface_init(websocket_interface_t *p_interface) {
	p_interface->event = m_env.event;
	p_interface->data = m_env.p_data;
	p_interface->text = websocket_text;
	p_interface->send = websocket_send;
}

ret_code_t websocket_interface_get_frame(websocket_frame_t *p_frame) {
	if (!m_env.send_pending) {
		return RET_CODE_ERROR;
	}

	memcpy(p_frame, &m_env.frame, sizeof(websocket_frame_t));
	m_env.send_pending = false;

	return RET_CODE_OK;
}
