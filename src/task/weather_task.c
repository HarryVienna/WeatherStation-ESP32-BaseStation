#include <string.h>
#include <sys/param.h>
#include <stdlib.h>
#include <ctype.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_system.h"
#include "esp_http_client.h"
#include "esp_tls.h"
#include "esp_crt_bundle.h"

#include "gui/gui.h"

#include "cJSON.h"

static const char *WEATHER_URL = "https://api.open-meteo.com/v1/forecast?latitude=48.2167&longitude=16.3&current=temperature_2m,relative_humidity_2m,is_day,weather_code,wind_speed_10m,wind_direction_10m,wind_gusts_10m&hourly=temperature_2m,precipitation_probability,rain,showers,snowfall,cloud_cover,wind_speed_10m,wind_gusts_10m&timezone=Europe%2FBerlin&forecast_days=14";

static const char* TAG = "weather_task";

extern SemaphoreHandle_t lvgl_mux;

typedef struct {
    char *buffer;
    int buffer_len;
} http_response_t;

typedef struct {
    struct tm time;           // Store the time in a struct tm
    double temperature_2m;    // Temperature in °C
    double relative_humidity_2m; // Relative humidity in %
    double precipitation_probability; // Precipitation probability in %
    double rain;             // Rain amount in mm
    double showers;          // Shower amount in mm
    double snowfall;         // Snowfall amount in cm
    int weather_code;        // WMO weather code
    double wind_speed_10m;   // Wind speed in km/h
    double wind_gusts_10m;   // Wind gusts in km/h
    double uv_index;         // UV index
    bool is_day;             // Boolean to indicate if it's day or night
    double sunshine_duration; // Sunshine duration in seconds
} hourly_weather_data_t;

esp_err_t _http_event_handler(esp_http_client_event_t *evt)
{
    http_response_t *response = (http_response_t *)evt->user_data;

    switch(evt->event_id) {
        case HTTP_EVENT_ERROR:
            ESP_LOGI(TAG, "HTTP_EVENT_ERROR");
            break;
        case HTTP_EVENT_ON_CONNECTED:
            ESP_LOGI(TAG, "HTTP_EVENT_ON_CONNECTED");
            break;
        case HTTP_EVENT_HEADER_SENT:
            ESP_LOGI(TAG, "HTTP_EVENT_HEADER_SENT");
            break;
        case HTTP_EVENT_ON_HEADER:
            ESP_LOGI(TAG, "HTTP_EVENT_ON_HEADER, key=%s, value=%s", evt->header_key, evt->header_value);
            break;
        case HTTP_EVENT_ON_DATA:
            ESP_LOGI(TAG, "HTTP_EVENT_ON_DATA, len=%d", evt->data_len);
            if (response->buffer == NULL) {
                response->buffer_len = evt->data_len;
                response->buffer = (char *)heap_caps_malloc(response->buffer_len + 1, MALLOC_CAP_8BIT | MALLOC_CAP_SPIRAM);
                memcpy(response->buffer, evt->data, evt->data_len);
            } else {
                response->buffer_len += evt->data_len;
                response->buffer = (char *)heap_caps_realloc(response->buffer, response->buffer_len + 1, MALLOC_CAP_8BIT | MALLOC_CAP_SPIRAM);
                memcpy(response->buffer + response->buffer_len - evt->data_len, evt->data, evt->data_len);
            }
            response->buffer[response->buffer_len] = 0; // Null-terminate the buffer
            break;
        case HTTP_EVENT_ON_FINISH:
            ESP_LOGI(TAG, "HTTP_EVENT_ON_FINISH");
            break;
        case HTTP_EVENT_DISCONNECTED:
            ESP_LOGI(TAG, "HTTP_EVENT_DISCONNECTED");
            break;
        case HTTP_EVENT_REDIRECT:
            ESP_LOGI(TAG, "HTTP_EVENT_DISCONNECTED");
            break;
    }
    return ESP_OK;
}

/**
 * @brief     Task for retrieving and displaying weather data from an API
 *
 * @param     pvParameter   Pointer to task parameters (not used in this function)
 *
 * @details   Retrieves weather data from the OpenWeatherMap API based on configured coordinates and API key.
 *            Periodically calls the API, parses the received JSON data, and displays weather information.
 */
