//
// Created by tholz on 05.12.2022.
//

#ifndef HTTP_SERVER_WEBSOCKET_HANDSHAKE_H
#define HTTP_SERVER_WEBSOCKET_HANDSHAKE_H

#include "common.h"
#include "http_request.h"
#include "http_response.h"

ret_code_t websocket_handshake(http_request_t *p_request, http_response_t *p_response);

#endif //HTTP_SERVER_WEBSOCKET_HANDSHAKE_H
