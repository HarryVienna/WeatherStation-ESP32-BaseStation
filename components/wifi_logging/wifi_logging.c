#include "wifi_logging.h"
#include "lwip/err.h"
#include "lwip/sockets.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include <stdarg.h> 
#include <stdio.h>

static int udp_socket;
static SemaphoreHandle_t socket_mutex;

// Maps esp_log_level_t to strings
static const char *level_strings[] = {
    "NONE", "ERROR", "WARN", "INFO", "DEBUG", "VERBOSE"
};

void init_wifi_logging() {
    udp_socket = socket(AF_INET, SOCK_DGRAM, 0);
    if (udp_socket < 0) {
        ESP_LOGE("WiFi Logging", "Error creating socket");
        return;
    }

    socket_mutex = xSemaphoreCreateMutex();
    if (socket_mutex == NULL) {
        ESP_LOGE("WiFi Logging", "Error creating mutex");
        close(udp_socket);
        return;
    }
}

void wifi_log_message(const char *tag, esp_log_level_t level, const char *func, int line, const char *format, ...) {

    if (level < WIFI_LOGGING_MIN_LEVEL) {
        return; // Filter out messages below the configured level
    }

    if (xSemaphoreTake(socket_mutex, portMAX_DELAY) == pdTRUE) {
        
         // Estimate initial size for prefix and format
        int initial_size = 256; // Arbitrary starting size
        char *final_message = (char *)malloc(initial_size);


        int prefix_len = snprintf(final_message, initial_size, "[%.20s][%.10s][%s:%d]: ", 
                                  tag, level_strings[level], func, line);

        va_list args;
        va_start(args, format);
        int msg_len = vsnprintf(NULL, 0, format, args); // Get required size for formatted string
        va_end(args);

        // Resize buffer if needed
        int total_len = prefix_len + msg_len + 2; // +1 for '\n', +1 for '\0'
        final_message = (char *)realloc(final_message, total_len);
        
        va_start(args, format);
        vsnprintf(final_message + prefix_len, msg_len + 1, format, args);
        va_end(args);

        // Append newline and null terminator
        final_message[prefix_len + msg_len] = '\n';
        final_message[prefix_len + msg_len + 1] = '\0';
        
        struct sockaddr_in dest_addr;
        dest_addr.sin_addr.s_addr = inet_addr(WIFI_LOGGING_SERVER_IP);
        dest_addr.sin_family = AF_INET; 
        dest_addr.sin_port = htons(WIFI_LOGGING_SERVER_PORT);

        int err = sendto(udp_socket, final_message, strlen(final_message), 0, 
                         (struct sockaddr *)&dest_addr, sizeof(dest_addr));
        if (err < 0) {
            ESP_LOGE("WiFi Logging", "Error occurred during sending: errno %d", errno);
        }

        free(final_message); // Free allocated memory
        
        xSemaphoreGive(socket_mutex);

    } else {
        ESP_LOGE("WiFi Logging", "Failed to take mutex");
    }
}