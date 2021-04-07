#ifndef HTTP_REQUEST_H
#define HTTP_REQUEST_H

#include "app_config.h"
#include "const.h"
#include <ArduinoHttpClient.h>
#include "gsm.h"

class HttpRequest
{
public:
	HttpRequest(TinyGsm *modem, const char *server);
	RetResult get(const char *path, char *resp_buff, int resp_buff_size);
	RetResult post(const char *path, const unsigned char *body, int body_len, char *content_type, 
		char *resp_buff, int resp_buff_size);

	uint16_t get_response_code();
	int get_response_length();

	RetResult set_port(int port);
private:
	enum Method
	{
		METHOD_GET,
		METHOD_POST
	};

	RetResult req(Method method, const char *path, char *resp_buff, int resp_buff_size,
		const unsigned char *body, int body_len, char *content_type);

	int _port = 80;
	char *_server = NULL;
	TinyGsm *_modem;
	uint16_t _response_code = 0;
	int _response_length = 0;
};

#endif