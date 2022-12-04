//
// Created by Tim Holzhey on 03.12.22.
//

#include "http_server.h"

HTTP_SERVER(app);

HTTP_ROUTE(myroute, {
	response.html("<h1>This is a route!</h1>");
	response.append(request.param("myparam"));
})

int main() {
	app.route("route", myroute);
	app.serve_static("static");
	app.run();
	return 0;
}
