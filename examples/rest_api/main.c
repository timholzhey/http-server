//
// Created by tholz on 19.06.2023.
//

#include "http_server.h"
#include "json.h"

/**
 * @brief This example is a REST API. It allows you to get the weather for a city.
 */

HTTP_SERVER(app);

HTTP_ROUTE_METHOD("weather", weather, HTTP_METHOD_POST) {
	const char *body = request.body();
	json_object_t obj = {0};

	if (json_parse(body, strlen(body), &obj) != RET_CODE_OK) {
		response.status(HTTP_STATUS_CODE_BAD_REQUEST);
		return;
	}

	json_object_member_t *city_member = json_object_get_member(&obj, "city");
	if (city_member == NULL || city_member->type != JSON_VALUE_TYPE_STRING) {
		response.status(HTTP_STATUS_CODE_BAD_REQUEST);
		return;
	}

	char *city = city_member->value.string;
	response.append_text("The weather in ");
	response.append_text(city);
	response.append_text(" is sunny.");
}

int main() {
	app.route(weather);
	app.serve_static("static");
	app.run();
	return 0;
}
