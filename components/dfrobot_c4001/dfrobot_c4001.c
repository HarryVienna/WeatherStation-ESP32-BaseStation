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


esp_err_t write_reg_i2c(DFRobot_C4001_t *sensor, uint8_t reg, uint8_t *data, uint8_t len) {
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (sensor->i2c_addr << 1) | I2C_MASTER_WRITE, true); // Write address and set for writing
    i2c_master_write_byte(cmd, reg, true); // Write register address
    i2c_master_write(cmd, data, len, true); // Write data
    i2c_master_stop(cmd);
    esp_err_t ret = i2c_master_cmd_begin(sensor->i2c_port, cmd, portMAX_DELAY);
    i2c_cmd_link_delete(cmd);
    return ret;
}

esp_err_t read_reg_i2c(DFRobot_C4001_t *sensor, uint8_t reg, uint8_t *data, uint8_t len) {
    if (len == 0) {
        return ESP_OK; // No data to read
    }

    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (sensor->i2c_addr << 1) | I2C_MASTER_WRITE, true); // Write address and set for writing
    i2c_master_write_byte(cmd, reg, true); // Write register address
    i2c_master_start(cmd); // Repeated start
    i2c_master_write_byte(cmd, (sensor->i2c_addr << 1) | I2C_MASTER_READ, true); // Set for reading
    i2c_master_read(cmd, data, len, I2C_MASTER_LAST_NACK); // Read data with NACK on last byte
    i2c_master_stop(cmd);
    esp_err_t ret = i2c_master_cmd_begin(sensor->i2c_port, cmd, portMAX_DELAY);
    i2c_cmd_link_delete(cmd);
    return ret;
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
  data.initStatus = (temp & 0x80) >> 7; // Correct bit mask and shift

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



bool dfrobot_c4001_set_detect_range(DFRobot_C4001_t *sensor, uint16_t min, uint16_t max, uint16_t trig) 
{
  if(max < 240 || max > 2000){
    return false;
  }
  if(min < 30 || min > max){
    return false;
  }

  uint8_t temp[10] = {0};
  temp[0] = (uint8_t)(min);
  temp[1] = (uint8_t)(min >> 8);
  temp[2] = (uint8_t)(max);
  temp[3] = (uint8_t)(max >> 8);
  temp[4] = (uint8_t)(trig);
  temp[5] = (uint8_t)(trig >> 8);
  write_reg_i2c(sensor, REG_E_MIN_RANGE_L, temp, (uint8_t)6);
  dfrobot_c4001_set_sensor(sensor, eSaveParams);

  return true;
}

uint16_t dfrobot_c4001_get_min_range(DFRobot_C4001_t *sensor){
  uint8_t temp[4] = {0};
  read_reg_i2c(sensor, REG_E_MIN_RANGE_L, temp, (uint8_t)2);
  return (uint16_t)(temp[0] | ((uint16_t)temp[1]) << 8);
}

uint16_t dfrobot_c4001_get_max_range(DFRobot_C4001_t *sensor){
  uint8_t temp[4] = {0};
  read_reg_i2c(sensor, REG_E_MAX_RANGE_L, temp, (uint8_t)2);
  return (uint16_t)(temp[0] | ((uint16_t)temp[1]) << 8);
}

uint16_t dfrobot_c4001_get_trig_range(DFRobot_C4001_t *sensor)
{
  uint8_t temp[4] = {0};
  read_reg_i2c(sensor, REG_E_TRIG_RANGE_L, temp, (uint8_t)2);
  return (uint16_t)(temp[0] | ((uint16_t)temp[1]) << 8);
}





