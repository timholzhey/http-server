//
// Created by Tim Holzhey on 03.12.22.
//

#include "http_server.h"
#include "json.h"

HTTP_SERVER(app);

HTTP_ROUTE(get_route, {
	response.html("<h1>This is a GET route!</h1>");
	response.append(request.param("myparam"));
})

HTTP_ROUTE_METHOD(post_route, HTTP_METHOD_POST, {
	response.text("This is a POST route!\njson[myparam]=");
	char *body = request.body();
	json_parse_string(body, obj);
	json_value_t *val = json_object_get_value(&obj, "myparam");
	response.append(val ? val->string : "null");
})

WEBSOCKET_ROUTE(ws_route, {
	switch (websocket.event) {
		case WEBSOCKET_EVENT_CONNECTED:
			log_debug("Websocket: connected");
			break;
		case WEBSOCKET_EVENT_DISCONNECTED:
			log_debug("Websocket: disconnected");
			break;
		case WEBSOCKET_EVENT_DATA:
			log_debug("Websocket: data [%s]", websocket.data);
			websocket.send("Hello from the server!");
			break;
	}
})

int main() {
	app.route("get_route", get_route);
	app.route("post_route", post_route);
	app.websocket("ws_route", ws_route);
	app.serve_static("static");
	app.run();
	return 0;
}
