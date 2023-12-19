#include <Arduino.h>
#include "WiFi.h"

#include <stdio.h>
#include <string.h>
#include <time.h>
#include <mutex>

#include "esp_log.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "wifiscan_task.h"

#include "../gui/gui.h"
#include "../config/config.h"


static const char* TAG = "wifiscan_task";

extern SemaphoreHandle_t mutex;

/**
 * @brief     Task for scanning nearby Wi-Fi networks
 *
 * @param     pvParameter   Pointer to task parameters (not used in this function)
 *
 * @details   Initiates Wi-Fi scanning to discover nearby networks and their signal strengths.
 *            Prints the scanned networks and their information.
 *            Displays the scanned Wi-Fi networks on the device's display.
 *            Deletes the task once the scan is complete.
 */
void wifiscan_task(void *pvParameter){

  xSemaphoreTake(mutex, portMAX_DELAY);
  disp_disable_scanbutton(true);
  xSemaphoreGive(mutex);

  // Start Wi-Fi scanning
  WiFi.disconnect(false); // Disconnect any previous connections
  
  delay(1000);

  int networksFound = WiFi.scanNetworks();

  String allNetworks;

  // Print scanned networks
  Serial.println("Scanning Wi-Fi networks...");
  for (int i = 0; i < networksFound; ++i) {
    String item = WiFi.SSID(i) + " (" + WiFi.RSSI(i) + ") " + ((WiFi.encryptionType(i) == WIFI_AUTH_OPEN) ? " " : "*");
    Serial.println(item);

	  allNetworks += WiFi.SSID(i);
    if (i != networksFound - 1) {
      allNetworks += "\n"; // Add newline character except for the last SSID
    }
  }

  xSemaphoreTake(mutex, portMAX_DELAY);
  disp_wifi_networks(allNetworks);
  disp_disable_scanbutton(false);
  xSemaphoreGive(mutex);

  vTaskDelete(NULL); // Delete the task when done
}