//
// Created by Tim Holzhey on 04.12.22.
//

#ifndef HTTP_SERVER_HTTP_REQUEST_PARAMS_H
#define HTTP_SERVER_HTTP_REQUEST_PARAMS_H

#include "common.h"
#include "http_request.h"

#define HTTP_REQUEST_MAX_PARAM_VALUE_SIZE		1024

ret_code_t http_request_params_get(http_request_t *p_request, const char *key, char *p_value, uint32_t *p_value_len, uint32_t max_value_len);

#endif //HTTP_SERVER_HTTP_REQUEST_PARAMS_H
