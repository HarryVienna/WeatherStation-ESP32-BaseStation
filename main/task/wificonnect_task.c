#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_wifi.h"
#include "esp_log.h"
#include "nvs_flash.h"

#include "wificonnect_task.h"

#include "wifi/network.h"
#include "gui/gui.h"


static const char* TAG = "wificonnect_task";

extern SemaphoreHandle_t lvgl_mux;

/**
 * @brief     Task for connecting to a Wi-Fi network
 *
 * @param     pvParameter   Pointer to Wi-Fi configuration parameters
 *
 * @details   Attempts to connect to the specified Wi-Fi network using provided credentials.
 *            Tries to establish a connection for a limited number of attempts and displays the connection status.
 *            Deletes the task once the connection attempt is finished.
 */
void wificonnect_task(void *pvParameter) {

    ESP_LOGI(TAG, "Start wifiscan_task");

    xSemaphoreTakeRecursive(lvgl_mux, portMAX_DELAY);
    disp_disable_connectbutton(true);
    xSemaphoreGiveRecursive(lvgl_mux);

    local_wifi_sta_config_t *wifiParams = (local_wifi_sta_config_t *)pvParameter;

    bool connected = wifi_connect(wifiParams->ssid, wifiParams->password, false);

    xSemaphoreTakeRecursive(lvgl_mux, portMAX_DELAY);
    disp_connect_status(connected);
    disp_disable_connectbutton(false);
    xSemaphoreGiveRecursive(lvgl_mux);

    vTaskDelete(NULL); // Delete the task when done
}