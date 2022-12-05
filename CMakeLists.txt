cmake_minimum_required(VERSION 3.23)
project(http_server C)

set(CMAKE_C_STANDARD 99)

include_directories(
        .
        ${CMAKE_CURRENT_SOURCE_DIR}/src/http
        ${CMAKE_CURRENT_SOURCE_DIR}/src/http/server
        ${CMAKE_CURRENT_SOURCE_DIR}/src/http/headers
        ${CMAKE_CURRENT_SOURCE_DIR}/src/http/request
        ${CMAKE_CURRENT_SOURCE_DIR}/src/http/request/parser
        ${CMAKE_CURRENT_SOURCE_DIR}/src/http/request/handler
        ${CMAKE_CURRENT_SOURCE_DIR}/src/http/request/params
        ${CMAKE_CURRENT_SOURCE_DIR}/src/http/response
        ${CMAKE_CURRENT_SOURCE_DIR}/src/http/response/builder
        ${CMAKE_CURRENT_SOURCE_DIR}/src/http/method
        ${CMAKE_CURRENT_SOURCE_DIR}/src/http/status
        ${CMAKE_CURRENT_SOURCE_DIR}/src/http/route
        ${CMAKE_CURRENT_SOURCE_DIR}/src/http/interface
        ${CMAKE_CURRENT_SOURCE_DIR}/src/http/static

        ${CMAKE_CURRENT_SOURCE_DIR}/src/json
        ${CMAKE_CURRENT_SOURCE_DIR}/src/json/lexer
        ${CMAKE_CURRENT_SOURCE_DIR}/src/json/parser
        ${CMAKE_CURRENT_SOURCE_DIR}/src/json/stringifier
)

add_executable(http_server
        ${CMAKE_CURRENT_SOURCE_DIR}/src/main.c

        ${CMAKE_CURRENT_SOURCE_DIR}/src/http/server/http_server.c
        ${CMAKE_CURRENT_SOURCE_DIR}/src/http/headers/http_headers.c
        ${CMAKE_CURRENT_SOURCE_DIR}/src/http/request/http_request.c
        ${CMAKE_CURRENT_SOURCE_DIR}/src/http/request/parser/http_request_parser.c
        ${CMAKE_CURRENT_SOURCE_DIR}/src/http/request/handler/http_request_handler.c
        ${CMAKE_CURRENT_SOURCE_DIR}/src/http/request/params/http_request_params.c
        ${CMAKE_CURRENT_SOURCE_DIR}/src/http/response/http_response.c
        ${CMAKE_CURRENT_SOURCE_DIR}/src/http/response/builder/http_response_builder.c
        ${CMAKE_CURRENT_SOURCE_DIR}/src/http/method/http_method.c
        ${CMAKE_CURRENT_SOURCE_DIR}/src/http/status/http_status.c
        ${CMAKE_CURRENT_SOURCE_DIR}/src/http/route/http_route.c
        ${CMAKE_CURRENT_SOURCE_DIR}/src/http/interface/http_interface.c
        ${CMAKE_CURRENT_SOURCE_DIR}/src/http/static/http_static.c

        ${CMAKE_CURRENT_SOURCE_DIR}/src/json/json.c
        ${CMAKE_CURRENT_SOURCE_DIR}/src/json/lexer/json_lexer.c
        ${CMAKE_CURRENT_SOURCE_DIR}/src/json/parser/json_parser.c
        ${CMAKE_CURRENT_SOURCE_DIR}/src/json/stringifier/json_stringifier.c
)

add_definitions(-DSOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}")