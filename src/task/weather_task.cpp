#include <Arduino.h>
#include <Preferences.h>

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
 * @brief     Task for retrieving and displaying weather data from an API
 *
 * @param     pvParameter   Pointer to task parameters (not used in this function)
 *
 * @details   Retrieves weather data from the OpenWeatherMap API based on configured coordinates and API key.
 *            Periodically calls the API, parses the received JSON data, and displays weather information.
 */
void weather_task(void *pvParameter){

  Preferences preferences;
  preferences.begin("Weatherstation", false);

  String openweather_lat = preferences.getString("latitude", ""); 
  String openweather_lon = preferences.getString("longitude", "");
  String openweather_appid = preferences.getString("appid", ""); 

  preferences.end();

  HTTPClient http;

  String openweather_url = "https://api.openweathermap.org/data/3.0/onecall";
  String openweather_units = "metric";
  String openweather_lang = "de";
  
  String apiURL = openweather_url + "?lat=" + openweather_lat + "&lon=" + openweather_lon + "&units=" + openweather_units + "&lang=" + openweather_lang + "&appid=" + openweather_appid;
  //String apiURL = "https://haraldkreuzer.net/test30.json";

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

    vTaskDelay(1000 * 60 * 10 / portTICK_PERIOD_MS); // Every 10 Minutes == 144 calls per day
  }
}