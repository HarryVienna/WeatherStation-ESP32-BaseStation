#ifndef WIFICONNECT_TASK_H
#define WIFICONNECT_TASK_H

typedef struct {
  char* ssid;
  char* password;
} local_wifi_sta_config_t;

void wificonnect_task(void *pvParameter);

#endif