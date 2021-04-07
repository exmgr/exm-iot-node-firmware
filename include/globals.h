#ifndef GLOBALS_H
#define GLOBALS_H

#include "app_config.h"
#include "struct.h"
#include "const.h"

/**
 * Large global buffer to be used for receiving large responses to http reqs.
 * Global to avoid polluting the stack
 */
extern char g_resp_buffer[GLOBAL_HTTP_RESPONSE_BUFFER_LEN];

#endif