#include <Arduino.h>
#include "WiFi.h"

#include <stdio.h>
#include <string.h>
#include <time.h>
#include <mutex>

#include "esp_log.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "wificonnect_task.h"

#include "../gui/gui.h"
#include "../config/config.h"


static const char* TAG = "wifiscan_task";

extern SemaphoreHandle_t mutex;

/**
 * @brief     Task for connecting to a Wi-Fi network
 *
 * @param     pvParameter   Pointer to Wi-Fi configuration parameters
 *
 * @details   Attempts to connect to the specified Wi-Fi network using provided credentials.
 *            Tries to establish a connection for a limited number of attempts and displays the connection status.
 *            Deletes the task once the connection attempt is finished.
 */
void wificonnect_task(void *pvParameter){

  local_wifi_sta_config_t *wifiParams = (local_wifi_sta_config_t *)pvParameter;

  const char* ssid = wifiParams->ssid;
  const char* password = wifiParams->password;

  const int maxAttempts = 10;
  int attempts = 0;

  WiFi.disconnect(true); // Disconnect any previous connections

  WiFi.mode(WIFI_STA);

  bool is_connected = false;
  
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi ..");

  while (attempts < maxAttempts) {
    delay(1000); // Allow some time for connection to establish
    Serial.print(".");

    int connectionStatus = WiFi.status();

    if (WiFi.status() == WL_CONNECTED) {
      // Successfully connected to the Wi-Fi network
      Serial.println("Connected to WiFi!");
      Serial.print("IP Address: ");
      Serial.println(WiFi.localIP());
      is_connected = true;
      break; // Exit the loop
    }

    attempts++;
  }

  WiFi.disconnect(true);

  xSemaphoreTake(mutex, portMAX_DELAY);
  disp_connect_status(is_connected);
  xSemaphoreGive(mutex);

  vTaskDelete(NULL); // Delete the task when done
}