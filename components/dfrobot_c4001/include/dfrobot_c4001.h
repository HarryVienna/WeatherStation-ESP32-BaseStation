#ifndef __DFROBOT_C4001_H__
#define __DFROBOT_C4001_H__

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
 * @enum eMode_t
 * @brief sensor work mode
 */
typedef enum{
  eExistMode  = 0x00,
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
sSensorStatus_t dfrobot_c4001_get_status(DFRobot_C4001_t *sensor);

/**
 * @fn setSensor
 * @brief Set the Sensor object
 * @param mode
 * @n  eStartSen        start collect
 * @n  eStopSen         stop collect
 * @n  eResetSen        reset sensor
 * @n  eRecoverSen      recover params
 * @n  eSaveParams      save config
 * @n  eChangeMode      change mode
 */
void dfrobot_c4001_set_sensor(DFRobot_C4001_t *sensor, eSetMode_t mode);


/**
 * @fn setSensorMode
 * @brief Set the Sensor Mode object
 * @param mode 
 * @n  eExitMode      presence detection mode
 * @n  eSpeedMode     speed and distance measurement mode
 * @return true or false
 */
bool dfrobot_c4001_set_sensormode(DFRobot_C4001_t *sensor, eMode_t mode);

/**
 * @fn dfrobot_c4001_set_detect_thres
 * @brief Set the Detect Thres object (REG_CFAR_THR_L, REG_CFAR_THR_H, REG_T_MIN_RANGE_L, REG_T_MIN_RANGE_H, REG_T_MAX_RANGE_L, REG_T_MAX_RANGE_H)

 * @param min Detection range Minimum distance, unit cm, range 0.3~20m (30~2500), not exceeding max, otherwise the function is abnormal.
 * @param max Detection range Maximum distance, unit cm, range 2.4~20m (240~2500)
 * @param thres Target detection threshold, dimensionless unit 0.1, range 0~6553.5 (0~65535)
 * @return true or false
 */
bool dfrobot_c4001_set_detect_thres(DFRobot_C4001_t *sensor, uint16_t min, uint16_t max, uint16_t thres);

/**
 * @fn getTMinRange
 * @brief get speed Min Range (REG_T_MIN_RANGE_L, REG_T_MIN_RANGE_H)
 * @return uint16_t 
 */
uint16_t dfrobot_c4001_get_tmin_range(DFRobot_C4001_t *sensor);

/**
 * @fn getTMaxRange
 * @brief get speed Max Range (REG_T_MAX_RANGE_L, REG_T_MAX_RANGE_H)
 * @return uint16_t 
 */
uint16_t dfrobot_c4001_get_tmax_range(DFRobot_C4001_t *sensor);

/**
 * @fn getThresRange
 * @brief Get the Thres Range object (REG_CFAR_THR_L, REG_CFAR_THR_H)
 * @return uint16_t 
 */
uint16_t dfrobot_c4001_get_thres_range(DFRobot_C4001_t *sensor);

/**
 * @fn dfrobot_c4001_set_detect_range
 * @brief Set the Detection Range object (REG_E_MIN_RANGE_L, REG_E_MIN_RANGE_H, REG_E_MAX_RANGE_L, REG_E_MAX_RANGE_H, REG_E_TRIG_RANGE_L, REG_E_TRIG_RANGE_H)
 * @param min Detection range Minimum distance, unit cm, range 0.3~20m (30~2000), not exceeding max, otherwise the function is abnormal.
 * @param max Detection range Maximum distance, unit cm, range 2.4~20m (240~2000)
 * @param trig The trigger distance (unit: cm) ranges from 2.4 to 20m (240 to 2000). The actual configuration range does not exceed the maximum and minimum detection distance.
 * @return true or false
 */
bool dfrobot_c4001_set_detect_range(DFRobot_C4001_t *sensor, uint16_t min, uint16_t max, uint16_t trig);

/**
 * @fn getMinRange
 * @brief Get the Min Range object (REG_E_MIN_RANGE_L, REG_E_MIN_RANGE_H)
 * @return uint16_t 
 */
uint16_t dfrobot_c4001_get_min_range(DFRobot_C4001_t *sensor);

/**
 * @fn getMaxRange
 * @brief Get the Max Range object (REG_E_MAX_RANGE_L, REG_E_MAX_RANGE_H)
 * @return  uint16_t 
 */
uint16_t dfrobot_c4001_get_max_range(DFRobot_C4001_t *sensor);

/**
 * @fn getTrigRange
 * @brief Get the Trig Range object (REG_E_TRIG_RANGE_L, REG_E_TRIG_RANGE_H)
 * @n     The triggering distance, in cm, ranges from 2.4 to 20m (240 to 2000). 
 * @n     The actual configuration range does not exceed the maximum and minimum detection distance.
 * @return uint16_t 
 */
uint16_t dfrobot_c4001_get_trig_range(DFRobot_C4001_t *sensor);



/**
 * @fn setFrettingDetection
 * @brief Set the Fretting Detection object
 * @param sta 
 */
void dfrobot_c4001_set_fretting_detection(DFRobot_C4001_t *sensor, eSwitch_t sta);

/**
 * @fn getFrettingDetection
 * @brief Get the Fretting Detection object
 * @return eSwitch_t 
 */
eSwitch_t dfrobot_c4001_get_fretting_detection(DFRobot_C4001_t *sensor);



/**
 * @fn getTargetNumber
 * @brief Get the Target Number object
 * @return uint8_t 
 */
uint8_t dfrobot_c4001_get_target_number(DFRobot_C4001_t *sensor);




/**
 * @fn motionDetection
 * @brief motion Detection
 * @return true or false
 */
bool dfrobot_c4001_motion_detection(DFRobot_C4001_t *sensor);




/**
 * @fn setDelay
 * @brief Set the Delay object (REG_TRIG_DELAY, REG_KEEP_TIMEOUT_L, REG_KEEP_TIMEOUT_H)
 * @param trig Trigger delay, unit 0.01s, range 0~2s (0~200)
 * @param keep Maintain the detection timeout, unit 0.5s, range 2~1500 seconds (4~3000)
 * @return true or false
 */
bool dfrobot_c4001_set_delay(DFRobot_C4001_t *sensor, uint8_t trig, uint16_t keep);

/**
 * @fn getTrigDelay ((REG_TRIG_DELAY)
 * @brief Get the Trig Delay object
 * @return uint8_t 
 */
uint8_t dfrobot_c4001_get_trig_delay(DFRobot_C4001_t *sensor);

/**
 * @fn getKeepTimerout (REG_KEEP_TIMEOUT_L, REG_KEEP_TIMEOUT_H)
 * @brief get keep timer out
 * @return  uint16_t 
 */
uint8_t dfrobot_c4001_get_keep_timerout(DFRobot_C4001_t *sensor);





/**
 * @fn setTrigSensitivity
 * @brief Set trigger sensitivity, 0~9
 * @param sensitivity 
 * @return true or false
 */
bool dfrobot_c4001_set_trig_sensitivity(DFRobot_C4001_t *sensor, uint8_t sensitivity);

/**
 * @fn getTrigSensitivity
 * @brief Get the Trig Sensitivity object
 * @return uint8_t 
 */
uint8_t dfrobot_c4001_get_trig_sensitivity(DFRobot_C4001_t *sensor);


/**
 * @fn setKeepSensitivity
 * @brief Set the Keep Sensitivity object，0~9
 * @param sensitivity 
 * @return true or false
 */
bool dfrobot_c4001_set_keep_sensitivity(DFRobot_C4001_t *sensor, uint8_t sensitivity);

/**
 * @fn getKeepSensitivity
 * @brief Get the Keep Sensitivity object
 * @return uint8_t 
 */
uint8_t dfrobot_c4001_get_keep_sensitivity(DFRobot_C4001_t *sensor);



/**
 * @fn getTargetSpeed
 * @brief Get the Target Speed object
 * @return float 
 */
float dfrobot_c4001_get_target_speed(DFRobot_C4001_t *sensor);

/**
 * @fn getTargetRange
 * @brief Get the Target Range object
 * @return float 
 */
float dfrobot_c4001_get_target_range(DFRobot_C4001_t *sensor);

/**
 * @fn getTargetEnergy
 * @brief Get the Target Energy object
 * @return uint32_t 
 */
uint32_t dfrobot_c4001_get_target_energy(DFRobot_C4001_t *sensor);

#ifdef __cplusplus
}
#endif

#endif /* __DFROBOT_C4001_H__ */
