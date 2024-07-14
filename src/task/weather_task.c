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

static const char *WEATHER_URL_BASE = "https://api.open-meteo.com/v1/forecast";
static const char *WEATHER_URL_CURRENT = "https://api.open-meteo.com/v1/forecast?latitude=48.2144&longitude=16.3234&current=temperature_2m,relative_humidity_2m,apparent_temperature,is_day,weather_code,cloud_cover,wind_speed_10m,wind_direction_10m,wind_gusts_10m,uv_index&timeformat=unixtime&timezone=auto";
static const char *WEATHER_URL_HOURLY = "https://api.open-meteo.com/v1/forecast?latitude=48.2167&longitude=16.3&hourly=temperature_2m,precipitation_probability,rain,showers,snowfall,wind_speed_10m,wind_gusts_10m,sunshine_duration,cloud_cover,is_day&timeformat=unixtime&timezone=auto&forecast_days=3";
static const char *WEATHER_URL_DAILY  = "https://api.open-meteo.com/v1/forecast?latitude=48.2167&longitude=16.3&daily=temperature_2m_max,temperature_2m_min,daylight_duration,sunshine_duration,rain_sum,showers_sum,snowfall_sum,precipitation_probability_max,wind_speed_10m_max,wind_gusts_10m_max&timeformat=unixtime&timezone=auto";

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

    esp_err_t err;

    current_weather_data_t current_data;
    hourly_weather_data_t hourly_data[48];
    daily_weather_data_t daily_data[7];
    
    http_response_t response = {0};

    esp_http_client_config_t config = {
        .event_handler = _http_event_handler,
        .url = WEATHER_URL_BASE,
        .crt_bundle_attach = esp_crt_bundle_attach,
        .user_data =  &response, // Pass the response buffer to the event handler
        .disable_auto_redirect = true,
    };

    esp_http_client_handle_t client = esp_http_client_init(&config);

    for (;;) {
        // ------- Hourly data -------
        ESP_LOGI(TAG, "Call current weather API ");

        esp_http_client_set_url(client, WEATHER_URL_CURRENT);

        err = esp_http_client_perform(client);
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
                cJSON *current = cJSON_GetObjectItem(json, "current");

                current_data.temperature_2m = cJSON_GetObjectItem(current, "temperature_2m")->valuedouble;
                current_data.relative_humidity_2m = cJSON_GetObjectItem(current, "relative_humidity_2m")->valueint;
                current_data.apparent_temperature = cJSON_GetObjectItem(current, "apparent_temperature")->valuedouble;
                current_data.is_day = cJSON_GetObjectItem(current, "is_day")->valueint;
                current_data.weather_code = cJSON_GetObjectItem(current, "weather_code")->valueint;
                current_data.cloud_cover = cJSON_GetObjectItem(current, "cloud_cover")->valueint;
                current_data.wind_speed_10m = cJSON_GetObjectItem(current, "wind_speed_10m")->valuedouble;
                current_data.wind_direction_10m = cJSON_GetObjectItem(current, "wind_direction_10m")->valueint;
                current_data.wind_gusts_10m = cJSON_GetObjectItem(current, "wind_gusts_10m")->valuedouble;
                current_data.uv_index = cJSON_GetObjectItem(current, "uv_index")->valuedouble;

                xSemaphoreTakeRecursive(lvgl_mux, portMAX_DELAY);
                disp_current_weather(&current_data);
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


        // ------- Hourly data -------
        ESP_LOGI(TAG, "Call hourly weather API ");

        esp_http_client_set_url(client, WEATHER_URL_HOURLY);

        err = esp_http_client_perform(client);
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
                cJSON *precipitation_probability = cJSON_GetObjectItem(hourly, "precipitation_probability");
                cJSON *rain = cJSON_GetObjectItem(hourly, "rain");
                cJSON *showers = cJSON_GetObjectItem(hourly, "showers");
                cJSON *snowfall = cJSON_GetObjectItem(hourly, "snowfall");
                cJSON *wind_speed_10m = cJSON_GetObjectItem(hourly, "wind_speed_10m");
                cJSON *wind_gusts_10m = cJSON_GetObjectItem(hourly, "wind_gusts_10m");
                cJSON *sunshine_duration = cJSON_GetObjectItem(hourly, "sunshine_duration");
                cJSON *cloud_cover = cJSON_GetObjectItem(hourly, "cloud_cover");
                cJSON *is_day = cJSON_GetObjectItem(hourly, "is_day");

                // Iterate through the "hourly" data array
                for (int i = 0; i < 48; i++) {

                    time_t unixTimestamp = (time_t)cJSON_GetArrayItem(time, i + currentHour)->valueint;
                    localtime_r(&unixTimestamp, &hourly_data[i].time); 
                    hourly_data[i].temperature_2m = cJSON_GetArrayItem(temperature_2m, i + currentHour)->valuedouble;
                    hourly_data[i].precipitation_probability = cJSON_GetArrayItem(precipitation_probability, i + currentHour)->valuedouble;
                    hourly_data[i].rain = cJSON_GetArrayItem(rain, i + currentHour)->valuedouble;
                    hourly_data[i].showers = cJSON_GetArrayItem(showers, i + currentHour)->valuedouble;
                    hourly_data[i].snowfall = cJSON_GetArrayItem(snowfall, i + currentHour)->valuedouble;
                    hourly_data[i].wind_speed_10m = cJSON_GetArrayItem(wind_speed_10m, i + currentHour)->valuedouble;
                    hourly_data[i].wind_gusts_10m = cJSON_GetArrayItem(wind_gusts_10m, i + currentHour)->valuedouble;
                    hourly_data[i].sunshine_duration = cJSON_GetArrayItem(sunshine_duration, i + currentHour)->valuedouble;
                    hourly_data[i].cloud_cover = cJSON_GetArrayItem(cloud_cover, i + currentHour)->valuedouble;
                    hourly_data[i].is_day = cJSON_GetArrayItem(is_day, i + currentHour)->valueint;
                }

                xSemaphoreTakeRecursive(lvgl_mux, portMAX_DELAY);
                disp_hourly_weather(hourly_data);
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

        // ------- Daily data -------
        ESP_LOGI(TAG, "Call daily weather API ");

        esp_http_client_set_url(client, WEATHER_URL_DAILY);

        err = esp_http_client_perform(client);
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
                cJSON *daily = cJSON_GetObjectItem(json, "daily");

                cJSON *time = cJSON_GetObjectItem(daily, "time");
                cJSON *temperature_2m_max = cJSON_GetObjectItem(daily, "temperature_2m_max");
                cJSON *temperature_2m_min = cJSON_GetObjectItem(daily, "temperature_2m_min");
                cJSON *daylight_duration  = cJSON_GetObjectItem(daily, "daylight_duration");
                cJSON *sunshine_duration  = cJSON_GetObjectItem(daily, "sunshine_duration");
                cJSON *rain_sum  = cJSON_GetObjectItem(daily, "rain_sum");
                cJSON *showers_sum  = cJSON_GetObjectItem(daily, "showers_sum");
                cJSON *snowfall_sum  = cJSON_GetObjectItem(daily, "snowfall_sum");
                cJSON *precipitation_probability_max  = cJSON_GetObjectItem(daily, "precipitation_probability_max");
                cJSON *wind_speed_10m_max   = cJSON_GetObjectItem(daily, "wind_speed_10m_max");
                cJSON *wind_gusts_10m_max   = cJSON_GetObjectItem(daily, "wind_gusts_10m_max");

                // Iterate through the "daily" data array
                for (int i = 0; i < 7; i++) {

                    time_t unixTimestamp = (time_t)cJSON_GetArrayItem(time, i)->valueint;
                    localtime_r(&unixTimestamp, &daily_data[i].time); 
                    daily_data[i].temperature_2m_max = cJSON_GetArrayItem(temperature_2m_max, i)->valuedouble;
                    daily_data[i].temperature_2m_min = cJSON_GetArrayItem(temperature_2m_min, i)->valuedouble;
                    daily_data[i].daylight_duration = cJSON_GetArrayItem(daylight_duration, i)->valuedouble;
                    daily_data[i].sunshine_duration = cJSON_GetArrayItem(sunshine_duration, i)->valuedouble;
                    daily_data[i].rain_sum = cJSON_GetArrayItem(rain_sum, i)->valuedouble;
                    daily_data[i].showers_sum = cJSON_GetArrayItem(showers_sum, i)->valuedouble;
                    daily_data[i].snowfall_sum = cJSON_GetArrayItem(snowfall_sum, i)->valuedouble;
                    daily_data[i].precipitation_probability_max = cJSON_GetArrayItem(precipitation_probability_max, i)->valuedouble;
                    daily_data[i].wind_speed_10m_max = cJSON_GetArrayItem(wind_speed_10m_max, i)->valuedouble;
                    daily_data[i].wind_gusts_10m_max = cJSON_GetArrayItem(wind_gusts_10m_max, i)->valuedouble;
                }

                xSemaphoreTakeRecursive(lvgl_mux, portMAX_DELAY);
                disp_daily_weather(daily_data);
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

        vTaskDelay(pdMS_TO_TICKS(1000 * 60 *15)); // Every 15 Minutes
    }
}