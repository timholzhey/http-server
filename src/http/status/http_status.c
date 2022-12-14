//
// Created by Tim Holzhey on 03.12.22.
//

#include "http_status.h"

const char *http_status_code_strings[] = {
		[HTTP_STATUS_CODE_CONTINUE]						= "Continue",
		[HTTP_STATUS_CODE_SWITCHING_PROTOCOLS]			= "Switching Protocols",
		[HTTP_STATUS_CODE_PROCESSING]					= "Processing",
		[HTTP_STATUS_CODE_EARLY_HINTS]					= "Early Hints",
		[HTTP_STATUS_CODE_OK]							= "OK",
		[HTTP_STATUS_CODE_CREATED]						= "Created",
		[HTTP_STATUS_CODE_ACCEPTED]						= "Accepted",
		[HTTP_STATUS_CODE_NON_AUTHORITATIVE_INFO]		= "Non-Authoritative Information",
		[HTTP_STATUS_CODE_NO_CONTENT]					= "No Content",
		[HTTP_STATUS_CODE_RESET_CONTENT]				= "Reset Content",
		[HTTP_STATUS_CODE_PARTIAL_CONTENT]				= "Partial Content",
		[HTTP_STATUS_CODE_MULTI_STATUS]					= "Multi-Status",
		[HTTP_STATUS_CODE_ALREADY_REPORTED]				= "Already Reported",
		[HTTP_STATUS_CODE_IM_USED]						= "IM Used",
		[HTTP_STATUS_CODE_MULTIPLE_CHOICES]				= "Multiple Choices",
		[HTTP_STATUS_CODE_MOVED_PERMANENTLY]			= "Moved Permanently",
		[HTTP_STATUS_CODE_FOUND]						= "Found",
		[HTTP_STATUS_CODE_SEE_OTHER]					= "See Other",
		[HTTP_STATUS_CODE_NOT_MODIFIED]					= "Not Modified",
		[HTTP_STATUS_CODE_USE_PROXY]					= "Use Proxy",
		[HTTP_STATUS_CODE_SWITCH_PROXY]					= "Switch Proxy",
		[HTTP_STATUS_CODE_TEMPORARY_REDIRECT]			= "Temporary Redirect",
		[HTTP_STATUS_CODE_PERMANENT_REDIRECT]			= "Permanent Redirect",
		[HTTP_STATUS_CODE_BAD_REQUEST]					= "Bad Request",
		[HTTP_STATUS_CODE_UNAUTHORIZED]					= "Unauthorized",
		[HTTP_STATUS_CODE_PAYMENT_REQUIRED]				= "Payment Required",
		[HTTP_STATUS_CODE_FORBIDDEN]					= "Forbidden",
		[HTTP_STATUS_CODE_NOT_FOUND]					= "Not Found",
		[HTTP_STATUS_CODE_METHOD_NOT_ALLOWED]			= "Method Not Allowed",
		[HTTP_STATUS_CODE_NOT_ACCEPTABLE]				= "Not Acceptable",
		[HTTP_STATUS_CODE_PROXY_AUTH_REQUIRED]			= "Proxy Authentication Required",
		[HTTP_STATUS_CODE_REQUEST_TIMEOUT]				= "Request Timeout",
		[HTTP_STATUS_CODE_CONFLICT]						= "Conflict",
		[HTTP_STATUS_CODE_GONE]							= "Gone",
		[HTTP_STATUS_CODE_LENGTH_REQUIRED]				= "Length Required",
		[HTTP_STATUS_CODE_PRECONDITION_FAILED]			= "Precondition Failed",
		[HTTP_STATUS_CODE_PAYLOAD_TOO_LARGE]			= "Payload Too Large",
		[HTTP_STATUS_CODE_URI_TOO_LONG]					= "URI Too Long",
		[HTTP_STATUS_CODE_UNSUPPORTED_MEDIA_TYPE]		= "Unsupported Media Type",
		[HTTP_STATUS_CODE_RANGE_NOT_SATISFIABLE]		= "Range Not Satisfiable",
		[HTTP_STATUS_CODE_EXPECTATION_FAILED]			= "Expectation Failed",
		[HTTP_STATUS_CODE_MISDIRECTED_REQUEST]			= "Misdirected Request",
		[HTTP_STATUS_CODE_UNPROCESSABLE_ENTITY]			= "Unprocessable Entity",
		[HTTP_STATUS_CODE_LOCKED]						= "Locked",
		[HTTP_STATUS_CODE_FAILED_DEPENDENCY]			= "Failed Dependency",
		[HTTP_STATUS_CODE_UPGRADE_REQUIRED]				= "Upgrade Required",
		[HTTP_STATUS_CODE_PRECONDITION_REQUIRED]		= "Precondition Required",
		[HTTP_STATUS_CODE_TOO_MANY_REQUESTS]			= "Too Many Requests",
		[HTTP_STATUS_CODE_REQUEST_HEADER_FIELDS_TOO_LARGE] = "Request Header Fields Too Large",
		[HTTP_STATUS_CODE_UNAVAILABLE_FOR_LEGAL_REASONS] = "Unavailable For Legal Reasons",
		[HTTP_STATUS_CODE_INTERNAL_SERVER_ERROR]		= "Internal Server Error",
		[HTTP_STATUS_CODE_NOT_IMPLEMENTED]				= "Not Implemented",
		[HTTP_STATUS_CODE_BAD_GATEWAY]					= "Bad Gateway",
		[HTTP_STATUS_CODE_SERVICE_UNAVAILABLE]			= "Service Unavailable",
		[HTTP_STATUS_CODE_GATEWAY_TIMEOUT]				= "Gateway Timeout",
		[HTTP_STATUS_CODE_HTTP_VERSION_NOT_SUPPORTED]	= "HTTP Version Not Supported",
		[HTTP_STATUS_CODE_VARIANT_ALSO_NEGOTIATES]		= "Variant Also Negotiates",
		[HTTP_STATUS_CODE_INSUFFICIENT_STORAGE]			= "Insufficient Storage",
		[HTTP_STATUS_CODE_LOOP_DETECTED]				= "Loop Detected",
		[HTTP_STATUS_CODE_NOT_EXTENDED]					= "Not Extended",
		[HTTP_STATUS_CODE_NETWORK_AUTHENTICATION_REQUIRED] = "Network Authentication Required",
};
