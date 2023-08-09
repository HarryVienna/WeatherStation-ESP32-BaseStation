#include <Arduino.h>

#include <stdio.h>
#include <string.h>
#include <time.h>
#include <mutex>

#include "esp_log.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "clock_task.h"

#include "../gui/gui.h"
#include "../config/config.h"


static const char* TAG = "clock_task";

extern SemaphoreHandle_t mutex;

/**
 * @brief Main task for controling the hands 
 *
 */
void clock_task(void *pvParameter){

  char str_ftime[64];
  char date_time[64];
  struct tm timeinfo;
  time_t now;

  for (;;) {

    time(&now);
    localtime_r(&now, &timeinfo);

    strftime(str_ftime, sizeof(str_ftime),"%d.%m.%Y  %H:%M", &timeinfo);
    sprintf(date_time, "%s, %s", DAY_NAMES[timeinfo.tm_wday], str_ftime);

    xSemaphoreTake(mutex, portMAX_DELAY);
    disp_date_time(date_time);
    xSemaphoreGive(mutex);

    vTaskDelay(1000 * 10 / portTICK_PERIOD_MS); // 10 seconds
  }
}