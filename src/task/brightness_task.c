#include <stdio.h>
#include <string.h>

#include "esp_log.h"

#include "esp_adc/adc_oneshot.h"
#include "esp_adc/adc_cali.h"
#include "esp_adc/adc_cali_scheme.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "sen0610.h"
#include "bh1750.h"

#include "brightness_task.h"

#include "config/config.h"

// Pin 19 --> https://www.espressif.com/sites/default/files/documentation/esp32-s3_technical_reference_manual_en.pdf#subsection.39.3
#define ADC_CHANNEL ADC_CHANNEL_8   
#define ADC_UNIT ADC_UNIT_2
#define ADC_ATTEN ADC_ATTEN_DB_12

static const char* TAG = "brightness_task";

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


/*
  bh_1750_t lux_sensor;
  bh1750_init(&lux_sensor, I2C_NUM, BH1750_ADDR_0);

  bh1750_power_on(&lux_sensor);
  bh1750_set_measure_time(&lux_sensor, 254);
  bh1750_send_opcode(&lux_sensor, CONT_HIGH_MODE);

  uint16_t lux;
  for (;;) {
    bh1750_read(&lux_sensor, &lux);

    vTaskDelay(pdMS_TO_TICKS(200));
    ESP_LOGI(TAG, "lux  = %d", lux);

  }

*/


  sen0610_t sensor;
  sen0610_init(&sensor, I2C_NUM, C4001_ADDR_0);

  sen0610_set_sensor(&sensor, RECOVER_SEN);
  vTaskDelay(pdMS_TO_TICKS(1500));

  uint32_t soft_version = sen0610_get_soft_version(&sensor);
  ESP_LOGI(TAG, "Software version  = %lu", soft_version);



  // Set sensor mode
  sen0610_set_mode(&sensor, PRESENCE_MODE);

  sensor_status_t data;
  data = sen0610_get_status(&sensor);
  
  //  0 stop  1 start
  ESP_LOGI(TAG, "work status  = %d", data.work_status);

  //  0 is presence   1 speed
  ESP_LOGI(TAG, "work mode  = %d", data.work_mode);

  //  0 no init    1 init success
  ESP_LOGI(TAG, "init status  = %d", data.init_status);


  if(sen0610_set_detect_range(&sensor, /*min*/30, /*max*/400, /*trig*/300)){
    ESP_LOGI(TAG, "set detection range successfully");
  }

  // set trigger sensitivity 0 - 9
  if(sen0610_set_trig_sensitivity(&sensor, 2)){
    ESP_LOGI(TAG, "set trig sensitivity successfully");
  }

  // set keep sensitivity 0 - 9
  if(sen0610_set_keep_sensitivity(&sensor, 2)){
    ESP_LOGI(TAG, "set keep sensitivity successfully");
  }

  ESP_LOGI(TAG, "min range = %d", sen0610_get_min_range(&sensor));
  ESP_LOGI(TAG, "max range = %d", sen0610_get_max_range(&sensor));
  ESP_LOGI(TAG, "trigger range = %d", sen0610_get_trig_range(&sensor));

  ESP_LOGI(TAG, "trigger sensitivity = %d", sen0610_get_trig_sensitivity(&sensor));
  ESP_LOGI(TAG, "keep sensitivity = %d", sen0610_get_keep_sensitivity(&sensor));

  presence_data_t presence_data;
  for (;;) {
    sen0610_get_presence_status(&sensor, &presence_data);

    ESP_LOGI(TAG, "Presence %d   Distance %d", presence_data.presence, presence_data.range);

    vTaskDelay(pdMS_TO_TICKS(250)); // Sleep for 1 second
  }




  // Set sensor mode
  sen0610_set_mode(&sensor, SPEED_MODE);

  ESP_LOGI(TAG, "speed min range = %d", sen0610_get_tmin_range(&sensor));
  ESP_LOGI(TAG, "speed max range = %d", sen0610_get_tmax_range(&sensor));
  ESP_LOGI(TAG, "threshold range = %d", sen0610_get_thres_range(&sensor));

  //sensor_status_t data;
  data = sen0610_get_status(&sensor);
  
  //  0 stop  1 start
  ESP_LOGI(TAG, "work status  = %d", data.work_status);

  //  0 is presence   1 speed
  ESP_LOGI(TAG, "work mode  = %d", data.work_mode);

  //  0 no init    1 init success
  ESP_LOGI(TAG, "init status  = %d", data.init_status);

  if (sen0610_set_detect_thres(&sensor, /*min*/ 30, /*max*/ 1000, /*thres*/ 400 )) {
    ESP_LOGI(TAG, "set detect threshold successfully");
  }

  // set Fretting Detection
  sen0610_set_micro_detection(&sensor, MICRO_OFF);

  
  ESP_LOGI(TAG, "speed min range = %d", sen0610_get_tmin_range(&sensor));
  ESP_LOGI(TAG, "speed max range = %d", sen0610_get_tmax_range(&sensor));
  ESP_LOGI(TAG, "threshold range = %d", sen0610_get_thres_range(&sensor));
  
  ESP_LOGI(TAG, "micro detection = %d", sen0610_get_micro_detection(&sensor));

  speed_data_t speed_data;
  for (;;) {
    sen0610_get_speed_status(&sensor, &speed_data);

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