void dfrobot_c4001_set_fretting_detection(DFRobot_C4001_t *sensor, eSwitch_t sta)
{
    uint8_t temp = sta;
    write_reg_i2c(sensor, REG_MICRO_MOTION, &temp, (uint8_t)1);
    dfrobot_c4001_set_sensor(sensor, eSaveParams);
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
  ESP_LOGI(TAG, "flash_number  = %d", flash_number);
  if(temp[0] == 1){
      flash_number = 0;
      sensor->buffer.number = 1;
      sensor->buffer.range  = (float)(int16_t)((uint16_t)(temp[1] | ((uint16_t)temp[2] << 8))) / 100.0;
      sensor->buffer.speed  = (float)(int16_t)((uint16_t)(temp[3] | ((uint16_t)temp[4] << 8))) / 100.0;
      sensor->buffer.energy = (uint16_t)(temp[5] | ((uint16_t)temp[6] << 8));

      ESP_LOGI(TAG, "range  = %f", sensor->buffer.range);
      ESP_LOGI(TAG, "speed  = %f", sensor->buffer.speed);
      ESP_LOGI(TAG, "energy  = %ld", sensor->buffer.energy);
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

bool dfrobot_c4001_motion_detection(DFRobot_C4001_t *sensor)
{
  uint8_t temp = 0;
  read_reg_i2c(sensor, REG_RESULT_STATUS, &temp, (uint8_t)1);
  if(temp&0x01){
    return true;
  }
  return false;
}

bool dfrobot_c4001_set_delay(DFRobot_C4001_t *sensor, uint8_t trig, uint16_t keep)
{
  if(trig > 200){
    return false;
  }
  if(keep < 4 || keep > 3000){
    return false;
  }

  uint8_t temp[3] = {0};
  temp[0] = trig;
  temp[1] = keep;
  temp[2] = keep>>8;
  write_reg_i2c(sensor, REG_TRIG_DELAY, temp, (uint8_t)3);
  dfrobot_c4001_set_sensor(sensor, eSaveParams);

  return true;
}

uint8_t dfrobot_c4001_get_trig_delay(DFRobot_C4001_t *sensor)
{
  uint8_t temp = 0;
  read_reg_i2c(sensor, REG_TRIG_DELAY, &temp, (uint8_t)1);
  return temp;
}

uint8_t dfrobot_c4001_get_keep_timerout(DFRobot_C4001_t *sensor)
{
  uint8_t temp[2] = {0};
  read_reg_i2c(sensor, REG_KEEP_TIMEOUT_L, temp, (uint8_t)2);
  return (((uint16_t)temp[1]) << 8) | temp[0];  
}

bool dfrobot_c4001_set_trig_sensitivity(DFRobot_C4001_t *sensor, uint8_t sensitivity)
{
  uint8_t temp = sensitivity;
  if(sensitivity > 9){
    return false;
  }

  write_reg_i2c(sensor, REG_TRIG_SENSITIVITY, &temp, (uint8_t)1);
  dfrobot_c4001_set_sensor(sensor, eSaveParams);
  return true;
}

uint8_t dfrobot_c4001_get_trig_sensitivity(DFRobot_C4001_t *sensor)
{
  uint8_t temp = 0;
  read_reg_i2c(sensor, REG_TRIG_SENSITIVITY, &temp, (uint8_t)1);
  return temp;
}

bool dfrobot_c4001_set_keep_sensitivity(DFRobot_C4001_t *sensor, uint8_t sensitivity)
{
  uint8_t temp = sensitivity;
  if(sensitivity > 9){
    return false;
  }

  write_reg_i2c(sensor, REG_KEEP_SENSITIVITY, &temp, (uint8_t)1);
  dfrobot_c4001_set_sensor(sensor, eSaveParams);
  return true;
}

uint8_t dfrobot_c4001_get_keep_sensitivity(DFRobot_C4001_t *sensor)
{
  uint8_t temp = 0;
  read_reg_i2c(sensor, REG_KEEP_SENSITIVITY, &temp, (uint8_t)1);
  return temp;
}


float dfrobot_c4001_get_target_speed(DFRobot_C4001_t *sensor)
{
  return sensor->buffer.speed;
}

float dfrobot_c4001_get_target_range(DFRobot_C4001_t *sensor)
{
  return sensor->buffer.range;
}

uint32_t dfrobot_c4001_get_target_energy(DFRobot_C4001_t *sensor)
{
  return sensor->buffer.energy;
}