/**
 * @file wifi_logging.h
 * @brief Provides a WiFi logging component for ESP32.
 *
 * This component allows you to send log messages from your ESP32 application 
 * over WiFi using UDP. It maintains a persistent UDP socket connection and 
 * handles synchronization between multiple tasks using a mutex. 
 *
 * @note  
 *    * Ensure that `WIFI_LOGGING_SERVER_IP` and `WIFI_LOGGING_SERVER_PORT` are correctly configured.
 *    * On your Linux server, you need to run a UDP listener to receive the logs. 
 *      A simple way to do this is using the `netcat` command:
 *
 *      ```bash
 *      nc -u -l <port_number>
 *      ```
 *
 *      Replace `<port_number>` with the same port specified in `WIFI_LOGGING_SERVER_PORT`.
 */

#ifndef __WIFI_LOGGING_H__
#define __WIFI_LOGGING_H__

#include "esp_log.h"

#ifdef __cplusplus
extern "C" {
#endif

// Configuration macros
#define WIFI_LOGGING_SERVER_IP "192.168.0.150"
#define WIFI_LOGGING_SERVER_PORT 5000
#define WIFI_LOGGING_MIN_LEVEL ESP_LOG_INFO  // Adjust to control log level

#define WIFI_LOGE(tag, format, ...)  wifi_log_message(tag, ESP_LOG_ERROR, __func__, __LINE__, format, ##__VA_ARGS__)
#define WIFI_LOGW(tag, format, ...)  wifi_log_message(tag, ESP_LOG_WARN,  __func__, __LINE__, format, ##__VA_ARGS__)
#define WIFI_LOGI(tag, format, ...)  wifi_log_message(tag, ESP_LOG_INFO,  __func__, __LINE__, format, ##__VA_ARGS__)
#define WIFI_LOGD(tag, format, ...)  wifi_log_message(tag, ESP_LOG_DEBUG, __func__, __LINE__, format, ##__VA_ARGS__)
#define WIFI_LOGV(tag, format, ...)  wifi_log_message(tag, ESP_LOG_VERBOSE, __func__, __LINE__, format, ##__VA_ARGS__)

/**
 * @brief Initializes the WiFi logging component.
 *
 * This function should be called once during setup, preferably before creating
 * any tasks that will use the logger.
 */
void init_wifi_logging();

/**
 * @brief Sends a log message over WiFi.
 *
 * @param tag A string tag to identify the source of the log message.
 * @param level The log level (e.g., ESP_LOG_ERROR, ESP_LOG_INFO).
 * @param func The name of the function where the log message is generated (use `__func__`).
 * @param line The line number where the log message is generated (use `__LINE__`).
 * @param format A format string, similar to those used in `printf`.
 * @param ... Variable arguments to be formatted according to the `format` string.
 */
void wifi_log_message(const char *tag, esp_log_level_t level, const char *func, int line, const char *format, ...);

#ifdef __cplusplus
}
#endif

#endif