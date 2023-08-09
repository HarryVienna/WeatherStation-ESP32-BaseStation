#include <Arduino.h>

#include <esp_now.h>

#include "WiFi.h"

#include "../gui/gui.h"

#include "network.h"

static const char* TAG = "network";

extern SemaphoreHandle_t mutex;

const char* ssid = "xxx";
const char* password = "yyy";

extern SemaphoreHandle_t mutex;

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




void print_mac(const uint8_t * mac_addr){
  char macStr[18];
  snprintf(macStr, sizeof(macStr), "%02x:%02x:%02x:%02x:%02x:%02x",
           mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);

  Serial.println(macStr);
}

void add_peer(const uint8_t * mac_addr, uint8_t chan){
  esp_now_peer_info_t peer;

  esp_now_del_peer(mac_addr);

  memset(&peer, 0, sizeof(esp_now_peer_info_t));
  peer.channel = chan;
  peer.encrypt = false;
  memcpy(peer.peer_addr, mac_addr, ESP_NOW_ETH_ALEN);

  if (esp_now_add_peer(&peer) != ESP_OK){
    Serial.println("Failed to add peer");
  }
}


void on_data_sent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("Last Packet Send Status: ");
  Serial.print(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success to " : "Delivery Fail to ");
  print_mac(mac_addr);
  Serial.println();
}

void on_data_recv(const uint8_t * mac_addr, const uint8_t *incomingData, int len) {

  Serial.print(len);
  Serial.print(" bytes of data received from : ");
  print_mac(mac_addr);

  uint8_t type = incomingData[0];
  switch (type) {
  case DATA :

    struct_data msg;
    memcpy(&msg, incomingData, sizeof(struct_data));

    char date_time[16];
    struct tm timeinfo;
    getLocalTime(&timeinfo);
    strftime(date_time, sizeof(date_time),"%H:%M", &timeinfo);

    xSemaphoreTake(mutex, portMAX_DELAY);
    disp_sensor_data(msg.sensor_nr, msg.temperature, msg.humidity, msg.pressure, msg.voltage, date_time);
    xSemaphoreGive(mutex);  

    break;

  case PAIRING_REQ:
    add_peer(mac_addr, WiFi.channel());

    struct_pairing_request pairingRequest;
    memcpy(&pairingRequest, incomingData, sizeof(struct_pairing_request));

    struct_pairing_response pairingResponse;
    pairingResponse.msg_type = PAIRING_RESP;
    pairingResponse.sensor_nr = pairingRequest.sensor_nr;
    pairingResponse.channel = WiFi.channel();
    WiFi.macAddress(pairingResponse.macAddr);

    esp_err_t result = esp_now_send(mac_addr, (uint8_t *) &pairingResponse, sizeof(struct_pairing_response));
    if (result != ESP_OK) {
        Serial.print("Fehler beim Senden der Nachricht");
    }

    break;
  }
}

void wifi_start() {
  WiFi.mode(WIFI_STA);
  WiFi.setSleep(false);
  WiFi.begin(ssid, password);
  
  Serial.print("Connecting to WiFi ..");
  
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print('.');
    delay(1000);
  }

  configTzTime("CET-1CEST,M3.5.0,M10.5.0/3", "de.pool.ntp.org");

  Serial.println(WiFi.localIP());
  Serial.println(WiFi.channel());
}

void esp_now_start(){

    if (esp_now_init() != ESP_OK) {
      Serial.println("Error initializing ESP-NOW");
      return;
    }
    esp_now_register_send_cb(on_data_sent);
    esp_now_register_recv_cb(on_data_recv);
}
