//
// Created by Tim Holzhey on 03.12.22.
//

#include "http_method.h"

const char *http_method_strings[] = {
		[HTTP_METHOD_GET] = "GET",
		[HTTP_METHOD_POST] = "POST",
		[HTTP_METHOD_PUT] = "PUT",
		[HTTP_METHOD_DELETE] = "DELETE",
		[HTTP_METHOD_HEAD] = "HEAD",
		[HTTP_METHOD_OPTIONS] = "OPTIONS",
		[HTTP_METHOD_TRACE] = "TRACE",
		[HTTP_METHOD_CONNECT] = "CONNECT",
		[HTTP_METHOD_PATCH] = "PATCH",
		[HTTP_METHOD_UNKNOWN] = "UNKNOWN",
};