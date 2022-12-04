//
// Created by Tim Holzhey on 04.12.22.
//

#include <dirent.h>
#include <errno.h>
#include "http_static.h"
#include "http_status.h"
#include <stdint.h>
#include <unistd.h>

#define HTTP_STATIC_MAX_PATH_SIZE			1024

static char http_static_path[HTTP_STATIC_MAX_PATH_SIZE];

ret_code_t http_static_init(const char *path) {
	uint32_t path_length = strlen(path);
	if (path_length > HTTP_STATIC_MAX_PATH_SIZE) {
		return RET_CODE_ERROR;
	}

	strcpy(http_static_path, SOURCE_DIR);
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

	if (strcmp(p_request->uri, "/") == 0) {
		strcat(path, "index.html");
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

	if (file_size > HTTP_RESPONSE_MAX_PAYLOAD_SIZE) {
		log_error("File %s too large", path);
		return RET_CODE_ERROR;
	}

	fread(p_response->payload, 1, file_size, file);
	p_response->payload_length = file_size;
	p_response->status_code = HTTP_STATUS_CODE_OK;

	return RET_CODE_OK;
}
