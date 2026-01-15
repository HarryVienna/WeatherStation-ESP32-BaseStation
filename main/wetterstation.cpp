#include <stdio.h>

#include "esp_err.h"
#include "esp_log.h"

#include "config/config.h"

#include "wifi/network.h"
#include "display/esp32_s3.h"
#include "ui/ui.h"

#include "lvgl/lv_screenshot.h"

static const char* TAG = "MAIN";

extern SemaphoreHandle_t lvgl_mux;


extern "C" void app_main(void)
{

    init_wifi();
    init_display();
   
    if (lvgl_port_lock(-1)) {
        ui_init();
        lvgl_port_unlock();
    }

    ESP_LOGI(TAG, "Weather station started");

    // Uncomment if you want to make automatic screenshots
    // wifi_connect("xxx", "xxx", false);
    // start_screenshot(60, 10);

}