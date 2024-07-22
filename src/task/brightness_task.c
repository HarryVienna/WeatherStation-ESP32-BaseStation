#include <stdio.h>
#include <string.h>

#include "esp_log.h"

#include "esp_adc/adc_oneshot.h"
#include "esp_adc/adc_cali.h"
#include "esp_adc/adc_cali_scheme.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "dfrobot_c4001.h"
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

  DFRobot_C4001_t sensor;
  dfrobot_c4001_init(&sensor, I2C_NUM, C4001_ADDR_0);

  dfrobot_c4001_set_sensor(&sensor, eRecoverSen);
  vTaskDelay(pdMS_TO_TICKS(1500));

  // Set sensor mode
  dfrobot_c4001_set_sensormode(&sensor, eExistMode);

  sSensorStatus_t data;
  data = dfrobot_c4001_get_status(&sensor);
  
  //  0 stop  1 start
  ESP_LOGI(TAG, "work status  = %d", data.workStatus);

  //  0 is exist   1 speed
  ESP_LOGI(TAG, "work mode  = %d", data.workMode);

  //  0 no init    1 init success
  ESP_LOGI(TAG, "init status  = %d", data.initStatus);

  // if (dfrobot_c4001_set_detect_thres(&sensor, /*min*/ 10, /*max*/ 1000, /*thres*/ 10 )) {
  //   ESP_LOGI(TAG, "set detect threshold successfully");
  // }

  if(dfrobot_c4001_set_detect_range(&sensor, /*min*/30, /*max*/1000, /*trig*/1000)){
    ESP_LOGI(TAG, "set detection range successfully");
  }

  // set trigger sensitivity 0 - 9
  if(dfrobot_c4001_set_trig_sensitivity(&sensor, 9)){
    ESP_LOGI(TAG, "set trig sensitivity successfully");
  }

  // set keep sensitivity 0 - 9
  if(dfrobot_c4001_set_keep_sensitivity(&sensor, 2)){
    ESP_LOGI(TAG, "set keep sensitivity successfully");
  }

  // set Fretting Detection
  // dfrobot_c4001_set_fretting_detection(&sensor, eON);

  ESP_LOGI(TAG, "speed min range = %d", dfrobot_c4001_get_tmin_range(&sensor));
  ESP_LOGI(TAG, "speed max range = %d", dfrobot_c4001_get_tmax_range(&sensor));
  ESP_LOGI(TAG, "threshold range = %d", dfrobot_c4001_get_thres_range(&sensor));
  
  ESP_LOGI(TAG, "min range = %d", dfrobot_c4001_get_min_range(&sensor));
  ESP_LOGI(TAG, "max range = %d", dfrobot_c4001_get_max_range(&sensor));
  ESP_LOGI(TAG, "trigger range = %d", dfrobot_c4001_get_trig_range(&sensor));

  ESP_LOGI(TAG, "trigger sensitivity = %d", dfrobot_c4001_get_trig_sensitivity(&sensor));
  ESP_LOGI(TAG, "keep sensitivity = %d", dfrobot_c4001_get_keep_sensitivity(&sensor));
  
  ESP_LOGI(TAG, "fretting detection = %d", dfrobot_c4001_get_fretting_detection(&sensor));

  for (;;) {
    //ESP_LOGI(TAG, "target number = %d", dfrobot_c4001_get_target_number(&sensor));
    if (dfrobot_c4001_motion_detection(&sensor)) {
      ESP_LOGI(TAG, "motion");
    }
    vTaskDelay(pdMS_TO_TICKS(1000)); // Sleep for 1 second
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