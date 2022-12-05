//
// Created by Tim Holzhey on 03.12.22.
//

#ifndef HTTP_SERVER_HTTP_STATUS_H
#define HTTP_SERVER_HTTP_STATUS_H

typedef enum {
	HTTP_STATUS_CODE_CONTINUE 					= 100,
	HTTP_STATUS_CODE_SWITCHING_PROTOCOLS 		= 101,
	HTTP_STATUS_CODE_PROCESSING 				= 102,
	HTTP_STATUS_CODE_EARLY_HINTS 				= 103,
	HTTP_STATUS_CODE_OK 						= 200,
	HTTP_STATUS_CODE_CREATED 					= 201,
	HTTP_STATUS_CODE_ACCEPTED 					= 202,
	HTTP_STATUS_CODE_NON_AUTHORITATIVE_INFO 	= 203,
	HTTP_STATUS_CODE_NO_CONTENT 				= 204,
	HTTP_STATUS_CODE_RESET_CONTENT 				= 205,
	HTTP_STATUS_CODE_PARTIAL_CONTENT 			= 206,
	HTTP_STATUS_CODE_MULTI_STATUS 				= 207,
	HTTP_STATUS_CODE_ALREADY_REPORTED 			= 208,
	HTTP_STATUS_CODE_IM_USED 					= 226,
	HTTP_STATUS_CODE_MULTIPLE_CHOICES 			= 300,
	HTTP_STATUS_CODE_MOVED_PERMANENTLY 			= 301,
	HTTP_STATUS_CODE_FOUND 						= 302,
	HTTP_STATUS_CODE_SEE_OTHER 					= 303,
	HTTP_STATUS_CODE_NOT_MODIFIED 				= 304,
	HTTP_STATUS_CODE_USE_PROXY 					= 305,
	HTTP_STATUS_CODE_SWITCH_PROXY 				= 306,
	HTTP_STATUS_CODE_TEMPORARY_REDIRECT 		= 307,
	HTTP_STATUS_CODE_PERMANENT_REDIRECT 		= 308,
	HTTP_STATUS_CODE_BAD_REQUEST 				= 400,
	HTTP_STATUS_CODE_UNAUTHORIZED 				= 401,
	HTTP_STATUS_CODE_PAYMENT_REQUIRED 			= 402,
	HTTP_STATUS_CODE_FORBIDDEN 					= 403,
	HTTP_STATUS_CODE_NOT_FOUND 					= 404,
	HTTP_STATUS_CODE_METHOD_NOT_ALLOWED 		= 405,
	HTTP_STATUS_CODE_NOT_ACCEPTABLE 			= 406,
	HTTP_STATUS_CODE_PROXY_AUTH_REQUIRED 		= 407,
	HTTP_STATUS_CODE_REQUEST_TIMEOUT 			= 408,
	HTTP_STATUS_CODE_CONFLICT 					= 409,
	HTTP_STATUS_CODE_GONE 						= 410,
	HTTP_STATUS_CODE_LENGTH_REQUIRED 			= 411,
	HTTP_STATUS_CODE_PRECONDITION_FAILED 		= 412,
	HTTP_STATUS_CODE_PAYLOAD_TOO_LARGE 			= 413,
	HTTP_STATUS_CODE_URI_TOO_LONG 				= 414,
	HTTP_STATUS_CODE_UNSUPPORTED_MEDIA_TYPE 	= 415,
	HTTP_STATUS_CODE_RANGE_NOT_SATISFIABLE 		= 416,
	HTTP_STATUS_CODE_EXPECTATION_FAILED 		= 417,
	HTTP_STATUS_CODE_IM_A_TEAPOT 				= 418,
	HTTP_STATUS_CODE_MISDIRECTED_REQUEST 		= 421,
	HTTP_STATUS_CODE_UNPROCESSABLE_ENTITY 		= 422,
	HTTP_STATUS_CODE_LOCKED 					= 423,
	HTTP_STATUS_CODE_FAILED_DEPENDENCY 			= 424,
	HTTP_STATUS_CODE_TOO_EARLY 					= 425,
	HTTP_STATUS_CODE_UPGRADE_REQUIRED 			= 426,
	HTTP_STATUS_CODE_PRECONDITION_REQUIRED 		= 428,
	HTTP_STATUS_CODE_TOO_MANY_REQUESTS 			= 429,
	HTTP_STATUS_CODE_REQUEST_HEADER_FIELDS_TOO_LARGE = 431,
	HTTP_STATUS_CODE_UNAVAILABLE_FOR_LEGAL_REASONS = 451,
	HTTP_STATUS_CODE_INTERNAL_SERVER_ERROR 		= 500,
	HTTP_STATUS_CODE_NOT_IMPLEMENTED 			= 501,
	HTTP_STATUS_CODE_BAD_GATEWAY 				= 502,
	HTTP_STATUS_CODE_SERVICE_UNAVAILABLE 		= 503,
	HTTP_STATUS_CODE_GATEWAY_TIMEOUT 			= 504,
	HTTP_STATUS_CODE_HTTP_VERSION_NOT_SUPPORTED = 505,
	HTTP_STATUS_CODE_VARIANT_ALSO_NEGOTIATES 	= 506,
	HTTP_STATUS_CODE_INSUFFICIENT_STORAGE 		= 507,
	HTTP_STATUS_CODE_LOOP_DETECTED 				= 508,
	HTTP_STATUS_CODE_NOT_EXTENDED 				= 510,
	HTTP_STATUS_CODE_NETWORK_AUTHENTICATION_REQUIRED = 511,
} http_status_code_t;

extern const char *http_status_code_strings[];

#endif //HTTP_SERVER_HTTP_STATUS_H