void weather_task(void *pvParameter) {
    
    ESP_LOGI(TAG, "Start Weather task");
    
    http_response_t response = {0};

    esp_http_client_config_t config = {
        .url = WEATHER_URL,
        .event_handler = _http_event_handler,
        .crt_bundle_attach = esp_crt_bundle_attach,
        .user_data =  &response, // Pass the response buffer to the event handler
        .disable_auto_redirect = true,
    };

    esp_http_client_handle_t client = esp_http_client_init(&config);

    for (;;) {

        ESP_LOGI(TAG, "Call weather API ");

        esp_err_t err = esp_http_client_perform(client);
        if (err == ESP_OK) {
            ESP_LOGI(TAG, "HTTP GET Status = %d, content_length = %"PRId64,
                    esp_http_client_get_status_code(client),
                    esp_http_client_get_content_length(client));
            ESP_LOGI(TAG, "JSON %s", response.buffer);

            
            // Parse JSON response
            cJSON *json = cJSON_Parse(response.buffer);
            if (json == NULL) {
                const char *error_ptr = cJSON_GetErrorPtr();
                if (error_ptr != NULL) {
                    ESP_LOGE(TAG, "Error before: %s", error_ptr);
                }
            }
            else {

                time_t rawtime;
                struct tm timeinfo; // Note: no pointer here
                time(&rawtime);
                localtime_r(&rawtime, &timeinfo); // Pass &timeinfo as the result buffer

                int currentHour = timeinfo.tm_hour;

                cJSON *hourly = cJSON_GetObjectItem(json, "hourly");

                // Create an array to store HourlyWeatherData structs
                hourly_weather_data_t hourly_data[48];

                // Iterate through the "hourly" data array
                for (int i = currentHour; i < currentHour + 48; i++) {
                    cJSON *hour = cJSON_GetArrayItem(hourly, i); 

                    // Extract time as Unix timestamp
                    cJSON *timeItem = cJSON_GetObjectItem(hour, "time");
                    time_t unixTimestamp = timeItem->valueint;
                    localtime_r(&unixTimestamp, &hourly_data[i].time); 

                    hourly_data[i].temperature_2m = cJSON_GetObjectItem(hour, "temperature_2m")->valuedouble;
                    hourly_data[i].relative_humidity_2m = cJSON_GetObjectItem(hour, "relative_humidity_2m")->valuedouble;
                    hourly_data[i].precipitation_probability = cJSON_GetObjectItem(hour, "precipitation_probability")->valuedouble;
                    hourly_data[i].rain = cJSON_GetObjectItem(hour, "rain")->valuedouble;
                    hourly_data[i].showers = cJSON_GetObjectItem(hour, "showers")->valuedouble;
                    hourly_data[i].snowfall = cJSON_GetObjectItem(hour, "snowfall")->valuedouble;
                    hourly_data[i].weather_code = cJSON_GetObjectItem(hour, "weather_code")->valueint;
                    hourly_data[i].wind_speed_10m = cJSON_GetObjectItem(hour, "wind_speed_10m")->valuedouble;
                    hourly_data[i].wind_gusts_10m = cJSON_GetObjectItem(hour, "wind_gusts_10m")->valuedouble;
                    hourly_data[i].uv_index = cJSON_GetObjectItem(hour, "uv_index")->valuedouble;
                    hourly_data[i].is_day = cJSON_IsTrue(cJSON_GetObjectItem(hour, "is_day"));
                    hourly_data[i].sunshine_duration = cJSON_GetObjectItem(hour, "sunshine_duration")->valuedouble;
 
                }
            }
            
            cJSON_Delete(json);
            
            // Clean up
            if (response.buffer) {
                heap_caps_free(response.buffer);
                response.buffer = NULL;  // Reset the buffer pointer
                response.buffer_len = 0; // Reset the buffer length
            }

        } else {
            ESP_LOGE(TAG, "HTTP GET request failed: %s", esp_err_to_name(err));
        }

        esp_http_client_close(client);


        vTaskDelay(1000 * 60  / portTICK_PERIOD_MS); // Every 10 Minutes == 144 calls per day
    }
}