#include <Arduino.h>

#include <stdio.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

#include "weather_task.h"

#include "../gui/gui.h"
#include "../config/config.h"
#include "../openweather/openweatherapi.h"

static const char* TAG = "weather_task";

extern SemaphoreHandle_t mutex;

/**
 * @brief Main task for controling the hands 
 *
 */
void weather_task(void *pvParameter){

  HTTPClient http;

  String openweather_url = "https://api.openweathermap.org/data/3.0/onecall";
  String openweather_lat = "48.xxx";
  String openweather_lon = "16.yyy";
  String openweather_units = "metric";
  String openweather_lang = "de";
  String openweather_appid = "zzz";

  String apiURL = openweather_url + "?lat=" + openweather_lat + "&lon=" + openweather_lon + "&units=" + openweather_units + "&lang=" + openweather_lang + "&appid=" + openweather_appid;

  for (;;) {

    Serial.println("Call weather API ");

    http.begin(apiURL);
    int httpResponseCode = http.GET();

    if (httpResponseCode == HTTP_CODE_OK) {
      BasicJsonDocument<SpiRamAllocator> doc(65536);
      DeserializationError error = deserializeJson(doc, http.getStream());

      if (error) {
        log_e("deserializeJson() failed: %s", error.c_str());
      }
      else {
        JsonObject root = doc.as<JsonObject>();
        
        OpenWeatherData data = OpenWeatherData::from_json(root);

        //data.logValues();

        xSemaphoreTake(mutex, portMAX_DELAY);
        disp_weather(data);
        xSemaphoreGive(mutex);
      }
    }

    http.end();

    vTaskDelay(1000 * 60 * 5 / portTICK_PERIOD_MS); // Every 5 Minutes
    //vTaskDelay(1000 * 10 / portTICK_PERIOD_MS); // Every 10 Minutes
  }
}