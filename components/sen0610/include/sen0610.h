#ifndef __sen0610_H__
#define __sen0610_H__

#define C4001_ADDR_0 0x2A
#define C4001_ADDR_1 0x2B

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>
#include <driver/i2c.h>

/**
 * @struct sSensorStatus_t
 * @brief sensor status
 * @note sensor status
 */
typedef struct{
  uint8_t work_status;
  uint8_t work_mode;
  uint8_t init_status;
} sensor_status_t;

/**
 * @struct presence_data_t
 * @brief presence mode data
 */
typedef struct{
  bool presence;
  uint16_t range;
} presence_data_t;

/**
 * @struct speed_data_t
 * @brief speed mode data
 */
typedef struct{
  uint8_t number;
  int16_t speed;
  uint16_t range;
  uint16_t energy;
} speed_data_t;

/**
 * @enum eMode_t
 * @brief sensor work mode
 */
typedef enum{
  PRESENCE_MODE  = 0x00,
  SPEED_MODE     = 0x01,
} sensor_mode_t;

/**
 * @enum eSwitch_t
 * @brief Micromotion detection switch
 */
typedef enum{
  MICRO_ON  = 0x01,
  MICRO_OFF = 0x00,
} switch_t;

/**
 * @enum eSetMode_t
 * @brief Set parameters for the sensor working status
 */
typedef enum{
  START_SEN   = 0x55,
  STOP_SEN    = 0x33,
  RESET_SEN   = 0xCC,
  RECOVER_SEN = 0xAA,
  SAVE_PARAMS = 0x5C,
  CHANGE_MODE = 0x3B,
} set_cmd_t;

typedef struct {
    uint8_t i2c_addr;
    i2c_port_t i2c_port;
} sen0610_t;

/**
 * @fn sen0610_init
 * @brief Set I2C values
 * @param i2c_port
 * @param i2c_addr
 */
void sen0610_init(sen0610_t *sensor, i2c_port_t i2c_port, uint8_t i2c_addr);

/**
 * @fn getStatus
 * @brief Get the Status object
 * @return sSensorStatus_t 
 * @n     workStatus
 * @n       0 stop
 * @n       1 start
 * @n     workMode
 * @n       0 indicates presence detection
 * @n       1 is speed measurement and ranging
 * @n     initStatus
 * @n       0 not init
 * @n       1 init success
 */
sensor_status_t sen0610_get_status(sen0610_t *sensor);

/**
 * @fn sen0610_get_soft_version
 * @brief Get software version of C4001
 * @return software version
 */
uint32_t sen0610_get_soft_version(sen0610_t *sensor);

/**
 * @fn setSensor
 * @brief Set the Sensor object
 * @param mode
 * @n  START_SEN        start collect
 * @n  STOP_SEN         stop collect
 * @n  RESET_SEN        reset sensor
 * @n  RECOVER_SEN      recover params
 * @n  SAVE_PARAMS      save config
 * @n  CHANGE_MODE      change mode
 */
void sen0610_set_sensor(sen0610_t *sensor, set_cmd_t mode);

/**
 * @fn setSensorMode
 * @brief Set the Sensor Mode object
 * @param mode 
 * @n  eExitMode      presence detection mode
 * @n  eSpeedMode     speed and distance measurement mode
 * @return
 *     - ESP_OK Success
 *     - ESP_FAIL Error setting mode
 */
esp_err_t sen0610_set_mode(sen0610_t *sensor, sensor_mode_t mode);


// Presence Detection Mode functions 

/**
 * @fn setTrigSensitivity
 * @brief Set trigger sensitivity, 0~9
 * @param sensitivity 
 * @return
 *     - ESP_OK Success
 *     - ESP_FAIL Value out of range
 */
esp_err_t sen0610_set_trig_sensitivity(sen0610_t *sensor, uint8_t sensitivity);

/**
 * @fn getTrigSensitivity
 * @brief Get the Trig Sensitivity object
 * @return uint8_t 
 */
uint8_t sen0610_get_trig_sensitivity(sen0610_t *sensor);


/**
 * @fn setKeepSensitivity
 * @brief Set the Keep Sensitivity object，0~9
 * @param sensitivity 
 * @return
 *     - ESP_OK Success
 *     - ESP_FAIL Value out of range
 */
esp_err_t sen0610_set_keep_sensitivity(sen0610_t *sensor, uint8_t sensitivity);

/**
 * @fn getKeepSensitivity
 * @brief Get the Keep Sensitivity object
 * @return uint8_t 
 */
uint8_t sen0610_get_keep_sensitivity(sen0610_t *sensor);

/**
 * @fn setDelay
 * @brief Set the Delay object (REG_TRIG_DELAY, REG_KEEP_TIMEOUT_L, REG_KEEP_TIMEOUT_H)
 * @param trig Trigger delay, unit 0.01s, range 0~2s (0~200)
 * @param keep Maintain the detection timeout, unit 0.5s, range 2~1500 seconds (4~3000)
 * @return
 *     - ESP_OK Success
 *     - ESP_FAIL Value out of range
 */
