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
#include "nvs/preferences.h"

#include "cJSON.h"

#include "../config/config.h"

#include "gui/gui.h"

#include "weather_task.h"
#include "weather/open_meteo.h"


static const char* TAG = "weather_task";


static const char *WEATHER_URL_BASE = "https://api.open-meteo.com/v1/forecast";

static const char *WEATHER_URL_CURRENT =
        "https://api.open-meteo.com/v1/forecast?"
        "latitude=%s&longitude=%s&"
        "current=temperature_2m,dew_point_2m,relative_humidity_2m,apparent_temperature,"
        "is_day,weather_code,cloud_cover,wind_speed_10m,wind_direction_10m,wind_gusts_10m,uv_index&"
        "timeformat=unixtime&timezone=auto";


static const char *WEATHER_URL_HOURLY =
        "https://api.open-meteo.com/v1/forecast?"
        "latitude=%s&longitude=%s&"
        "hourly=temperature_2m,dew_point_2m,precipitation_probability,rain,showers,snowfall,"
        "wind_speed_10m,wind_gusts_10m,sunshine_duration,cloud_cover,is_day&"
        "timeformat=unixtime&timezone=auto&forecast_days=3";

static const char *WEATHER_URL_DAILY =
        "https://api.open-meteo.com/v1/forecast?"
        "latitude=%s&longitude=%s&"
        "daily=temperature_2m_max,temperature_2m_min,"
        "daylight_duration,sunshine_duration,"
        "rain_sum,showers_sum,snowfall_sum,precipitation_probability_max,"
        "wind_speed_10m_max,wind_gusts_10m_max,sunrise,sunset&"
        "timeformat=unixtime&timezone=auto";

extern SemaphoreHandle_t lvgl_mux;

typedef struct {
    char *buffer;
    int buffer_len;
} http_response_t;



/**
 * @brief Validate latitude/longitude coordinate strings
 *
 * @param lat Latitude string (expected range: -90 to 90)
 * @param lon Longitude string (expected range: -180 to 180)
 * @return true if valid, false otherwise
 */
static bool validate_coordinates(const char* lat, const char* lon) {
    if (lat == NULL || lon == NULL || lat[0] == '\0' || lon[0] == '\0') {
        ESP_LOGE(TAG, "Coordinates are NULL or empty");
        return false;
    }

    // Convert to double and validate range
    char* endptr_lat;
    char* endptr_lon;
    double lat_val = strtod(lat, &endptr_lat);
    double lon_val = strtod(lon, &endptr_lon);

    // Check for conversion errors (endptr should point to '\0' if fully parsed)
    if (*endptr_lat != '\0' || *endptr_lon != '\0') {
        ESP_LOGE(TAG, "Invalid coordinate format: lat='%s', lon='%s'", lat, lon);
        return false;
    }

    // Validate ranges
    if (lat_val < -90.0 || lat_val > 90.0) {
        ESP_LOGE(TAG, "Latitude out of range: %f (must be -90 to 90)", lat_val);
        return false;
    }

    if (lon_val < -180.0 || lon_val > 180.0) {
        ESP_LOGE(TAG, "Longitude out of range: %f (must be -180 to 180)", lon_val);
        return false;
    }

    return true;
}

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
 * @details   Retrieves weather data from the Open-Meteo API based on configured coordinates.
 *            Makes three API calls to fetch current conditions, 48-hour forecast, and 7-day forecast.
 *            Uses HTTPS with certificate bundle for secure communication.
 */
