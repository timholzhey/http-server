//
// Created by Tim Holzhey on 04.12.22.
//

#ifndef HTTP_SERVER_HTTP_RESPONSE_BUILDER_H
#define HTTP_SERVER_HTTP_RESPONSE_BUILDER_H

#include "common.h"
#include "http_response.h"

ret_code_t http_response_build(http_response_t *p_response, uint8_t *p_data, uint32_t *p_data_len, uint32_t max_data_len, bool is_headless);

#endif //HTTP_SERVER_HTTP_RESPONSE_BUILDER_H
