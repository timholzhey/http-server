cmake_minimum_required(VERSION 3.16)
project(web_shell C)

set(CMAKE_C_STANDARD 99)

include_directories(
        .
        ${CMAKE_CURRENT_SOURCE_DIR}/../../src/http
        ${CMAKE_CURRENT_SOURCE_DIR}/../../src/http/server
        ${CMAKE_CURRENT_SOURCE_DIR}/../../src/http/headers
        ${CMAKE_CURRENT_SOURCE_DIR}/../../src/http/request
        ${CMAKE_CURRENT_SOURCE_DIR}/../../src/http/request/parser
        ${CMAKE_CURRENT_SOURCE_DIR}/../../src/http/request/handler
        ${CMAKE_CURRENT_SOURCE_DIR}/../../src/http/request/params
        ${CMAKE_CURRENT_SOURCE_DIR}/../../src/http/response
        ${CMAKE_CURRENT_SOURCE_DIR}/../../src/http/response/builder
        ${CMAKE_CURRENT_SOURCE_DIR}/../../src/http/method
        ${CMAKE_CURRENT_SOURCE_DIR}/../../src/http/status
        ${CMAKE_CURRENT_SOURCE_DIR}/../../src/http/route
        ${CMAKE_CURRENT_SOURCE_DIR}/../../src/http/interface
        ${CMAKE_CURRENT_SOURCE_DIR}/../../src/http/static
        ${CMAKE_CURRENT_SOURCE_DIR}/../../src/http/mime

        ${CMAKE_CURRENT_SOURCE_DIR}/../../src/websocket
        ${CMAKE_CURRENT_SOURCE_DIR}/../../src/websocket/server
        ${CMAKE_CURRENT_SOURCE_DIR}/../../src/websocket/handshake
        ${CMAKE_CURRENT_SOURCE_DIR}/../../src/websocket/route
        ${CMAKE_CURRENT_SOURCE_DIR}/../../src/websocket/frame
        ${CMAKE_CURRENT_SOURCE_DIR}/../../src/websocket/interface

        ${CMAKE_CURRENT_SOURCE_DIR}/../../src/json
        ${CMAKE_CURRENT_SOURCE_DIR}/../../src/json/lexer
        ${CMAKE_CURRENT_SOURCE_DIR}/../../src/json/parser
        ${CMAKE_CURRENT_SOURCE_DIR}/../../src/json/stringifier

        ${CMAKE_CURRENT_SOURCE_DIR}/../../src/lib
        ${CMAKE_CURRENT_SOURCE_DIR}/../../src/lib/sha1
        ${CMAKE_CURRENT_SOURCE_DIR}/../../src/lib/base64
)

add_executable(web_shell
        ${CMAKE_CURRENT_SOURCE_DIR}/main.c

        ${CMAKE_CURRENT_SOURCE_DIR}/../../src/http/server/http_server.c
        ${CMAKE_CURRENT_SOURCE_DIR}/../../src/http/headers/http_headers.c
        ${CMAKE_CURRENT_SOURCE_DIR}/../../src/http/request/http_request.c
        ${CMAKE_CURRENT_SOURCE_DIR}/../../src/http/request/parser/http_request_parser.c
        ${CMAKE_CURRENT_SOURCE_DIR}/../../src/http/request/handler/http_request_handler.c
        ${CMAKE_CURRENT_SOURCE_DIR}/../../src/http/request/params/http_request_params.c
        ${CMAKE_CURRENT_SOURCE_DIR}/../../src/http/response/http_response.c
        ${CMAKE_CURRENT_SOURCE_DIR}/../../src/http/response/builder/http_response_builder.c
        ${CMAKE_CURRENT_SOURCE_DIR}/../../src/http/method/http_method.c
        ${CMAKE_CURRENT_SOURCE_DIR}/../../src/http/status/http_status.c
        ${CMAKE_CURRENT_SOURCE_DIR}/../../src/http/route/http_route.c
        ${CMAKE_CURRENT_SOURCE_DIR}/../../src/http/interface/http_interface.c
        ${CMAKE_CURRENT_SOURCE_DIR}/../../src/http/static/http_static.c
        ${CMAKE_CURRENT_SOURCE_DIR}/../../src/http/mime/http_mime.c

        ${CMAKE_CURRENT_SOURCE_DIR}/../../src/websocket/server/websocket_server.c
        ${CMAKE_CURRENT_SOURCE_DIR}/../../src/websocket/handshake/websocket_handshake.c
        ${CMAKE_CURRENT_SOURCE_DIR}/../../src/websocket/route/websocket_route.c
        ${CMAKE_CURRENT_SOURCE_DIR}/../../src/websocket/frame/websocket_frame.c
        ${CMAKE_CURRENT_SOURCE_DIR}/../../src/websocket/interface/websocket_interface.c

        ${CMAKE_CURRENT_SOURCE_DIR}/../../src/json/json.c
        ${CMAKE_CURRENT_SOURCE_DIR}/../../src/json/lexer/json_lexer.c
        ${CMAKE_CURRENT_SOURCE_DIR}/../../src/json/parser/json_parser.c
        ${CMAKE_CURRENT_SOURCE_DIR}/../../src/json/stringifier/json_stringifier.c

        ${CMAKE_CURRENT_SOURCE_DIR}/../../src/lib/sha1/sha1.c
        ${CMAKE_CURRENT_SOURCE_DIR}/../../src/lib/base64/base64.c
        )

add_definitions(
        -DHTTP_SERVER_ROOT_DIR="${CMAKE_CURRENT_SOURCE_DIR}"
        -DDEBUG_GLOBAL=1
)
