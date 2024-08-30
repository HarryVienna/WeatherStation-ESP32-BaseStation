/**
 * @file wifi_logging.h
 * @brief Provides a WiFi logging component for ESP32.
 *
 * This component allows you to send log messages from your ESP32 application 
 * over WiFi using UDP. It maintains a persistent UDP socket connection and 
 * handles synchronization between multiple tasks using a mutex. 
 * 
 * Mostly written by ChatGPT and Gemini :-)
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
#define WIFI_LOGGING_MIN_LEVEL ESP_LOG_DEBUG  // Adjust to control log level

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

/**
 * @brief Sends a log message over WiFi using UDP.
 *
 * This function takes a formatted log message, acquires a mutex to ensure 
 * thread-safe access to the UDP socket, and then sends the message to the 
 * configured server. It handles potential errors during sending and releases 
 * the mutex afterwards. 
 *
 * @param log_message The formatted log message to be sent.
 * 
 * @note 
 *    * This function is intended to be called internally by `generate_log_message`.
 *    * It assumes that the UDP socket and mutex have been initialized by `wifi_logging_init`.
 *    * The server IP address and port are configured using the `WIFI_LOGGING_SERVER_IP` and 
 *      `WIFI_LOGGING_SERVER_PORT` macros.
 */
void wifi_send_message(const char* log_message);

/**
 * @brief Routes a formatted log message to WIFI logger and outputs it to the console.
 *
 * This function formats a log message using the provided format string and variable 
 * argument list, then sends the formatted message to a queue and prints it to the 
 * console. The function dynamically allocates memory for the formatted message based 
 * on its size, avoiding the need for a fixed buffer size.
 *
 * @param[in] fmt The format string, similar to the format used in `printf`.
 * @param[in] args A variable argument list containing the data to format.
 *
 * @return 
 *     - The number of characters printed to the console (as returned by `vprintf`).
 *     - In case of memory allocation failure, it returns `-1`.
 *
 * @note
 *     - The function allocates memory dynamically for the formatted message, so it is 
 *       important to ensure that the memory is properly freed after use.
 *     - If `malloc` fails to allocate memory, the function will return `-1` and the 
 *       log message will not be sent or printed.
 *     - The log message is sent to a queue using `send_to_queue`, so ensure that this 
 *       function is implemented and handles the message appropriately.
 * 
 * @warning
 *     - Ensure that the variable argument list (`args`) is valid and correctly 
 *       initialized before calling this function.
 *     - Improper format strings or invalid arguments may lead to undefined behavior.
 */
int wifi_system_send_message(const char* fmt, va_list tag);

#ifdef __cplusplus
}
#endif

#endif