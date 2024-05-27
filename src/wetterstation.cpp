#include <stdio.h>

#include "esp_err.h"
#include "esp_log.h"

#include "config/config.h"

#include "wifi/network.h"
#include "display/esp32_s3.h"
#include "ui/ui.h"

static const char* TAG = "MAIN";

extern SemaphoreHandle_t lvgl_mux;


extern "C" void app_main(void)
{

    init_wifi();
    init_display();

    ESP_LOGI(TAG, "Start LVGL");

    xSemaphoreTakeRecursive(lvgl_mux, portMAX_DELAY);
    ui_init();
    xSemaphoreGiveRecursive(lvgl_mux);


    // This lop handles LVGL operations in the background. It periodically calls
    // the LVGL timer handler to update the GUI.
    while (1) {

        xSemaphoreTakeRecursive(lvgl_mux, portMAX_DELAY);
        lv_timer_handler();
        xSemaphoreGiveRecursive(lvgl_mux);

        vTaskDelay(pdMS_TO_TICKS(LVGL_TASK_DELAY_MS));
    }

}