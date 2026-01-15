#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_wifi.h"
#include "esp_log.h"

#include "gui/gui.h"


static const char* TAG = "wifiscan_task";

extern SemaphoreHandle_t lvgl_mux;

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
void wifiscan_task(void *pvParameter) {
    ESP_LOGI(TAG, "Start wifiscan_task");

    xSemaphoreTakeRecursive(lvgl_mux, portMAX_DELAY);
    disp_disable_scanbutton(true);
    xSemaphoreGiveRecursive(lvgl_mux);

    if (esp_wifi_start() != ESP_OK) {
        ESP_LOGE(TAG, "Failed to start wifi");     
    }

    ESP_ERROR_CHECK(esp_wifi_disconnect());
    vTaskDelay(pdMS_TO_TICKS(1000)); // Delay for 1 second
    
    wifi_scan_config_t scan_config = {
        .ssid = NULL,
        .bssid = NULL,
        .channel = 0,
        .show_hidden = true,
        .scan_type = WIFI_SCAN_TYPE_ACTIVE,
        .scan_time.active.min = 1000,
        .scan_time.active.max = 5000,
    };

    uint16_t ap_count = 16;
    esp_wifi_scan_start(&scan_config, true);
    esp_wifi_scan_get_ap_records(&ap_count, NULL);

    wifi_ap_record_t *ap_records = (wifi_ap_record_t *)malloc(sizeof(wifi_ap_record_t) * ap_count);
    esp_wifi_scan_get_ap_records(&ap_count, ap_records);

    char allNetworks[4096] = {0}; // Assuming a maximum of 4096 characters for all network names

    // Print scanned networks
    for (uint16_t i = 0; i < ap_count; i++) {
        // Check if the SSID is not empty
        if (strlen((const char *)ap_records[i].ssid) > 0) {
             char item[128]; // Assuming a maximum of 128 characters per network item
            snprintf(item, sizeof(item), "%s (%d) %s", (const char *)ap_records[i].ssid, ap_records[i].rssi, (ap_records[i].authmode == WIFI_AUTH_OPEN) ? "" : "*");
            ESP_LOGI(TAG, "%s", item);

            strlcat(allNetworks, (const char *)ap_records[i].ssid, sizeof(allNetworks));
            if (i != ap_count - 1) {
                strlcat(allNetworks, "\n", sizeof(allNetworks)); // Add newline character except for the last SSID
            }
        }

    }

    free(ap_records);

    ESP_ERROR_CHECK(esp_wifi_stop());

    xSemaphoreTakeRecursive(lvgl_mux, portMAX_DELAY);
    disp_wifi_networks(allNetworks);
    disp_disable_scanbutton(false);
    xSemaphoreGiveRecursive(lvgl_mux);

    vTaskDelete(NULL); // Delete the task when done
}