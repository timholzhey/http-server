//
// Created by tholz on 06.12.2022.
//

#include <netinet/in.h>
#include "websocket_frame.h"

#ifndef ntohll
#define ntohll(x) be64toh(x)
#endif
#ifndef htonll
#define htonll(x) htobe64(x)
#endif

ret_code_t websocket_frame_decode(uint8_t *p_data, uint32_t data_len, websocket_frame_t *p_frame, uint32_t *p_bytes_consumed) {
	VERIFY_ARGS_NOT_NULL(p_data, p_frame);

	if (data_len < sizeof(websocket_frame_header_t)) {
		log_error("Invalid frame header");
		return RET_CODE_ERROR;
	}

	p_frame->header.fin = (p_data[0] & 0x80) >> 7;
	p_frame->header.rsv1 = (p_data[0] & 0x40) >> 6;
	p_frame->header.rsv2 = (p_data[0] & 0x20) >> 5;
	p_frame->header.rsv3 = (p_data[0] & 0x10) >> 4;
	p_frame->header.opcode = p_data[0] & 0x0f;
	p_frame->header.mask = (p_data[1] & 0x80) >> 7;
	p_frame->header.payload_len = p_data[1] & 0x7f;

	*p_bytes_consumed = 2;

	if (p_frame->header.rsv1 || p_frame->header.rsv2 || p_frame->header.rsv3) {
		log_error("Invalid frame header");
		return RET_CODE_ERROR;
	}

	if (p_frame->header.payload_len == 126) {
		if (data_len < *p_bytes_consumed + 2) {
			log_error("Invalid frame header");
			return RET_CODE_ERROR;
		}

		uint16_t *p_len = (uint16_t *) (p_data + *p_bytes_consumed);
		p_frame->payload_len = ntohs(*p_len);
		*p_bytes_consumed += 2;
	} else if (p_frame->header.payload_len == 127) {
		if (data_len < *p_bytes_consumed + 8) {
			log_error("Invalid frame header");
			return RET_CODE_ERROR;
		}

		uint64_t *p_len = (uint64_t *) (p_data + *p_bytes_consumed);
		p_frame->payload_len = ntohll(*p_len);
		*p_bytes_consumed += 8;
	} else {
		p_frame->payload_len = p_frame->header.payload_len;
	}

	if (p_frame->header.mask) {
		if (data_len < *p_bytes_consumed + 4) {
			log_error("Invalid frame header");
			return RET_CODE_ERROR;
		}

		memcpy(p_frame->mask, p_data + *p_bytes_consumed, 4);
		*p_bytes_consumed += 4;
	}

	if (data_len < *p_bytes_consumed + p_frame->payload_len) {
		log_error("Invalid frame header");
		return RET_CODE_ERROR;
	}

	p_frame->p_payload = p_data + *p_bytes_consumed;

	// unmask payload
	if (p_frame->header.mask) {
		for (uint32_t i = 0; i < p_frame->payload_len; i++) {
			p_frame->p_payload[i] = p_frame->p_payload[i] ^ p_frame->mask[i % 4];
		}
	}

	// consume rest
	*p_bytes_consumed += p_frame->payload_len;

	return RET_CODE_OK;
}

// build a frame from the given data to send
ret_code_t websocket_frame_build(uint8_t *p_data, uint32_t data_len, websocket_frame_t *p_frame, websocket_opcode_t opcode) {
	VERIFY_ARGS_NOT_NULL(p_data, p_frame);

	p_frame->header.fin = 1;
	p_frame->header.rsv1 = 0;
	p_frame->header.rsv2 = 0;
	p_frame->header.rsv3 = 0;
	p_frame->header.opcode = opcode;
	p_frame->header.mask = 0;
	p_frame->payload_len = data_len;
	p_frame->p_payload = p_data;

	return RET_CODE_OK;
}

ret_code_t websocket_frame_encode(websocket_frame_t *p_frame, uint8_t *p_data_out, uint32_t *p_data_out_len, uint32_t max_data_out_len) {
	VERIFY_ARGS_NOT_NULL(p_frame, p_data_out, p_data_out_len);

	if (p_frame->payload_len > max_data_out_len) {
		log_error("Payload too large");
		return RET_CODE_ERROR;
	}

	uint32_t bytes_consumed = 0;

	p_data_out[bytes_consumed++] = (p_frame->header.fin << 7) | (p_frame->header.rsv1 << 6) | (p_frame->header.rsv2 << 5) | (p_frame->header.rsv3 << 4) | p_frame->header.opcode;

	if (p_frame->payload_len < 126) {
		p_data_out[bytes_consumed++] = p_frame->payload_len;
	} else if (p_frame->payload_len < 65536) {
		p_data_out[bytes_consumed++] = 126;
		uint16_t *p_len = (uint16_t *) (p_data_out + bytes_consumed);
		*p_len = htons(p_frame->payload_len);
		bytes_consumed += 2;
	} else {
		p_data_out[bytes_consumed++] = 127;
		uint64_t *p_len = (uint64_t *) (p_data_out + bytes_consumed);
		*p_len = htonll(p_frame->payload_len);
		bytes_consumed += 8;
	}

	memcpy(p_data_out + bytes_consumed, p_frame->p_payload, p_frame->payload_len);
	bytes_consumed += p_frame->payload_len;

	*p_data_out_len = bytes_consumed;

	return RET_CODE_OK;
}
