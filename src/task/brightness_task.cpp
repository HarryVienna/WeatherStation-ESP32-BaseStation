#include <Arduino.h>

#include <stdio.h>
#include <string.h>
#include <time.h>
#include <mutex>

#include "esp_log.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "brightness_task.h"

#include "../gui/gui.h"
#include "../config/config.h"

static const char* TAG = "brightness_task";

uint8_t map_brightness(float value) {
  
  const float adc_from = 800;   // Values from ADC 800 = very bright, 4000 = very dark
  const float adc_to   = 4000;  

  const float brightness_from = log(255);   // Brightness levels
  const float brightness_to   = log(8);

  if (value < adc_from) {
    value = adc_from;
  }
  else  if (value > adc_to) {
    value = adc_to;
  }

  const float scale = (brightness_to - brightness_from) / (adc_to - adc_from);

  uint8_t brightness = (uint8_t)exp(brightness_from + scale * (value - adc_from));

  return brightness;
}


/**
 * @brief Main task for controling the hands 
 *
 */
void brightness_task(void *pvParameter){

  // Hysteresis thresholds
  const uint8_t threshold = 10; 

  uint8_t target_brightness, current_brightness;   

  current_brightness = 127;

  for (;;) {

    uint16_t sensorVal = analogRead(ADC_PIN);
    //log_i("%d", sensorVal);
    target_brightness = map_brightness(sensorVal);
    //log_i("%d %d", current_brightness, target_brightness);
  
    int8_t brightness_difference = target_brightness - current_brightness;

    if (abs(brightness_difference) >= threshold) {
      int direction = (brightness_difference > 0) ? 1 : -1;

      while (current_brightness != target_brightness) {
        current_brightness += direction;

        set_brightness(current_brightness);
        vTaskDelay(25 / portTICK_PERIOD_MS);
      }
    }

    vTaskDelay(500 / portTICK_PERIOD_MS); 
  }
}