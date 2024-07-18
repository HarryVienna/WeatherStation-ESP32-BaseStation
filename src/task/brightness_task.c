#include <stdio.h>
#include <string.h>

#include "esp_log.h"

#include "esp_adc/adc_oneshot.h"
#include "esp_adc/adc_cali.h"
#include "esp_adc/adc_cali_scheme.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "dfrobot_c4001.h"

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


  DFRobot_C4001_t sensor;
  dfrobot_c4001_init(&sensor, I2C_NUM, C4001_ADDR_0);

  // Set sensor mode
  //dfrobot_c4001_set_sensormode(&sensor, eResetSen);
  //dfrobot_c4001_set_sensormode(&sensor, eStartSen);
  dfrobot_c4001_set_sensormode(&sensor, eSpeedMode);

  sSensorStatus_t data;
  data = dfrobot_c4001_get_status(&sensor);
  
  //  0 stop  1 start
  ESP_LOGI(TAG, "work status  = %d", data.workStatus);

  //  0 is exist   1 speed
  ESP_LOGI(TAG, "work mode  = %d", data.workMode);

  //  0 no init    1 init success
  ESP_LOGI(TAG, "init status  = %d", data.initStatus);

  if (dfrobot_c4001_set_detect_thres(&sensor, /*min*/ 11, /*max*/ 1200, /*thres*/ 10 )) {
    ESP_LOGI(TAG, "set detect threshold successfully");
  }

  // set Fretting Detection
  dfrobot_c4001_set_fretting_detection(&sensor, eON);

  ESP_LOGI(TAG, "min range = %d", dfrobot_c4001_get_tmin_range(&sensor));
  ESP_LOGI(TAG, "max range = %d", dfrobot_c4001_get_tmax_range(&sensor));
  ESP_LOGI(TAG, "threshold range = %d", dfrobot_c4001_get_thres_range(&sensor));
  ESP_LOGI(TAG, "fretting detection = %d", dfrobot_c4001_get_fretting_detection(&sensor));

  for (;;) {
    ESP_LOGI(TAG, "target number = %d", dfrobot_c4001_get_target_number(&sensor));
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