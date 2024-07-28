#include <stdio.h>
#include <string.h>
#include <math.h>

#include "esp_log.h"

#include "esp_adc/adc_oneshot.h"
#include "esp_adc/adc_cali.h"
#include "esp_adc/adc_cali_scheme.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "sen0610.h"
#include "bh1750.h"

#include "brightness_task.h"

#include "gui/gui.h"
#include "config/config.h"

// Pin 19 --> https://www.espressif.com/sites/default/files/documentation/esp32-s3_technical_reference_manual_en.pdf#subsection.39.3
#define ADC_CHANNEL ADC_CHANNEL_8   
#define ADC_UNIT ADC_UNIT_2
#define ADC_ATTEN ADC_ATTEN_DB_12

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
uint8_t map_brightness(uint16_t lux, bool presence) {

  float a = 63.0f;
  float b = -61.0f;

  uint8_t brightness = (uint8_t)(a * log10(lux) + b) * presence;

  //ESP_LOGI(TAG, "                 Mapped value %f %d", brightness, (uint8_t)brightness); 

  return (uint8_t)brightness;
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

  ESP_LOGI(TAG, "Start Brighness task");


  // Init Lux sensor
  bh_1750_t lux_sensor;
  bh1750_init(&lux_sensor, I2C_NUM, BH1750_ADDR_0);

  bh1750_power_on(&lux_sensor);
  bh1750_set_measure_time(&lux_sensor, 254);
  bh1750_send_opcode(&lux_sensor, CONT_HIGH_MODE);


  // Init presence sensor
  sen0610_t presence_sensor;
  sen0610_init(&presence_sensor, I2C_NUM, C4001_ADDR_0);

  sen0610_set_sensor(&presence_sensor, RECOVER_SEN);

  uint32_t soft_version = sen0610_get_soft_version(&presence_sensor);
  ESP_LOGI(TAG, "Software version  = %lu", soft_version);

  // Set sensor mode
  sen0610_set_mode(&presence_sensor, PRESENCE_MODE);

  if(sen0610_set_detect_range(&presence_sensor, /*min*/30, /*max*/400, /*trig*/300)){
    ESP_LOGI(TAG, "set detection range successfully");
  }

  // set trigger sensitivity 0 - 9
  if(sen0610_set_trig_sensitivity(&presence_sensor, 2)){
    ESP_LOGI(TAG, "set trig sensitivity successfully");
  }

  // set keep sensitivity 0 - 9
  if(sen0610_set_keep_sensitivity(&presence_sensor, 2)){
    ESP_LOGI(TAG, "set keep sensitivity successfully");
  }


  uint16_t lux;
  presence_data_t presence_data;

  // Hysteresis thresholds
  const uint8_t threshold = 10; 

  uint8_t target_brightness, current_brightness;   
  current_brightness = 127;

  for (;;) {

    bh1750_read(&lux_sensor, &lux);
    sen0610_get_presence_status(&presence_sensor, &presence_data);

    target_brightness = map_brightness(lux, presence_data.presence);

    ESP_LOGI(TAG, "current_brightness = %d   target_brightnessux  = %d", current_brightness, target_brightness);

    int16_t brightness_difference = target_brightness - current_brightness;

    if (abs(brightness_difference) >= threshold) {
      int direction = (brightness_difference > 0) ? 1 : -1;

      while (current_brightness != target_brightness) {
        current_brightness += direction;

        set_brightness(current_brightness);
        vTaskDelay(pdMS_TO_TICKS(25));
      }
    }

    //ESP_LOGI(TAG, "lux  = %d Presence %d   Distance %d", lux, presence_data.presence, presence_data.range);

    vTaskDelay(pdMS_TO_TICKS(250));
  }




  // Set sensor mode
  sen0610_set_mode(&presence_sensor, SPEED_MODE);

  ESP_LOGI(TAG, "speed min range = %d", sen0610_get_tmin_range(&presence_sensor));
  ESP_LOGI(TAG, "speed max range = %d", sen0610_get_tmax_range(&presence_sensor));
  ESP_LOGI(TAG, "threshold range = %d", sen0610_get_thres_range(&presence_sensor));

  sensor_status_t data;
  data = sen0610_get_status(&presence_sensor);
  
  //  0 stop  1 start
  ESP_LOGI(TAG, "work status  = %d", data.work_status);

  //  0 is presence   1 speed
  ESP_LOGI(TAG, "work mode  = %d", data.work_mode);

  //  0 no init    1 init success
  ESP_LOGI(TAG, "init status  = %d", data.init_status);

  if (sen0610_set_detect_thres(&presence_sensor, /*min*/ 30, /*max*/ 1000, /*thres*/ 400 )) {
    ESP_LOGI(TAG, "set detect threshold successfully");
  }

  // set Fretting Detection
  sen0610_set_micro_detection(&presence_sensor, MICRO_OFF);

  
  ESP_LOGI(TAG, "speed min range = %d", sen0610_get_tmin_range(&presence_sensor));
  ESP_LOGI(TAG, "speed max range = %d", sen0610_get_tmax_range(&presence_sensor));
  ESP_LOGI(TAG, "threshold range = %d", sen0610_get_thres_range(&presence_sensor));
  
  ESP_LOGI(TAG, "micro detection = %d", sen0610_get_micro_detection(&presence_sensor));

  speed_data_t speed_data;
  for (;;) {
    sen0610_get_speed_status(&presence_sensor, &speed_data);

    ESP_LOGI(TAG, "Number %d    Speed %d     Distance %d     Energy %d", speed_data.number, speed_data.speed, speed_data.range, speed_data.energy);

    vTaskDelay(pdMS_TO_TICKS(100)); // Sleep for 1 second
  }








  static int adc_raw;

  adc_oneshot_unit_handle_t adc_handle;
  adc_oneshot_unit_init_cfg_t init_config = {
      .unit_id = ADC_UNIT,
      .ulp_mode = ADC_ULP_MODE_DISABLE,
  };
  ESP_ERROR_CHECK(adc_oneshot_new_unit(&init_config, &adc_handle));

  adc_oneshot_chan_cfg_t config = {
      .atten = ADC_ATTEN,
      .bitwidth = ADC_BITWIDTH_DEFAULT,
  };
  ESP_ERROR_CHECK(adc_oneshot_config_channel(adc_handle, ADC_CHANNEL, &config));

  adc_cali_handle_t adc_cali_chan_handle = NULL;

  adc_cali_curve_fitting_config_t cali_config = {
      .unit_id = ADC_UNIT,
      .chan = ADC_CHANNEL,
      .atten = ADC_ATTEN,
      .bitwidth = ADC_BITWIDTH_DEFAULT,
  };
  ESP_ERROR_CHECK(adc_cali_create_scheme_curve_fitting(&cali_config, &adc_cali_chan_handle));

  int brightness;

  /*
  2100  without LDR = total dark
  
  */

  for (;;) {

    ESP_ERROR_CHECK(adc_oneshot_read(adc_handle, ADC_CHANNEL, &adc_raw));
    ESP_LOGI(TAG, "ADC%d Channel[%d] Raw Data: %d", ADC_UNIT + 1, ADC_CHANNEL, adc_raw);

    ESP_ERROR_CHECK(adc_cali_raw_to_voltage(adc_cali_chan_handle, adc_raw, &brightness));
    ESP_LOGI(TAG, "ADC%d Channel[%d] Cali Voltage: %d mV", ADC_UNIT + 1, ADC_CHANNEL, brightness);

    vTaskDelay(pdMS_TO_TICKS(1000)); // Sleep for 1 second
  }
}