cmake_minimum_required(VERSION 3.23)
project(http_server C)

set(CMAKE_C_STANDARD 99)

include_directories(
        .
        http
        http/server
        http/headers
        http/request
        http/request/parser
        http/request/handler
        http/request/params
        http/response
        http/response/builder
        http/method
        http/status
        http/route
        http/interface
        http/static
)

add_executable(http_server
        main.c
        http/server/http_server.c
        http/headers/http_headers.c
        http/request/http_request.c
        http/request/parser/http_request_parser.c
        http/request/handler/http_request_handler.c
        http/request/params/http_request_params.c
        http/response/http_response.c
        http/response/builder/http_response_builder.c
        http/method/http_method.c
        http/status/http_status.c
        http/route/http_route.c
        http/interface/http_interface.c
        http/static/http_static.c
)

add_definitions(-DSOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}")