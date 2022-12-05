//
// Created by tholz on 05.12.2022.
//

#include "websocket_handshake.h"
#include "http_status.h"
#include "sha1.h"
#include "base64.h"

ret_code_t websocket_handshake(http_request_t *p_request, http_response_t *p_response) {
//	HTTP/1.1 101 Switching Protocols
//	Upgrade: websocket
//	Connection: Upgrade
//	Sec-WebSocket-Accept: HSmrc0sMlYUkAGmm5OPpG2HaGWk=
//  Sec-WebSocket-Protocol: chat
	p_response->status_code = HTTP_STATUS_CODE_SWITCHING_PROTOCOLS;

	RET_ON_FAIL(http_headers_set_value_string(p_response->headers, &p_response->num_headers, "Upgrade", "websocket"));
	RET_ON_FAIL(http_headers_set_value_string(p_response->headers, &p_response->num_headers, "Connection", "Upgrade"));

	// compute hash
	char sec_websocket_key[HTTP_HEADER_MAX_VALUE_SIZE];
	RET_ON_FAIL(http_headers_get_value_string(p_request->headers, p_request->num_headers, "Sec-WebSocket-Key", sec_websocket_key));

	// apply sha1
	char magic_string[] = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";
	char hash_input[HTTP_HEADER_MAX_VALUE_SIZE + sizeof(magic_string)];
	SHA1_CTX ctx;
	SHA1Init(&ctx);
	sprintf(hash_input, "%s%s", sec_websocket_key, magic_string);
	SHA1Update(&ctx, (uint8_t *) hash_input, strlen(hash_input));
#define SHA1_DIGEST_SIZE 20
	uint8_t hash[SHA1_DIGEST_SIZE];
	SHA1Final(hash, &ctx);

	// base64 encode
	char *sec_websocket_accept = base64_encode(hash);
	if (sec_websocket_accept == NULL) {
		return RET_CODE_ERROR;
	}

	RET_ON_FAIL(http_headers_set_value_string(p_response->headers, &p_response->num_headers, "Sec-WebSocket-Accept", sec_websocket_accept));

	free(sec_websocket_accept);

	return RET_CODE_OK;
}