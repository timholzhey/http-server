//
// Created by tholz on 05.12.2022.
//

#ifndef HTTP_SERVER_HTTP_MIME_H
#define HTTP_SERVER_HTTP_MIME_H

#include "common.h"

typedef struct {
	const char *extension;
	const char *mime_type;
} http_mime_t;

extern const http_mime_t http_mime_types[];

ret_code_t http_mime_get(const char *extension, const char **p_mime_type);

#endif //HTTP_SERVER_HTTP_MIME_H
