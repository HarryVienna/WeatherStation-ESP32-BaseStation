#include <stdio.h>
#include <string.h>
#include <time.h>

#include "esp_log.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "clock_task.h"

#include "gui/gui.h"
#include "config/config.h"


static const char* TAG = "clock_task";

extern SemaphoreHandle_t lvgl_mux;

/**
 * @brief     Task for updating and displaying date and time
 *
 * @param     pvParameter   Pointer to task parameters (not used in this function)
 *
 * @details   Updates the date and time information continuously and displays it.
 *            Utilizes a loop to update and format the date-time string at regular intervals.
 */
void clock_task(void *pvParameter){

  ESP_LOGI(TAG, "Start Clock task");

  struct tm timeinfo;
  time_t now;

  char str_ftime[18];
  char date_time[21];

  for (;;) {

    time(&now);
    localtime_r(&now, &timeinfo);

    strftime(str_ftime, sizeof(str_ftime), "%d.%m.%Y  %H:%M", &timeinfo);
    snprintf(date_time, sizeof(date_time), "%s %s", DAY_NAMES[timeinfo.tm_wday], str_ftime);

    xSemaphoreTakeRecursive(lvgl_mux, portMAX_DELAY);
    disp_date_time(date_time);
    xSemaphoreGiveRecursive(lvgl_mux);

    vTaskDelay(pdMS_TO_TICKS(1000)); // Sleep for 1 second
  }
}