#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_err.h"
#include "esp_log.h"
#include "esp_check.h"
#include "driver/gpio.h"
#include "driver/i2c.h"

#include "dfrobot_c4001.h"



#define REG_STATUS              0x00
#define REG_CTRL0               0x01
#define REG_CTRL1               0x02
#define REG_SOFT_VERSION        0x03
#define REG_RESULT_STATUS       0x10
#define REG_TRIG_SENSITIVITY    0x20
#define REG_KEEP_SENSITIVITY    0x21
#define REG_TRIG_DELAY          0x22
#define REG_KEEP_TIMEOUT_L      0x23
#define REG_KEEP_TIMEOUT_H      0x24
#define REG_E_MIN_RANGE_L       0x25
#define REG_E_MIN_RANGE_H       0x26
#define REG_E_MAX_RANGE_L       0x27
#define REG_E_MAX_RANGE_H       0x28
#define REG_E_TRIG_RANGE_L      0x29
#define REG_E_TRIG_RANGE_H      0x2A
#define REG_RESULT_OBJ_MUN      0x10
#define REG_RESULT_RANGE_L      0x11
#define REG_RESULT_RANGE_H      0x12
#define REG_RESULT_SPEED_L      0x13
#define REG_RESULT_SPEED_H      0x14
#define REG_RESULT_ENERGY_L     0x15
#define REG_RESULT_ENERGY_H     0x16
#define REG_CFAR_THR_L          0x20
#define REG_CFAR_THR_H          0x21
#define REG_T_MIN_RANGE_L       0x22
#define REG_T_MIN_RANGE_H       0x23
#define REG_T_MAX_RANGE_L       0x24
#define REG_T_MAX_RANGE_H       0x25
#define REG_MICRO_MOTION        0x26

static const char *TAG = "C4001";


static void write_reg_i2c(DFRobot_C4001_t *sensor, uint8_t reg, uint8_t *data, uint8_t len) {
  ESP_ERROR_CHECK(i2c_master_write_to_device(sensor->i2c_port, sensor->i2c_addr, data, len, portMAX_DELAY));
}

static void read_reg_i2c(DFRobot_C4001_t *sensor, uint8_t reg, uint8_t *data, uint8_t len) {
  ESP_ERROR_CHECK(i2c_master_read_from_device(sensor->i2c_port, sensor->i2c_addr, data, len, portMAX_DELAY));
}

esp_err_t dfrobot_c4001_init(DFRobot_C4001_t *sensor, i2c_port_t i2c_port, uint8_t i2c_addr) {
  sensor->i2c_port = i2c_port;
  sensor->i2c_addr = i2c_addr;
  return ESP_OK;
}

sSensorStatus_t dfrobot_c4001_get_status(DFRobot_C4001_t *sensor) {
  sSensorStatus_t data;

  uint8_t temp = 0;
  read_reg_i2c(sensor, REG_STATUS, &temp, 1);
  data.workStatus = (temp & 0x01);
  data.workMode = (temp & 0x02) >> 1;
  data.initStatus = (temp & 0x04) >> 2;

  return data;
}

void dfrobot_c4001_set_sensor(DFRobot_C4001_t *sensor, eSetMode_t mode) {
  uint8_t temp = mode;

  if(mode == eStartSen){
    write_reg_i2c(sensor, REG_CTRL0, &temp, (uint8_t)1);
    vTaskDelay(pdMS_TO_TICKS(200));
  }else if(mode == eStopSen){
    write_reg_i2c(sensor, REG_CTRL0, &temp, (uint8_t)1);
    vTaskDelay(pdMS_TO_TICKS(200));
  }else if(mode == eResetSen){
    write_reg_i2c(sensor, REG_CTRL0, &temp, (uint8_t)1);
    vTaskDelay(pdMS_TO_TICKS(1500));
  }else if(mode == eSaveParams){
    write_reg_i2c(sensor, REG_CTRL1, &temp, (uint8_t)1);
    vTaskDelay(pdMS_TO_TICKS(500));
  }else if(mode == eRecoverSen){
    write_reg_i2c(sensor, REG_CTRL1, &temp, (uint8_t)1);
    vTaskDelay(pdMS_TO_TICKS(800));
  }else if(mode == eChangeMode){
    write_reg_i2c(sensor, REG_CTRL1, &temp, (uint8_t)1);
    vTaskDelay(pdMS_TO_TICKS(1500));
  }
}

