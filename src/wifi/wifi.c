#include "sdkconfig.h"
#include "esp_log.h"

#include "esp_wifi.h"
#include "nvs_flash.h"
#include "esp_event.h"

#include "../config/config.h"



static const char* TAG = "WIFI";

/**
 * @brief Initialize WIFI
 *
 * This function initializes the WIFI driver.
 */
void init_wifi(void) {
    ESP_LOGI(TAG, "Install WIFI driver");

    esp_err_t ret;

    //Initialize NVS
    ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
      ESP_ERROR_CHECK(nvs_flash_erase());
      if (nvs_flash_init() != ESP_OK) {
        ESP_LOGE(TAG, "Failed to init nvs flash");
      }
    }

    // Initialize the underlying TCP/IP stack
    if (esp_netif_init() != ESP_OK) {
        ESP_LOGE(TAG, "Failed to init netif");  
    }

    // Create default event loop
    if (esp_event_loop_create_default() != ESP_OK) {
        ESP_LOGE(TAG, "Failed to set create event loop");      
    }

    // Creates default WIFI ST
    esp_netif_t *sta_netif = esp_netif_create_default_wifi_sta();
    assert(sta_netif);

    // Set hostname
    if (esp_netif_set_hostname(sta_netif, HOST_NAME)) {
        ESP_LOGE(TAG, "Failed to set hostname");
    }

    //  Init WiFi
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    if (esp_wifi_init(&cfg) != ESP_OK) {
        ESP_LOGE(TAG, "Failed to init wifi");   
    }
    
    // Set the WiFi operating mode
    if (esp_wifi_set_mode(WIFI_MODE_STA) != ESP_OK) {
        ESP_LOGE(TAG, "Failed to set mode");   
    }

    // Set storage to flash
    if (esp_wifi_set_storage(WIFI_STORAGE_FLASH) != ESP_OK) {
        ESP_LOGE(TAG, "Failed to set storage to flash");
    }

    if (esp_wifi_start() != ESP_OK) {
        ESP_LOGE(TAG, "Failed to start wifi");     
    }
}