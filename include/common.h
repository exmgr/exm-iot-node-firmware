#ifndef COMMON_H
#define COMMON_H
#include "app_config.h"
#include "utils.h"

#if WIFI_DEBUG_CONSOLE
#include "wifi_serial.h"
#endif

#if DEBUG || RELEASE
    #if WIFI_DEBUG_CONSOLE
        #define SERIAL_OBJECT WifiDebugSerial        
    #else
        #define SERIAL_OBJECT Serial
    #endif

    #define debug_print(msg, ...); { SERIAL_OBJECT.print(msg, ##__VA_ARGS__); }
    #define debug_print_e(msg, ...); { Serial.print(DEBUG_LEVEL_ERROR_STYLE); SERIAL_OBJECT.print(msg, ##__VA_ARGS__); Utils::serial_style(STYLE_RESET); }
    #define debug_print_w(msg, ...); { Serial.print(DEBUG_LEVEL_WARNING_STYLE); SERIAL_OBJECT.print(msg, ##__VA_ARGS__); Utils::serial_style(STYLE_RESET); }
    #define debug_print_i(msg, ...); { Serial.print(DEBUG_LEVEL_INFO_STYLE); SERIAL_OBJECT.print(msg, ##__VA_ARGS__); Utils::serial_style(STYLE_RESET); }

    // #define debug_println(); SERIAL_OBJECT.println();
    #define debug_println(msg, ...); { SERIAL_OBJECT.println(msg, ##__VA_ARGS__); }
    #define debug_println_e(msg, ...); { Serial.print(DEBUG_LEVEL_ERROR_STYLE); SERIAL_OBJECT.println(msg, ##__VA_ARGS__); Utils::serial_style(STYLE_RESET); }
    #define debug_println_w(msg, ...); { Serial.print(DEBUG_LEVEL_WARNING_STYLE); SERIAL_OBJECT.println(msg, ##__VA_ARGS__); Utils::serial_style(STYLE_RESET); }
    #define debug_println_i(msg, ...); { Serial.print(DEBUG_LEVEL_INFO_STYLE); SERIAL_OBJECT.println(msg, ##__VA_ARGS__); Utils::serial_style(STYLE_RESET); }

    #define debug_printf(format, ...); { SERIAL_OBJECT.printf(format, ##__VA_ARGS__); }
    #define debug_printf_e(format, ...); { Serial.print(DEBUG_LEVEL_ERROR_STYLE); SERIAL_OBJECT.printf(format, ##__VA_ARGS__); Utils::serial_style(STYLE_RESET); }
    #define debug_printf_w(format, ...); { Serial.print(DEBUG_LEVEL_WARNING_STYLE); SERIAL_OBJECTSerial.printf(format, ##__VA_ARGS__); Utils::serial_style(STYLE_RESET); }
    #define debug_printf_i(format, ...); { Serial.print(DEBUG_LEVEL_INFO_STYLE); SeSERIAL_OBJECTrial.printf(format, ##__VA_ARGS__); Utils::serial_style(STYLE_RESET); }
#else
    #define debug_print(msg, ...); {}
    #define debug_print_e(msg, ...); {}
    #define debug_print_w(msg, ...); {}
    #define debug_print_i(msg, ...); {}

    #define debug_println(); {}
    #define debug_println(msg, ...); {}
    #define debug_println_e(msg, ...); {}
    #define debug_println_w(msg, ...); {}
    #define debug_println_i(msg, ...); {}

    #define debug_printf(format, ...); {}
    #define debug_printf_e(format, ...); {}
    #define debug_printf_w(format, ...); {}
    #define debug_printf_i(format, ...); {}

#endif

#endif