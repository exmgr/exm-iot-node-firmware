#include "http_request.h"
#include "common.h"
#include "wifi_modem.h"

// TODO: Comment everything

/******************************************************************************
* Constructor
* @param modem TinyGsm object
* @param server Host address
******************************************************************************/
HttpRequest::HttpRequest(TinyGsm *modem, const char *server)
{
	_modem = modem;
	_server = (char*)server;
}

/******************************************************************************
* Execute GET request
* @param path URL path
* @param resp_buff Buffer for response. Can be NULL
* @param resp_buff_size Response buffer size. 0 if no buffer
******************************************************************************/
RetResult HttpRequest::get(const char *path, char *resp_buff, int resp_buff_size)
{
	return req(METHOD_GET, path, resp_buff, resp_buff_size, NULL, 0, NULL);
}

/******************************************************************************
* Execute POST request
* @param path URL path
* @param body Request body
* @param body_len Body length
* @param content_type Content-type header
* @param resp_buff Buffer for response. Can be NULL
* @param resp_buff_size Response buffer size. 0 if no buffer
******************************************************************************/
RetResult HttpRequest::post(const char *path, const unsigned char *body, int body_len, char *content_type, 
	char *resp_buff, int resp_buff_size)
{
	return req(METHOD_POST, path, resp_buff, resp_buff_size, body, body_len, content_type);
}

/******************************************************************************
* Execute a request
******************************************************************************/
RetResult HttpRequest::req(Method method, const char *path, char *resp_buff, int resp_buff_size,
	const unsigned char *body, int body_len, char *content_type)
{
	// Use WiFi client in WiFi mode
	#if WIFI_DATA_SUBMISSION
		WiFiClient client;
	#else
		TinyGsmClient client(*_modem);
	#endif

    HttpClient http_client(client, _server, _port);

	debug_print(F("Request to: "));
	debug_print(_server);
	debug_println(path);
	debug_print(F("Port: "));
	debug_println(_port, DEC);

	if(!GSM::is_gprs_connected())
    {
        debug_println(F("GPRS is not connected, request aborted."));
        return RET_ERROR;
    }

	http_client.setTimeout(HTTP_CLIENT_STREAM_TIMEOUT);
	http_client.setHttpResponseTimeout(HTTL_CLIENT_REPONSE_TIMEOUT);

	int ret = 0;

	if(method == METHOD_GET)
	{
		ret = http_client.get(path);
	}
	else if(method == METHOD_POST)
	{
		ret = http_client.post(path, content_type, body_len, body);
	}

    if(ret != 0)
    {
        debug_print(F("Could not execute request. Error: "));
        debug_println(ret, DEC);
        return RET_ERROR;
    }

    _response_code = http_client.responseStatusCode();
    debug_print(F("Response code: "));
    debug_println(_response_code, DEC);
    if(!_response_code)
    {
        debug_println(F("Could not get response code."));
        return RET_ERROR;
    }

    int content_length = http_client.contentLength();

    debug_print(F("Content length: "));
    debug_println(content_length, DEC);

	int bytes_read = 0;
	if(resp_buff != NULL && resp_buff_size > 1)
	{   
		// If content length header set, read up to this amount of bytes or until buffer is full
		// If header not set, read until stream has no more bytes or buffer is full
		int bytes_to_read = content_length > 0 && content_length < resp_buff_size ? content_length : resp_buff_size;

		bytes_read = http_client.readBytes(resp_buff, bytes_to_read);

		if(content_length > 0 && bytes_read != bytes_to_read)
		{
			debug_print(F("Partial response read. Should be: "));
			debug_print(bytes_to_read, DEC);
			debug_print(F(" - Read: "));
			debug_println(bytes_read);
		}

		if(bytes_read == resp_buff_size)
		{
			resp_buff[resp_buff_size - 1] = '\0';
		}
		else
		{
			resp_buff[bytes_read] = '\0';
		}
	}

    http_client.stop();

    _response_length = bytes_read;
    
    return RET_OK;
}

/******************************************************************************
* Get response code after request has been executed
******************************************************************************/
uint16_t HttpRequest::get_response_code()
{
	return _response_code;
}

/******************************************************************************
* Response content-length after request has been executed
******************************************************************************/
int HttpRequest::get_response_length()
{
	return _response_length;
}

/******************************************************************************
* Port to use for request
******************************************************************************/
RetResult HttpRequest::set_port(int port)
{
	_port = port;

	return RET_OK;
}