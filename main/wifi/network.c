#include <string.h>

#include "sdkconfig.h"
#include "nvs_flash.h"
#include "nvs.h"
#include "esp_log.h"

#include "esp_now.h"
#include "esp_wifi.h"
#include "nvs_flash.h"
#include "esp_event.h"
#include "esp_sntp.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "nvs/preferences.h"

#include "gui/gui.h"

#include "config/config.h"

#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT      BIT1

#define MAC_STR_LEN 18

static const char* TAG = "WIFI";

extern SemaphoreHandle_t lvgl_mux;
static SemaphoreHandle_t sync_semaphore;

typedef struct struct_data {
    uint8_t msg_type;
    uint8_t sensor_nr;
    uint32_t voltage;
    double pressure;
    double temperature;
    double humidity;
} struct_data;

typedef struct struct_pairing_response {
    uint8_t msg_type;
    uint8_t sensor_nr;
    uint8_t macAddr[ESP_NOW_ETH_ALEN];
    uint8_t channel;
} struct_pairing_response;

typedef struct struct_pairing_request {
    uint8_t msg_type;
    uint8_t sensor_nr;
} struct_pairing_request;

enum MessageType {
    PAIRING_REQ,
    PAIRING_RESP,
    DATA,
};

/**
 * @brief     Print MAC address to Serial monitor
 *
 * @param     mac_addr  Pointer to the MAC address array
 *
 * @details   Formats the MAC address provided as an array of uint8_t into a string.
 *            Prints the formatted MAC address to the Serial monitor.
 */
char* get_mac_string(const uint8_t *mac_addr, char *macStrBuffer) {
    snprintf(macStrBuffer, MAC_STR_LEN, "%02x:%02x:%02x:%02x:%02x:%02x",
             mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
    return macStrBuffer;
}

/**
 * @brief     Add a peer for ESP-NOW communication
 *
 * @param     mac_addr  Pointer to the MAC address of the peer
 * @param     chan      Channel to communicate with the peer
 *
 * @details   Prepares and adds a peer for ESP-NOW communication.
 *            Deletes the existing peer with the provided MAC address if present.
 *            Initializes peer information, sets the channel and encryption settings,
 *            and adds the peer using ESP-NOW API.
 *
 * @note      The function will print an error message if adding the peer fails.
 */
void add_peer(const uint8_t * mac_addr, uint8_t chan){
  esp_now_peer_info_t peer;

  esp_now_del_peer(mac_addr);

  memset(&peer, 0, sizeof(esp_now_peer_info_t));
  peer.channel = chan;
  peer.encrypt = false;
  memcpy(peer.peer_addr, mac_addr, ESP_NOW_ETH_ALEN);

  if (esp_now_add_peer(&peer) != ESP_OK){
    ESP_LOGE(TAG, "Failed to add peer");
  }
}

/**
 * @brief     Callback function for handling data transmission status
 *
 * @param     mac_addr  Pointer to the MAC address of the recipient
 * @param     status    Status of the data transmission (success or failure)
 *
 * @details   Prints the status of the last packet transmission to the specified MAC address.
 *            Displays whether the delivery was successful or failed via the Serial monitor.
 */
void on_data_sent(const wifi_tx_info_t *info, esp_now_send_status_t status) {
    
    const uint8_t *mac_addr = info->des_addr;

    if (mac_addr == NULL) {
        ESP_LOGE(TAG, "Send cb arg error");
        return;
    }
    char mac[MAC_STR_LEN];
    ESP_LOGI(TAG, "Last Packet Send to %s with status: %s ", get_mac_string(mac_addr, mac), status == ESP_NOW_SEND_SUCCESS ? "Success" : "Fail");
}

/**
 * @brief     Callback function for handling received data
 *
 * @param     recv_info     ESPNOW packet information
 * @param     incomingData  Pointer to the received data
 * @param     len           Length of the received data
 *
 * @details   Processes received data based on its type, such as sensor data or pairing requests.
 *            Displays the received data details, including sensor information and time, if applicable.
 *            Handles pairing requests by responding and adding peers for communication.
 */
void on_data_recv(const esp_now_recv_info_t *recv_info, const uint8_t *incoming_data, int len) {

    uint8_t * mac_addr = recv_info->src_addr;

    if (mac_addr == NULL || incoming_data == NULL || len <= 0) {
        ESP_LOGE(TAG, "Receive cb arg error");
        return;
    }

    char mac[MAC_STR_LEN];
    ESP_LOGI(TAG, "%d bytes of data received from %s", len, get_mac_string(mac_addr, mac));
  
    uint8_t type = incoming_data[0];
    switch (type) {
    case DATA :

        struct_data msg;
        memcpy(&msg, incoming_data, sizeof(struct_data));

        char date_time[16];
        time_t now;
        struct tm timeinfo;
        time(&now);
        localtime_r(&now, &timeinfo);
        strftime(date_time, sizeof(date_time),"%H:%M", &timeinfo);

        xSemaphoreTakeRecursive(lvgl_mux, portMAX_DELAY);
        disp_sensor_data(msg.sensor_nr, msg.temperature, msg.humidity, msg.pressure, msg.voltage, date_time);
        xSemaphoreGiveRecursive(lvgl_mux);  

        break;

    case PAIRING_REQ:
        uint8_t primary;
        wifi_second_chan_t second;
        esp_wifi_get_channel(&primary, &second);
        add_peer(mac_addr, primary);

        struct_pairing_request pairingRequest;
        memcpy(&pairingRequest, incoming_data, sizeof(struct_pairing_request));

        struct_pairing_response pairingResponse;
        pairingResponse.msg_type = PAIRING_RESP;
        pairingResponse.sensor_nr = pairingRequest.sensor_nr;
        pairingResponse.channel = primary;
        esp_wifi_get_mac(ESP_IF_WIFI_STA, pairingResponse.macAddr);

        esp_err_t result = esp_now_send(mac_addr, (uint8_t *) &pairingResponse, sizeof(struct_pairing_response));
        if (result != ESP_OK) {
            ESP_LOGE(TAG, "Error sending message");
        }

        break;
    }
}

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

    // Set storage to RAM
    if (esp_wifi_set_storage(WIFI_STORAGE_RAM) != ESP_OK) {
        ESP_LOGE(TAG, "Failed to set storage");
    }


}


