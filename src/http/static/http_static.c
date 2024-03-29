//
// Created by Tim Holzhey on 04.12.22.
//

#include <dirent.h>
#include <errno.h>
#include "http_static.h"
#include "http_status.h"
#include <stdint.h>
#include <unistd.h>
#include <stdlib.h>
#include <ctype.h>
#include <http_mime.h>

#define HTTP_STATIC_MAX_PATH_SIZE			1024

#ifndef HTTP_SERVER_ROOT_DIR
#warning "HTTP_SERVER_ROOT_DIR not defined, using default"
#define HTTP_SERVER_ROOT_DIR				"/var/www/"
#endif

static char http_static_path[HTTP_STATIC_MAX_PATH_SIZE];

ret_code_t http_static_init(const char *path) {
	uint32_t path_length = strlen(path);
	if (path_length > HTTP_STATIC_MAX_PATH_SIZE) {
		return RET_CODE_ERROR;
	}

	strcpy(http_static_path, HTTP_SERVER_ROOT_DIR);
	if (http_static_path[path_length - 1] != '/') {
		strcat(http_static_path, "/");
	}
	strcat(http_static_path, path);

	DIR *dir = opendir(http_static_path);
	if (dir) {
		closedir(dir);
	} else if (ENOENT == errno) {
		log_error("Directory %s does not exist", http_static_path);
		return RET_CODE_ERROR;
	} else {
		log_error("Error opening directory %s", http_static_path);
		return RET_CODE_ERROR;
	}

	return RET_CODE_OK;
}

ret_code_t http_static_route(http_request_t *p_request, http_response_t *p_response) {
	char path[HTTP_STATIC_MAX_PATH_SIZE];
	strcpy(path, http_static_path);
	strcat(path, p_request->uri);

	// check for path traversal and reject
	if (strstr(path, "..") != NULL) {
		p_response->status_code = HTTP_STATUS_CODE_FORBIDDEN;
		return RET_CODE_OK;
	}

	if (strcmp(p_request->uri, "/") == 0) {
		strcat(path, "index.html");
	}

	char *extension = strrchr(path, '.');
	if (extension == NULL) {
		p_response->status_code = HTTP_STATUS_CODE_NOT_FOUND;
		return RET_CODE_OK;
	}
	for (char *c = extension; *c != '\0'; c++) {
		*c = tolower(*c);
	}
	const char *content_type = NULL;
	if (http_mime_get(extension, &content_type) != RET_CODE_OK) {
		http_headers_set_value_string(p_response->headers, &p_response->num_headers, "Content-Type", "text/plain");
	} else {
		http_headers_set_value_string(p_response->headers, &p_response->num_headers, "Content-Type", content_type);
	}

	if (access(path, F_OK) != 0) {
		return RET_CODE_ERROR;
	}

	DIR *dir = opendir(path);
	if (dir) {
		closedir(dir);
		return RET_CODE_ERROR;
	}

	FILE *file = fopen(path, "r");
	if (file == NULL) {
		log_error("Error opening file %s", path);
		return RET_CODE_ERROR;
	}

	fseek(file, 0, SEEK_END);
	uint32_t file_size = ftell(file);
	fseek(file, 0, SEEK_SET);

	if (file_size > HTTP_RESPONSE_MAX_STATIC_PAYLOAD_SIZE) {
		if (file_size > HTTP_RESPONSE_MAX_DYNAMIC_PAYLOAD_SIZE) {
			log_error("File %s is too large", path);
			fclose(file);
			return RET_CODE_ERROR;
		}
		p_response->dynamic_payload = malloc(file_size);
		if (p_response->dynamic_payload == NULL) {
			log_error("Error allocating memory for file %s", path);
			fclose(file);
			return RET_CODE_ERROR;
		}
		p_response->dynamic_payload_allocated = true;
		p_response->payload_length = fread(p_response->dynamic_payload, 1, file_size, file);
	} else {
		p_response->payload_length = fread(p_response->payload, 1, file_size, file);
	}

	p_response->status_code = HTTP_STATUS_CODE_OK;

	http_headers_set_value_numeric(p_response->headers, &p_response->num_headers, "Content-Length", p_response->payload_length);

	fclose(file);

	return RET_CODE_OK;
}
