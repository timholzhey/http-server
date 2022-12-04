//
// Created by Tim Holzhey on 03.12.22.
//

#ifndef HTTP_SERVER_HTTP_PARSE_REQUEST_H
#define HTTP_SERVER_HTTP_PARSE_REQUEST_H

#include "common.h"
#include <stdint.h>
#include "http_request.h"

ret_code_t http_request_parse(uint8_t *p_data, uint32_t data_len, http_request_t *p_request, uint32_t *p_bytes_consumed);

#endif //HTTP_SERVER_HTTP_PARSE_REQUEST_H