/* FreeRTOS event group to signal when we are connected*/
static EventGroupHandle_t s_wifi_event_group;

static int s_retry_num = 0;
static bool s_retry_forever = false;

static void event_handler(void* arg, esp_event_base_t event_base,
                                int32_t event_id, void* event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    } 
    else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        disp_wifi_status(false);

        if (s_retry_forever || s_retry_num < 10) {
            esp_wifi_connect();
            s_retry_num++;
            ESP_LOGI(TAG, "retry to connect to the AP");
        } else {
            xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT);
        }
        ESP_LOGI(TAG,"connect to the AP fail");
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        disp_wifi_status(true);

        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        ESP_LOGI(TAG, "got ip:" IPSTR, IP2STR(&event->ip_info.ip));
        s_retry_num = 0;

        xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
    } 
}

/**
 * @brief Connect WIFI
 *
 * This function connects to WIFI
 */
bool wifi_connect(const char* ssid, const char* password, bool retry_forever) {

    ESP_LOGI(TAG, "connecting to ap SSID: >%s<  password: >%s<", ssid, password);

    ESP_ERROR_CHECK(esp_wifi_stop());

    s_wifi_event_group = xEventGroupCreate();

    esp_event_handler_instance_t instance_any_id;
    esp_event_handler_instance_t instance_got_ip;
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                                                        ESP_EVENT_ANY_ID,
                                                        &event_handler,
                                                        NULL,
                                                        &instance_any_id));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT,
                                                        IP_EVENT_STA_GOT_IP,
                                                        &event_handler,
                                                        NULL,
                                                        &instance_got_ip));
    wifi_config_t wifi_config = {};
    memcpy(wifi_config.sta.ssid, ssid, sizeof(wifi_config.sta.ssid));
    memcpy(wifi_config.sta.password, password, sizeof(wifi_config.sta.password));

    s_retry_forever = retry_forever;
    s_retry_num = 0;

    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config) );
    ESP_ERROR_CHECK(esp_wifi_start());

    /* Waiting until either the connection is established (WIFI_CONNECTED_BIT) or connection failed for the maximum
     * number of re-tries (WIFI_FAIL_BIT). The bits are set by event_handler() (see above) */
    ESP_LOGI(TAG, "Waiting for xEventGroupWaitBits");
    EventBits_t bits = xEventGroupWaitBits(s_wifi_event_group,
            WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
            pdFALSE,
            pdFALSE,
            portMAX_DELAY);

    /* xEventGroupWaitBits() returns the bits before the call returned, hence we can test which event actually
     * happened. */
    return (bits & WIFI_CONNECTED_BIT);
    
}

/**
 * @brief Callback function from time sync
 *
 */
void sync_callback(struct timeval *tv) {
  ESP_LOGI(TAG, "Syncing date/time: %s", ctime(&tv->tv_sec));
  xSemaphoreGive(sync_semaphore);
}


/**
 * @brief     Start WiFi connection and setup time synchronization
 *
 * @details   Initializes WiFi connection using stored credentials.
 *            Connects to the specified SSID using the provided password.
 *            Configures time synchronization based on the given timezone and NTP server.
 */
void wifi_start() {

    nvs_handle_t nvs_handle;
    nvs_open("weatherstation", NVS_READONLY, &nvs_handle);

    char* ssid = get_string_from_nvs(nvs_handle, "ssid", "");
    char* password = get_string_from_nvs(nvs_handle, "password", "");
    char* tz = get_string_from_nvs(nvs_handle, "tz", "");

    nvs_close(nvs_handle);

    wifi_connect(ssid, password, true);

    // Create and take the semaphore
    sync_semaphore = xSemaphoreCreateBinary();

    esp_sntp_setoperatingmode(SNTP_OPMODE_POLL);
    esp_sntp_setservername(0, NTP_SERVER);
    sntp_set_time_sync_notification_cb(&sync_callback);
    esp_sntp_init();

    setenv("TZ", tz, 1);
    tzset();

    // Wait for the sync_callback to give the semaphore
    if (xSemaphoreTake(sync_semaphore, portMAX_DELAY) == pdTRUE) {
        ESP_LOGI(TAG, "Time synchronization successful");
    } else {
        ESP_LOGW(TAG, "Time synchronization timeout");
    }
}

/**
 * @brief     Start ESP-NOW communication
 *
 * @details   Initializes ESP-NOW communication protocol on the ESP32.
 *            Registers callbacks for sending and receiving data.
 *
 */
void esp_now_start(){

    if (esp_now_init() != ESP_OK) {
      ESP_LOGE(TAG, "Error initializing ESP-NOW");
      return;
    }
    esp_now_register_send_cb(on_data_sent);
    esp_now_register_recv_cb(on_data_recv);
}