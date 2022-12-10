//
// Created by Tim Holzhey on 03.12.22.
//

#include "http_server.h"

HTTP_SERVER(app);

int main() {
	app.serve_static("static");
	app.run();
	return 0;
}
