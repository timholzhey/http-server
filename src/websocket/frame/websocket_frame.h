//
// Created by tholz on 06.12.2022.
//

#ifndef HTTP_SERVER_WEBSOCKET_FRAME_H
#define HTTP_SERVER_WEBSOCKET_FRAME_H

#include "common.h"
#include <stdint.h>

typedef struct {
	uint8_t fin:1;
	uint8_t rsv1:1;
	uint8_t rsv2:1;
	uint8_t rsv3:1;
	uint8_t opcode:4;
	uint8_t mask:1;
	uint8_t payload_len:7;
} websocket_frame_header_t;

typedef struct {
	websocket_frame_header_t header;
	uint64_t payload_len;
	uint8_t mask[4];
	uint8_t *p_payload;
} websocket_frame_t;

typedef enum {
	WEBSOCKET_OPCODE_CONTINUATION = 0x0,
	WEBSOCKET_OPCODE_TEXT = 0x1,
	WEBSOCKET_OPCODE_BINARY = 0x2,
	WEBSOCKET_OPCODE_CLOSE = 0x8,
	WEBSOCKET_OPCODE_PING = 0x9,
	WEBSOCKET_OPCODE_PONG = 0xa,
} websocket_opcode_t;

ret_code_t websocket_frame_decode(uint8_t *p_data, uint32_t data_len, websocket_frame_t *p_frame, uint32_t *p_bytes_consumed);

ret_code_t websocket_frame_build(uint8_t *p_data, uint32_t data_len, websocket_frame_t *p_frame);

ret_code_t websocket_frame_encode(websocket_frame_t *p_frame, uint8_t *p_data_out, uint32_t *p_data_out_len, uint32_t max_data_out_len);

#endif //HTTP_SERVER_WEBSOCKET_FRAME_H
