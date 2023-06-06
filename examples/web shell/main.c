//
// Created by tholz on 10.12.2022.
//

#include "http_server.h"

/**
 * @brief This example is a web shell. It allows you to execute commands on the server via a web interface.
 * @note DO NOT USE THIS IN PRODUCTION! This is just an example.
 */

HTTP_SERVER(app);

WEBSOCKET_ROUTE(ws_shell, {
	switch (websocket.event) {
		case WEBSOCKET_EVENT_CONNECTED:
			log_debug("Websocket: connected");
			break;
		case WEBSOCKET_EVENT_DISCONNECTED:
			log_debug("Websocket: disconnected");
			break;
		case WEBSOCKET_EVENT_DATA:
			log_debug("Websocket: data [%s]", websocket.data);
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
			break;
	}
})

int main() {
	app.websocket("ws_shell", ws_shell);
	app.serve_static("static");
	app.run();
	return 0;
}
