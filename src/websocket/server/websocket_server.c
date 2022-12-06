//
// Created by Tim Holzhey on 06.12.22.
//

#include "websocket_server.h"

ret_code_t websocket_server_handle(uint8_t *p_data_in, uint32_t data_in_len, uint32_t *p_num_bytes_consumed,
									 uint8_t **pp_data_out, uint32_t *p_data_out_len, uint32_t max_data_out_len) {
	log_debug("Websocket: Handling %d bytes", data_in_len);
	return RET_CODE_ERROR;
}
