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

#include "weather_task.h"
#include "weather/open_meteo.h"

#include "cJSON.h"

static const char *WEATHER_URL_HOURLY = "https://api.open-meteo.com/v1/forecast?latitude=48.2167&longitude=16.3&hourly=temperature_2m,relative_humidity_2m,precipitation_probability,rain,showers,snowfall,weather_code,wind_speed_10m,wind_gusts_10m,uv_index,is_day,sunshine_duration&timeformat=unixtime&timezone=auto&forecast_days=3";

static const char* TAG = "weather_task";

extern SemaphoreHandle_t lvgl_mux;

typedef struct {
    char *buffer;
    int buffer_len;
} http_response_t;



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

    // Create an array to store HourlyWeatherData structs
    hourly_weather_data_t hourly_data[48];
    
    http_response_t response = {0};

    esp_http_client_config_t config = {
        .url = WEATHER_URL_HOURLY,
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
            //ESP_LOGI(TAG, "JSON %s", response.buffer);

            
            // Parse JSON response
            cJSON *json = cJSON_Parse(response.buffer);
            if (json == NULL) {
                const char *error_ptr = cJSON_GetErrorPtr();
                if (error_ptr != NULL) {
                    ESP_LOGE(TAG, "Error before: %s", error_ptr);
                }
            }
            else {
                struct tm timeinfo;
                time_t now;
                time(&now);
                localtime_r(&now, &timeinfo);

                int currentHour = timeinfo.tm_hour;

                cJSON *hourly = cJSON_GetObjectItem(json, "hourly");
                cJSON *time = cJSON_GetObjectItem(hourly, "time");
                cJSON *temperature_2m = cJSON_GetObjectItem(hourly, "temperature_2m");
                cJSON *relative_humidity_2m = cJSON_GetObjectItem(hourly, "relative_humidity_2m");
                cJSON *precipitation_probability = cJSON_GetObjectItem(hourly, "precipitation_probability");
                cJSON *rain = cJSON_GetObjectItem(hourly, "rain");
                cJSON *showers = cJSON_GetObjectItem(hourly, "showers");
                cJSON *snowfall = cJSON_GetObjectItem(hourly, "snowfall");
                cJSON *weather_code = cJSON_GetObjectItem(hourly, "weather_code");
                cJSON *wind_speed_10m = cJSON_GetObjectItem(hourly, "wind_speed_10m");
                cJSON *wind_gusts_10m = cJSON_GetObjectItem(hourly, "wind_gusts_10m");
                cJSON *uv_index = cJSON_GetObjectItem(hourly, "uv_index");
                cJSON *is_day = cJSON_GetObjectItem(hourly, "is_day");
                cJSON *sunshine_duration = cJSON_GetObjectItem(hourly, "sunshine_duration");

                // Iterate through the "hourly" data array
                for (int i = 0; i < 48; i++) {

                    time_t unixTimestamp = (time_t)cJSON_GetArrayItem(time, i + currentHour)->valueint;
                    localtime_r(&unixTimestamp, &hourly_data[i].time); 
                    hourly_data[i].temperature_2m = cJSON_GetArrayItem(temperature_2m, i + currentHour)->valuedouble;
                    hourly_data[i].relative_humidity_2m = cJSON_GetArrayItem(relative_humidity_2m, i + currentHour)->valuedouble;
                    hourly_data[i].precipitation_probability = cJSON_GetArrayItem(precipitation_probability, i + currentHour)->valuedouble;
                    hourly_data[i].rain = cJSON_GetArrayItem(rain, i + currentHour)->valuedouble;
                    hourly_data[i].showers = cJSON_GetArrayItem(showers, i + currentHour)->valuedouble;
                    hourly_data[i].snowfall = cJSON_GetArrayItem(snowfall, i + currentHour)->valuedouble;
                    hourly_data[i].weather_code = cJSON_GetArrayItem(weather_code, i + currentHour)->valueint;
                    hourly_data[i].wind_speed_10m = cJSON_GetArrayItem(wind_speed_10m, i + currentHour)->valuedouble;
                    hourly_data[i].wind_gusts_10m = cJSON_GetArrayItem(wind_gusts_10m, i + currentHour)->valuedouble;
                    hourly_data[i].uv_index = cJSON_GetArrayItem(uv_index, i + currentHour)->valuedouble;
                    hourly_data[i].is_day = cJSON_GetArrayItem(is_day, i + currentHour)->valueint;
                    hourly_data[i].sunshine_duration = cJSON_GetArrayItem(sunshine_duration, i + currentHour)->valuedouble;

                    // ESP_LOGI(TAG, "Time: %04d-%02d-%02dT%02d:%02d, Temp: %.1f°C, Humidity: %.1f%%, Precip Prob: %.1f%%, Rain: %.2fmm, Showers: %.2fmm, Snow: %.2fcm",
                    //     hourly_data[i].time.tm_year + 1900, hourly_data[i].time.tm_mon + 1, hourly_data[i].time.tm_mday,
                    //     hourly_data[i].time.tm_hour, hourly_data[i].time.tm_min, hourly_data[i].temperature_2m,
                    //     hourly_data[i].relative_humidity_2m, hourly_data[i].precipitation_probability,
                    //     hourly_data[i].rain, hourly_data[i].showers, hourly_data[i].snowfall);

                    // ESP_LOGI(TAG, "Code: %d, Wind Speed: %.1fkm/h, Gusts: %.1fkm/h, UV: %.2f, Is Day: %d, Sunshine: %.1fs",                       
                    //     hourly_data[i].weather_code, hourly_data[i].wind_speed_10m,
                    //     hourly_data[i].wind_gusts_10m, hourly_data[i].uv_index,
                    //     hourly_data[i].is_day, hourly_data[i].sunshine_duration);    
 

                }

                xSemaphoreTakeRecursive(lvgl_mux, portMAX_DELAY);
                disp_weather(hourly_data);
                xSemaphoreGiveRecursive(lvgl_mux);                
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