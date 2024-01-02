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

/**
 * @brief     Map sensor value to a corresponding brightness level
 *
 * @param     value   Sensor value to be mapped to brightness
 *
 * @return    uint8_t The mapped brightness level
 *
 * @details   Maps sensor values within a specified range to corresponding brightness levels.
 *            Uses logarithmic scaling to convert sensor values to a suitable brightness scale.
 */
uint8_t map_brightness(uint16_t value) {

  const uint16_t adc_from = 0;     // Set these two values according to the LDR used. The first value for very bright surroundings, the second value for darkness
  const uint16_t adc_to   = 1200;  

  const float brightness_from = log(255);   // Brightness levels
  const float brightness_to   = log(10);

  if (value < adc_from) {
    value = adc_from;
  }
  else  if (value > adc_to) {
    value = adc_to;
  }

  const float scale = (brightness_to - brightness_from) / (adc_to - adc_from);

  uint8_t brightness = (uint8_t)exp(brightness_from + scale * (value - adc_from));

  //log_i("Sensor value %d    Mapped value %d", value, brightness); 

  return brightness;
}


/**
 * @brief     Task for adjusting brightness based on sensor readings
 *
 * @param     pvParameter   Pointer to task parameters (not used in this function)
 *
 * @details   Monitors sensor values and adjusts brightness levels accordingly.
 *            Uses hysteresis thresholds to control smooth transitions in brightness changes.
 *            Implements a loop to continuously monitor and update brightness levels.
 */
void brightness_task(void *pvParameter){

  // Hysteresis thresholds
  const uint8_t threshold = 10; 

  uint8_t target_brightness, current_brightness;   

  current_brightness = 127;

  for (;;) {

    uint16_t sensorVal = analogRead(ADC_PIN);

    target_brightness = map_brightness(sensorVal);

  
    int16_t brightness_difference = target_brightness - current_brightness;

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