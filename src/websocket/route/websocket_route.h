//
// Created by tholz on 06.12.2022.
//

#ifndef HTTP_SERVER_WEBSOCKET_ROUTE_H
#define HTTP_SERVER_WEBSOCKET_ROUTE_H

#include "common.h"
#include <stdbool.h>
#include "http_request.h"
#include "http_route.h"
#include "websocket_frame.h"

bool websocket_route_exists(http_request_t *p_request, http_route_t *p_routes, uint32_t num_routes);

ret_code_t websocket_route_assign(http_client_t *p_client, http_request_t *p_request, http_route_t *p_routes, uint32_t num_routes);

ret_code_t websocket_route_forward(http_client_t *p_client, http_route_t *p_routes, uint32_t num_routes);

#endif //HTTP_SERVER_WEBSOCKET_ROUTE_H
