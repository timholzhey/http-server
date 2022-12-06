//
// Created by Tim Holzhey on 03.12.22.
//

#ifndef HTTP_SERVER_HTTP_HANDLE_REQUEST_H
#define HTTP_SERVER_HTTP_HANDLE_REQUEST_H

#include "common.h"
#include "http_request.h"
#include "http_response.h"
#include "http_route.h"

ret_code_t http_request_handle(http_request_t *p_request, http_response_t *p_response, http_route_t *p_routes, uint32_t num_routes, bool *p_upgrade_websocket);

#endif //HTTP_SERVER_HTTP_HANDLE_REQUEST_H
