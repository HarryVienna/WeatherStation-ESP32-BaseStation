#ifndef __DFROBOT_C4001_H__
#define __DFROBOT_C4001_H__

#define C4001_ADDR_0 0x2A
#define C4001_ADDR_1 0x2B

#include <stdint.h>
#include <stdbool.h>
#include <driver/i2c.h>

/**
 * @struct sSensorStatus_t
 * @brief sensor status
 * @note sensor status
 */
typedef struct{
  uint8_t workStatus;
  uint8_t workMode;
  uint8_t initStatus;
}sSensorStatus_t;


/**
 * @struct sPrivateData_t
 * @brief speed mode data
 */
typedef struct{
  uint8_t number;
  float speed;
  float range;
  uint32_t energy;
}sPrivateData_t;

/**
 * @struct sResponseData_t
 * @brief response data
 */
typedef struct{
  bool status;
  float response1;
  float response2;
  float response3;
}sResponseData_t;


/**
 * @struct sPwmData_t
 * @brief config pwm data param
 */
typedef struct{
  uint8_t pwm1;
  uint8_t pwm2;
  uint8_t timer;
}sPwmData_t;

/**
 * @struct sAllData_t
 * @brief sensor return data
 */
typedef struct{
  uint8_t exist;
  sSensorStatus_t sta;
  sPrivateData_t target;
}sAllData_t;

/**
 * @enum eMode_t
 * @brief sensor work mode
 */
typedef enum{
  eExitMode  = 0x00,
  eSpeedMode = 0x01,
}eMode_t;

/**
 * @enum eSwitch_t
 * @brief Micromotion detection switch
 */
typedef enum{
  eON  = 0x01,
  eOFF = 0x00,
}eSwitch_t;

/**
 * @enum eSetMode_t
 * @brief Set parameters for the sensor working status
 */
typedef enum{
  eStartSen   = 0x55,
  eStopSen    = 0x33,
  eResetSen   = 0xCC,
  eRecoverSen = 0xAA,
  eSaveParams = 0x5C,
  eChangeMode = 0x3B,
}eSetMode_t;

typedef struct {
    uint8_t i2c_addr;
    i2c_port_t i2c_port;
    sPrivateData_t buffer;
} DFRobot_C4001_t;


esp_err_t dfrobot_c4001_init(DFRobot_C4001_t *sensor, i2c_port_t i2c_port, uint8_t i2c_addr);
sSensorStatus_t dfrobot_c4001_get_status(DFRobot_C4001_t *sensor);
void dfrobot_c4001_set_sensor(DFRobot_C4001_t *sensor, eSetMode_t mode);
bool dfrobot_c4001_set_sensormode(DFRobot_C4001_t *sensor, eMode_t mode);
bool dfrobot_c4001_set_detect_thres(DFRobot_C4001_t *sensor, uint16_t min, uint16_t max, uint16_t thres);
void dfrobot_c4001_set_fretting_detection(DFRobot_C4001_t *sensor, eSwitch_t sta);
uint16_t dfrobot_c4001_get_tmin_range(DFRobot_C4001_t *sensor);
uint16_t dfrobot_c4001_get_tmax_range(DFRobot_C4001_t *sensor);
uint16_t dfrobot_c4001_get_thres_range(DFRobot_C4001_t *sensor);
eSwitch_t dfrobot_c4001_get_fretting_detection(DFRobot_C4001_t *sensor);
uint8_t dfrobot_c4001_get_target_number(DFRobot_C4001_t *sensor);

#endif
