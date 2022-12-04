//
// Created by Tim Holzhey on 04.12.22.
//

#ifndef HTTP_SERVER_HTTP_STATIC_H
#define HTTP_SERVER_HTTP_STATIC_H

#include "common.h"
#include "http_request.h"
#include "http_response.h"

ret_code_t http_static_init(const char *path);

ret_code_t http_static_route(http_request_t *p_request, http_response_t *p_response);

#endif //HTTP_SERVER_HTTP_STATIC_H