void weather_task(void *pvParameter) {

    ESP_LOGI(TAG, "Start Weather task");

    nvs_handle_t nvs_handle;
    nvs_open("weatherstation", NVS_READONLY, &nvs_handle);

    char* latitude = get_string_from_nvs(nvs_handle, "latitude", "");
    char* longitude = get_string_from_nvs(nvs_handle, "longitude", "");

    nvs_close(nvs_handle);

    // Validate coordinates before proceeding
    if (!validate_coordinates(latitude, longitude)) {
        ESP_LOGE(TAG, "Invalid coordinates from NVS. Task cannot proceed.");
        // Free allocated strings
        free(latitude);
        free(longitude);
        vTaskDelete(NULL);  // Terminate task
        return;
    }

    ESP_LOGI(TAG, "Using coordinates: lat=%s, lon=%s", latitude, longitude);

    esp_err_t err;

    char url[512];

    current_weather_data_t current_data = {0};
    hourly_weather_data_t hourly_data[48] = {0};
    daily_weather_data_t daily_data[7] = {0};

    http_response_t response = {0};

    // Success flags for tracking if all API calls completed successfully
    bool current_success = false;
    bool hourly_success = false;
    bool daily_success = false;

    esp_http_client_config_t config = {
        .event_handler = _http_event_handler,
        .url = WEATHER_URL_BASE,
        .is_async = false,
        .crt_bundle_attach = esp_crt_bundle_attach,
        .user_data = &response, // Pass the response buffer to the event handler
        .disable_auto_redirect = true,
    };

    esp_http_client_handle_t client = esp_http_client_init(&config);

    for (;;) {
        // ------- Current data -------
        sprintf(url, WEATHER_URL_CURRENT, latitude, longitude);
        ESP_LOGI(TAG, "Call current weather API: %s", url);

        esp_http_client_set_url(client, url);

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

                // Check structural integrity
                if (current == NULL || !cJSON_IsObject(current)) {
                    ESP_LOGE(TAG, "Missing or invalid 'current' object in API response");
                    cJSON_Delete(json);
                    if (response.buffer) {
                        heap_caps_free(response.buffer);
                        response.buffer = NULL;
                        response.buffer_len = 0;
                    }
                    esp_http_client_close(client);
                    continue;  // Skip to next iteration of main loop
                }

                // Extract fields with strict validation - fail if ANY field is missing
                cJSON *temp_2m = cJSON_GetObjectItem(current, "temperature_2m");
                cJSON *dew_point = cJSON_GetObjectItem(current, "dew_point_2m");
                cJSON *humidity = cJSON_GetObjectItem(current, "relative_humidity_2m");
                cJSON *apparent_temp = cJSON_GetObjectItem(current, "apparent_temperature");
                cJSON *is_day_item = cJSON_GetObjectItem(current, "is_day");
                cJSON *weather_code_item = cJSON_GetObjectItem(current, "weather_code");
                cJSON *cloud_cover_item = cJSON_GetObjectItem(current, "cloud_cover");
                cJSON *wind_speed = cJSON_GetObjectItem(current, "wind_speed_10m");
                cJSON *wind_dir = cJSON_GetObjectItem(current, "wind_direction_10m");
                cJSON *wind_gusts = cJSON_GetObjectItem(current, "wind_gusts_10m");
                cJSON *uv = cJSON_GetObjectItem(current, "uv_index");

                // Validate all critical fields exist and are correct type
                if (!temp_2m || !cJSON_IsNumber(temp_2m) ||
                    !dew_point || !cJSON_IsNumber(dew_point) ||
                    !humidity || !cJSON_IsNumber(humidity) ||
                    !apparent_temp || !cJSON_IsNumber(apparent_temp) ||
                    !is_day_item || !cJSON_IsNumber(is_day_item) ||
                    !weather_code_item || !cJSON_IsNumber(weather_code_item) ||
                    !cloud_cover_item || !cJSON_IsNumber(cloud_cover_item) ||
                    !wind_speed || !cJSON_IsNumber(wind_speed) ||
                    !wind_dir || !cJSON_IsNumber(wind_dir) ||
                    !wind_gusts || !cJSON_IsNumber(wind_gusts) ||
                    !uv || !cJSON_IsNumber(uv)) {

                    ESP_LOGE(TAG, "Current weather data incomplete - missing required fields");
                    cJSON_Delete(json);
                    if (response.buffer) {
                        heap_caps_free(response.buffer);
                        response.buffer = NULL;
                        response.buffer_len = 0;
                    }
                    esp_http_client_close(client);
                    continue;
                }

                // All fields valid - extract data
                current_data.temperature_2m = temp_2m->valuedouble;
                current_data.dew_point_2m = dew_point->valuedouble;
                current_data.relative_humidity_2m = humidity->valueint;
                current_data.apparent_temperature = apparent_temp->valuedouble;
                current_data.is_day = is_day_item->valueint;
                current_data.weather_code = weather_code_item->valueint;
                current_data.cloud_cover = cloud_cover_item->valueint;
                current_data.wind_speed_10m = wind_speed->valuedouble;
                current_data.wind_direction_10m = wind_dir->valueint;
                current_data.wind_gusts_10m = wind_gusts->valuedouble;
                current_data.uv_index = uv->valuedouble;

                current_success = true;
                ESP_LOGI(TAG, "Current weather data successfully retrieved");
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

            // Clean up any existing buffer from previous successful calls
            if (response.buffer) {
                heap_caps_free(response.buffer);
                response.buffer = NULL;
                response.buffer_len = 0;
            }
        }

        esp_http_client_close(client);


        // ------- Hourly data -------
        sprintf(url, WEATHER_URL_HOURLY, latitude, longitude);
        ESP_LOGI(TAG, "Call hourly weather API: %s", url);

        esp_http_client_set_url(client, url);

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

                // Check structural integrity
                if (hourly == NULL || !cJSON_IsObject(hourly)) {
                    ESP_LOGE(TAG, "Missing or invalid 'hourly' object in API response");
                    cJSON_Delete(json);
                    if (response.buffer) {
                        heap_caps_free(response.buffer);
                        response.buffer = NULL;
                        response.buffer_len = 0;
                    }
                    esp_http_client_close(client);
                    continue;
                }

                // Get all array references
                cJSON *time = cJSON_GetObjectItem(hourly, "time");
                cJSON *temperature_2m = cJSON_GetObjectItem(hourly, "temperature_2m");
                cJSON *dew_point_2m = cJSON_GetObjectItem(hourly, "dew_point_2m");
                cJSON *precipitation_probability = cJSON_GetObjectItem(hourly, "precipitation_probability");
                cJSON *rain = cJSON_GetObjectItem(hourly, "rain");
                cJSON *showers = cJSON_GetObjectItem(hourly, "showers");
                cJSON *snowfall = cJSON_GetObjectItem(hourly, "snowfall");
                cJSON *wind_speed_10m = cJSON_GetObjectItem(hourly, "wind_speed_10m");
                cJSON *wind_gusts_10m = cJSON_GetObjectItem(hourly, "wind_gusts_10m");
                cJSON *sunshine_duration = cJSON_GetObjectItem(hourly, "sunshine_duration");
                cJSON *cloud_cover = cJSON_GetObjectItem(hourly, "cloud_cover");
                cJSON *is_day = cJSON_GetObjectItem(hourly, "is_day");

                // Validate all required arrays exist and are correct type
                if (!time || !cJSON_IsArray(time) ||
                    !temperature_2m || !cJSON_IsArray(temperature_2m) ||
                    !dew_point_2m || !cJSON_IsArray(dew_point_2m) ||
                    !precipitation_probability || !cJSON_IsArray(precipitation_probability) ||
                    !rain || !cJSON_IsArray(rain) ||
                    !showers || !cJSON_IsArray(showers) ||
                    !snowfall || !cJSON_IsArray(snowfall) ||
                    !wind_speed_10m || !cJSON_IsArray(wind_speed_10m) ||
                    !wind_gusts_10m || !cJSON_IsArray(wind_gusts_10m) ||
                    !sunshine_duration || !cJSON_IsArray(sunshine_duration) ||
                    !cloud_cover || !cJSON_IsArray(cloud_cover) ||
                    !is_day || !cJSON_IsArray(is_day)) {

                    ESP_LOGE(TAG, "Hourly weather data incomplete - missing required arrays");
                    cJSON_Delete(json);
                    if (response.buffer) {
                        heap_caps_free(response.buffer);
                        response.buffer = NULL;
                        response.buffer_len = 0;
                    }
                    esp_http_client_close(client);
                    continue;
                }

                // Get array size and validate we have enough data
                int time_array_size = cJSON_GetArraySize(time);
                int required_size = currentHour + NUM_HOURS;

                if (time_array_size < required_size) {
                    ESP_LOGE(TAG, "Insufficient hourly data: need %d items, have %d (current hour: %d)",
                             required_size, time_array_size, currentHour);
                    cJSON_Delete(json);
                    if (response.buffer) {
                        heap_caps_free(response.buffer);
                        response.buffer = NULL;
                        response.buffer_len = 0;
                    }
                    esp_http_client_close(client);
                    continue;
                }

                // Iterate through hourly data with strict validation
                bool hourly_data_valid = true;
                for (int i = 0; i < NUM_HOURS; i++) {
                    int array_index = i + currentHour;

                    // Extract and validate each item
                    cJSON *time_item = cJSON_GetArrayItem(time, array_index);
                    cJSON *temp_item = cJSON_GetArrayItem(temperature_2m, array_index);
                    cJSON *dew_item = cJSON_GetArrayItem(dew_point_2m, array_index);
                    cJSON *precip_item = cJSON_GetArrayItem(precipitation_probability, array_index);
                    cJSON *rain_item = cJSON_GetArrayItem(rain, array_index);
                    cJSON *shower_item = cJSON_GetArrayItem(showers, array_index);
                    cJSON *snow_item = cJSON_GetArrayItem(snowfall, array_index);
                    cJSON *wind_speed_item = cJSON_GetArrayItem(wind_speed_10m, array_index);
                    cJSON *wind_gust_item = cJSON_GetArrayItem(wind_gusts_10m, array_index);
                    cJSON *sun_item = cJSON_GetArrayItem(sunshine_duration, array_index);
                    cJSON *cloud_item = cJSON_GetArrayItem(cloud_cover, array_index);
                    cJSON *day_item = cJSON_GetArrayItem(is_day, array_index);

                    // Fail if any item is NULL or wrong type
                    if (!time_item || !cJSON_IsNumber(time_item) ||
                        !temp_item || !cJSON_IsNumber(temp_item) ||
                        !dew_item || !cJSON_IsNumber(dew_item) ||
                        !precip_item || !cJSON_IsNumber(precip_item) ||
                        !rain_item || !cJSON_IsNumber(rain_item) ||
                        !shower_item || !cJSON_IsNumber(shower_item) ||
                        !snow_item || !cJSON_IsNumber(snow_item) ||
                        !wind_speed_item || !cJSON_IsNumber(wind_speed_item) ||
                        !wind_gust_item || !cJSON_IsNumber(wind_gust_item) ||
                        !sun_item || !cJSON_IsNumber(sun_item) ||
                        !cloud_item || !cJSON_IsNumber(cloud_item) ||
                        !day_item || !cJSON_IsNumber(day_item)) {

                        ESP_LOGE(TAG, "Hourly data item at index %d is invalid", array_index);
                        hourly_data_valid = false;
                        break;
                    }

                    // All items valid - extract data
                    time_t unixTimestamp = (time_t)time_item->valueint;
                    localtime_r(&unixTimestamp, &hourly_data[i].time);

                    hourly_data[i].temperature_2m = temp_item->valuedouble;
                    hourly_data[i].dew_point_2m = dew_item->valuedouble;
                    hourly_data[i].precipitation_probability = precip_item->valuedouble;
                    hourly_data[i].rain = rain_item->valuedouble;
                    hourly_data[i].showers = shower_item->valuedouble;
                    hourly_data[i].snowfall = snow_item->valuedouble;
                    hourly_data[i].wind_speed_10m = wind_speed_item->valuedouble;
                    hourly_data[i].wind_gusts_10m = wind_gust_item->valuedouble;
                    hourly_data[i].sunshine_duration = sun_item->valuedouble;
                    hourly_data[i].cloud_cover = cloud_item->valuedouble;
                    hourly_data[i].is_day = day_item->valueint ? true : false;
                }

                if (hourly_data_valid) {
                    hourly_success = true;
                    ESP_LOGI(TAG, "Hourly weather data successfully retrieved");
                } else {
                    ESP_LOGE(TAG, "Hourly data validation failed");
                    cJSON_Delete(json);
                    if (response.buffer) {
                        heap_caps_free(response.buffer);
                        response.buffer = NULL;
                        response.buffer_len = 0;
                    }
                    esp_http_client_close(client);
                    continue;
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

            // Clean up any existing buffer
            if (response.buffer) {
                heap_caps_free(response.buffer);
                response.buffer = NULL;
                response.buffer_len = 0;
            }
        }

        esp_http_client_close(client);

        // ------- Daily data -------
        sprintf(url, WEATHER_URL_DAILY, latitude, longitude);
        ESP_LOGI(TAG, "Call daily weather API: %s", url);

        esp_http_client_set_url(client, url);

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

                // Check structural integrity
                if (daily == NULL || !cJSON_IsObject(daily)) {
                    ESP_LOGE(TAG, "Missing or invalid 'daily' object in API response");
                    cJSON_Delete(json);
                    if (response.buffer) {
                        heap_caps_free(response.buffer);
                        response.buffer = NULL;
                        response.buffer_len = 0;
                    }
                    esp_http_client_close(client);
                    continue;
                }

                // Get all array references
                cJSON *time_array = cJSON_GetObjectItem(daily, "time");
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
                cJSON *sunrise = cJSON_GetObjectItem(daily, "sunrise");
                cJSON *sunset = cJSON_GetObjectItem(daily, "sunset");

                // Validate all required arrays exist and are correct type
                if (!time_array || !cJSON_IsArray(time_array) ||
                    !temperature_2m_max || !cJSON_IsArray(temperature_2m_max) ||
                    !temperature_2m_min || !cJSON_IsArray(temperature_2m_min) ||
                    !daylight_duration || !cJSON_IsArray(daylight_duration) ||
                    !sunshine_duration || !cJSON_IsArray(sunshine_duration) ||
                    !rain_sum || !cJSON_IsArray(rain_sum) ||
                    !showers_sum || !cJSON_IsArray(showers_sum) ||
                    !snowfall_sum || !cJSON_IsArray(snowfall_sum) ||
                    !precipitation_probability_max || !cJSON_IsArray(precipitation_probability_max) ||
                    !wind_speed_10m_max || !cJSON_IsArray(wind_speed_10m_max) ||
                    !wind_gusts_10m_max || !cJSON_IsArray(wind_gusts_10m_max) ||
                    !sunrise || !cJSON_IsArray(sunrise) ||
                    !sunset || !cJSON_IsArray(sunset)) {

                    ESP_LOGE(TAG, "Daily weather data incomplete - missing required arrays");
                    cJSON_Delete(json);
                    if (response.buffer) {
                        heap_caps_free(response.buffer);
                        response.buffer = NULL;
                        response.buffer_len = 0;
                    }
                    esp_http_client_close(client);
                    continue;
                }

                // Get array size and validate we have enough data
                int time_array_size = cJSON_GetArraySize(time_array);

                if (time_array_size < NUM_DAYS) {
                    ESP_LOGE(TAG, "Insufficient daily data: need %d days, have %d", NUM_DAYS, time_array_size);
                    cJSON_Delete(json);
                    if (response.buffer) {
                        heap_caps_free(response.buffer);
                        response.buffer = NULL;
                        response.buffer_len = 0;
                    }
                    esp_http_client_close(client);
                    continue;
                }

                // Iterate through daily data with strict validation
                bool daily_data_valid = true;
                for (int i = 0; i < NUM_DAYS; i++) {
                    // Extract and validate each item
                    cJSON *time_item = cJSON_GetArrayItem(time_array, i);
                    cJSON *temp_max_item = cJSON_GetArrayItem(temperature_2m_max, i);
                    cJSON *temp_min_item = cJSON_GetArrayItem(temperature_2m_min, i);
                    cJSON *daylight_item = cJSON_GetArrayItem(daylight_duration, i);
                    cJSON *sunshine_item = cJSON_GetArrayItem(sunshine_duration, i);
                    cJSON *rain_item = cJSON_GetArrayItem(rain_sum, i);
                    cJSON *shower_item = cJSON_GetArrayItem(showers_sum, i);
                    cJSON *snow_item = cJSON_GetArrayItem(snowfall_sum, i);
                    cJSON *precip_item = cJSON_GetArrayItem(precipitation_probability_max, i);
                    cJSON *wind_speed_item = cJSON_GetArrayItem(wind_speed_10m_max, i);
                    cJSON *wind_gust_item = cJSON_GetArrayItem(wind_gusts_10m_max, i);
                    cJSON *sunrise_item = cJSON_GetArrayItem(sunrise, i);
                    cJSON *sunset_item = cJSON_GetArrayItem(sunset, i);

                    // Fail if any item is NULL or wrong type
                    if (!time_item || !cJSON_IsNumber(time_item) ||
                        !temp_max_item || !cJSON_IsNumber(temp_max_item) ||
                        !temp_min_item || !cJSON_IsNumber(temp_min_item) ||
                        !daylight_item || !cJSON_IsNumber(daylight_item) ||
                        !sunshine_item || !cJSON_IsNumber(sunshine_item) ||
                        !rain_item || !cJSON_IsNumber(rain_item) ||
                        !shower_item || !cJSON_IsNumber(shower_item) ||
                        !snow_item || !cJSON_IsNumber(snow_item) ||
                        !precip_item || !cJSON_IsNumber(precip_item) ||
                        !wind_speed_item || !cJSON_IsNumber(wind_speed_item) ||
                        !wind_gust_item || !cJSON_IsNumber(wind_gust_item) ||
                        !sunrise_item || !cJSON_IsNumber(sunrise_item) ||
                        !sunset_item || !cJSON_IsNumber(sunset_item)) {

                        ESP_LOGE(TAG, "Daily data item at index %d is invalid", i);
                        daily_data_valid = false;
                        break;
                    }

                    // All items valid - extract data
                    time_t timeTimestamp = (time_t)time_item->valueint;
                    localtime_r(&timeTimestamp, &daily_data[i].time);

                    daily_data[i].temperature_2m_max = temp_max_item->valuedouble;
                    daily_data[i].temperature_2m_min = temp_min_item->valuedouble;
                    daily_data[i].daylight_duration = daylight_item->valuedouble;
                    daily_data[i].sunshine_duration = sunshine_item->valuedouble;
                    daily_data[i].rain_sum = rain_item->valuedouble;
                    daily_data[i].showers_sum = shower_item->valuedouble;
                    daily_data[i].snowfall_sum = snow_item->valuedouble;
                    daily_data[i].precipitation_probability_max = precip_item->valuedouble;
                    daily_data[i].wind_speed_10m_max = wind_speed_item->valuedouble;
                    daily_data[i].wind_gusts_10m_max = wind_gust_item->valuedouble;

                    time_t sunriseTimestamp = (time_t)sunrise_item->valueint;
                    localtime_r(&sunriseTimestamp, &daily_data[i].sunrise);

                    time_t sunsetTimestamp = (time_t)sunset_item->valueint;
                    localtime_r(&sunsetTimestamp, &daily_data[i].sunset);
                }

                if (daily_data_valid) {
                    daily_success = true;
                    ESP_LOGI(TAG, "Daily weather data successfully retrieved");
                } else {
                    ESP_LOGE(TAG, "Daily data validation failed");
                    cJSON_Delete(json);
                    if (response.buffer) {
                        heap_caps_free(response.buffer);
                        response.buffer = NULL;
                        response.buffer_len = 0;
                    }
                    esp_http_client_close(client);
                    continue;
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

            // Clean up any existing buffer
            if (response.buffer) {
                heap_caps_free(response.buffer);
                response.buffer = NULL;
                response.buffer_len = 0;
            }
        }

        esp_http_client_close(client);

        // Only update display if ALL three API calls succeeded with complete data
        if (current_success && hourly_success && daily_success) {
            ESP_LOGI(TAG, "All weather data retrieved successfully - updating display");
            xSemaphoreTakeRecursive(lvgl_mux, portMAX_DELAY);
            disp_weather(&current_data, hourly_data, daily_data);
            xSemaphoreGiveRecursive(lvgl_mux);
        } else {
            ESP_LOGW(TAG, "Weather data incomplete - skipping display update (current:%d, hourly:%d, daily:%d)",
                     current_success, hourly_success, daily_success);
        }

        // Reset success flags for next iteration
        current_success = false;
        hourly_success = false;
        daily_success = false;

        vTaskDelay(pdMS_TO_TICKS(1000 * 60 *15)); // Every 15 Minutes
        //vTaskDelay(pdMS_TO_TICKS(1000)); 
    }
}