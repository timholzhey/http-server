//
// Created by tholz on 06.12.2022.
//

#ifndef HTTP_SERVER_WEBSOCKET_INTERFACE_H
#define HTTP_SERVER_WEBSOCKET_INTERFACE_H

#include <stdint.h>
#include <stdbool.h>
#include "websocket_frame.h"

typedef enum {
	WEBSOCKET_EVENT_NONE,
	WEBSOCKET_EVENT_CONNECTED,
	WEBSOCKET_EVENT_DISCONNECTED,
	WEBSOCKET_EVENT_DATA,
} websocket_event_t;

typedef struct {
	websocket_event_t event;
	uint8_t *data;
	void (*text)(const char *data);
	void (*send)(uint8_t *p_data, uint32_t len);
} websocket_interface_t;

void websocket_interface_set(websocket_event_t event);
void websocket_interface_set_frame(websocket_frame_t *p_frame);
void websocket_interface_reset();

void websocket_interface_init(websocket_interface_t *p_interface);

ret_code_t websocket_interface_get_frame(websocket_frame_t *p_frame);

#endif //HTTP_SERVER_WEBSOCKET_INTERFACE_H