esp_err_t sen0610_set_delay(sen0610_t *sensor, uint8_t trig, uint16_t keep);

/**
 * @fn getTrigDelay ((REG_TRIG_DELAY)
 * @brief Get the Trig Delay object
 * @return uint8_t 
 */
uint8_t sen0610_get_trig_delay(sen0610_t *sensor);

/**
 * @fn getKeepTimerout (REG_KEEP_TIMEOUT_L, REG_KEEP_TIMEOUT_H)
 * @brief get keep timer out
 * @return  uint16_t 
 */
uint8_t sen0610_get_keep_timerout(sen0610_t *sensor);

/**
 * @fn sen0610_set_detect_range
 * @brief Set the Detection Range object (REG_E_MIN_RANGE_L, REG_E_MIN_RANGE_H, REG_E_MAX_RANGE_L, REG_E_MAX_RANGE_H, REG_E_TRIG_RANGE_L, REG_E_TRIG_RANGE_H)
 * @param min Detection range Minimum distance, unit cm, range 0.3~20m (30~2000), not exceeding max, otherwise the function is abnormal.
 * @param max Detection range Maximum distance, unit cm, range 2.4~20m (240~2000)
 * @param trig The trigger distance (unit: cm) ranges from 2.4 to 20m (240 to 2000). The actual configuration range does not exceed the maximum and minimum detection distance.
 * @return
 *     - ESP_OK Success
 *     - ESP_FAIL Value out of range
 */
esp_err_t sen0610_set_detect_range(sen0610_t *sensor, uint16_t min, uint16_t max, uint16_t trig);

/**
 * @fn getMinRange
 * @brief Get the Min Range object (REG_E_MIN_RANGE_L, REG_E_MIN_RANGE_H)
 * @return uint16_t 
 */
uint16_t sen0610_get_min_range(sen0610_t *sensor);

/**
 * @fn getMaxRange
 * @brief Get the Max Range object (REG_E_MAX_RANGE_L, REG_E_MAX_RANGE_H)
 * @return  uint16_t 
 */
uint16_t sen0610_get_max_range(sen0610_t *sensor);

/**
 * @fn getTrigRange
 * @brief Get the Trig Range object (REG_E_TRIG_RANGE_L, REG_E_TRIG_RANGE_H)
 * @n     The triggering distance, in cm, ranges from 2.4 to 20m (240 to 2000). 
 * @n     The actual configuration range does not exceed the maximum and minimum detection distance.
 * @return uint16_t 
 */
uint16_t sen0610_get_trig_range(sen0610_t *sensor);

/**
 * @fn motionDetection
 * @brief presence Detection
 * @param presence_data Struct holding boolean value and range
 */
void sen0610_get_presence_status(sen0610_t *sensor, presence_data_t *presence_data);



// Velocimetry and Ranging Mode functions 

/**
 * @fn sen0610_set_detect_thres
 * @brief Set the Detect Thres object (REG_CFAR_THR_L, REG_CFAR_THR_H, REG_T_MIN_RANGE_L, REG_T_MIN_RANGE_H, REG_T_MAX_RANGE_L, REG_T_MAX_RANGE_H)

 * @param min Detection range Minimum distance, unit cm, range 0.3~13m (30~1300), not exceeding max, otherwise the function is abnormal.
 * @param max Detection range Maximum distance, unit cm, range 2.4~13m (240~1300)
 * @param thres Target detection threshold, dimensionless unit 0.1, range 0~6553.5 (0~65535)
 * @return
 *     - ESP_OK Success
 *     - ESP_FAIL Value out of range
 */
esp_err_t sen0610_set_detect_thres(sen0610_t *sensor, uint16_t min, uint16_t max, uint16_t thres);

/**
 * @fn getTMinRange
 * @brief get speed Min Range (REG_T_MIN_RANGE_L, REG_T_MIN_RANGE_H)
 * @return uint16_t 
 */
uint16_t sen0610_get_tmin_range(sen0610_t *sensor);

/**
 * @fn getTMaxRange
 * @brief get speed Max Range (REG_T_MAX_RANGE_L, REG_T_MAX_RANGE_H)
 * @return uint16_t 
 */
uint16_t sen0610_get_tmax_range(sen0610_t *sensor);

/**
 * @fn getThresRange
 * @brief Get the Thres Range object (REG_CFAR_THR_L, REG_CFAR_THR_H)
 * @return uint16_t 
 */
uint16_t sen0610_get_thres_range(sen0610_t *sensor);






/**
 * @fn setFrettingDetection
 * @brief Set the Fretting Detection object
 * @param sta 
 */
void sen0610_set_micro_detection(sen0610_t *sensor, switch_t sta);

/**
 * @fn getFrettingDetection
 * @brief Get the Fretting Detection object
 * @return eSwitch_t 
 */
switch_t sen0610_get_micro_detection(sen0610_t *sensor);



/**
 * @fn getTargetNumber
 * @brief Get the Target Number object
 * @return uint8_t 
 */
void sen0610_get_speed_status(sen0610_t *sensor, speed_data_t *speed_data);
















#ifdef __cplusplus
}
#endif

#endif /* __sen0610_H__ */
