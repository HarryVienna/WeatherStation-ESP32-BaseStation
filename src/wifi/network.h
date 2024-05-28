#ifndef NETWORK_H
#define NETWORK_H

#include <stdio.h>


#ifdef __cplusplus
extern "C" {
#endif

// Function declarations
void init_wifi(void);
bool wifi_connect(const char* ssid, const char* password, bool retry_forever);
void wifi_start();
void esp_now_start();

#ifdef __cplusplus
}
#endif

#endif /* NETWORK_H */