bool dfrobot_c4001_set_sensormode(DFRobot_C4001_t *sensor, eMode_t mode) {
  sSensorStatus_t data;
  data = dfrobot_c4001_get_status(sensor);

  if(data.workMode == mode){
    return true;
  }else{
    dfrobot_c4001_set_sensor(sensor, eChangeMode);
    data = dfrobot_c4001_get_status(sensor);
    ESP_LOGI(TAG, "new work mode  = %d", data.workMode);
    if(data.workMode == mode){
      return true;
    }else{
      return false;
    }
  }      
}

bool dfrobot_c4001_set_detect_thres(DFRobot_C4001_t *sensor, uint16_t min, uint16_t max, uint16_t thres)
{
  if(max > 2500){
      return false;
  }
  if(min > max){
      return false;
  }

  uint8_t temp[10] = {0};
  temp[0] = (uint8_t)(thres);
  temp[1] = (uint8_t)(thres >> 8);
  temp[2] = (uint8_t)(min);
  temp[3] = (uint8_t)(min >> 8);
  temp[4] = (uint8_t)(max);
  temp[5] = (uint8_t)(max >> 8);
  write_reg_i2c(sensor, REG_CFAR_THR_L, temp, (uint8_t)6);
  dfrobot_c4001_set_sensor(sensor, eSaveParams);
  return true;
}

void dfrobot_c4001_set_fretting_detection(DFRobot_C4001_t *sensor, eSwitch_t sta)
{
    uint8_t temp = sta;
    write_reg_i2c(sensor, REG_MICRO_MOTION, &temp, (uint8_t)1);
    dfrobot_c4001_set_sensor(sensor, eSaveParams);
}

uint16_t dfrobot_c4001_get_tmin_range(DFRobot_C4001_t *sensor)
{
    uint8_t temp[4] = {0};
    read_reg_i2c(sensor, REG_T_MIN_RANGE_L, temp, (uint8_t)2);
    return (uint16_t)(temp[0] | ((uint16_t)temp[1]) << 8);
}

uint16_t dfrobot_c4001_get_tmax_range(DFRobot_C4001_t *sensor)
{
    uint8_t temp[4] = {0};
    read_reg_i2c(sensor, REG_T_MAX_RANGE_L, temp, (uint8_t)2);
    return (uint16_t)(temp[0] | ((uint16_t)temp[1]) << 8);
}

uint16_t dfrobot_c4001_get_thres_range(DFRobot_C4001_t *sensor)
{
    uint8_t temp[4] = {0};
    read_reg_i2c(sensor, REG_CFAR_THR_L, temp, (uint8_t)2);
    return (uint16_t)(temp[0] | ((uint16_t)temp[1]) << 8);
}

eSwitch_t dfrobot_c4001_get_fretting_detection(DFRobot_C4001_t *sensor)
{
    uint8_t temp = 0;
    read_reg_i2c(sensor, REG_MICRO_MOTION, &temp, (uint8_t)1);
    return (eSwitch_t)temp;
}

uint8_t dfrobot_c4001_get_target_number(DFRobot_C4001_t *sensor)
{
  static uint8_t flash_number = 0;
  uint8_t temp[10] = {0};
  read_reg_i2c(sensor, REG_RESULT_OBJ_MUN, temp, (uint8_t)7);
  ESP_LOGI(TAG, "REG_RESULT_OBJ_MUN  = %d", temp[0]);
  if(temp[0] == 1){
      flash_number = 0;
      sensor->buffer.number = 1;
      sensor->buffer.range  = (float)(int16_t)((uint16_t)(temp[1] | ((uint16_t)temp[2] << 8))) / 100.0;
      sensor->buffer.speed  = (float)(int16_t)((uint16_t)(temp[3] | ((uint16_t)temp[4] << 8))) / 100.0;
      sensor->buffer.energy = (uint16_t)(temp[5] | ((uint16_t)temp[6] << 8));
  }else{
      if(flash_number++ > 10){
      sensor->buffer.number = 0;
      sensor->buffer.range  = 0;
      sensor->buffer.speed  = 0;
      sensor->buffer.energy = 0;
      }
  }
  return sensor->buffer.number;
}