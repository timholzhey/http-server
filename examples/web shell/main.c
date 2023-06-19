//
// Created by tholz on 10.12.2022.
//

#include "http_server.h"

/**
 * @brief This example is a web shell. It allows you to execute commands on the server via a web interface.
 * @note Dangerous! Do not use in production!
 */

HTTP_SERVER(app);

WEBSOCKET_ROUTE("ws_shell", ws_shell) {
	switch (websocket.event) {
		case WEBSOCKET_EVENT_DATA: {
			log_info("Received data: %s", websocket.data);
			FILE *fp = popen((const char *) websocket.data, "r");
			if (fp == NULL) {
				log_error("Failed to run command");
				return;
			}
			char buffer[1024];
			while (fgets(buffer, sizeof(buffer), fp) != NULL) {
				websocket.text(buffer);
			}
			pclose(fp);
		}
			break;
		default:
			break;
	}
}

int main() {
	app.route(ws_shell);
	app.serve_static("static");
	app.run();
	return 0;
}
