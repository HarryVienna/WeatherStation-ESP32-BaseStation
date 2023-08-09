#include <Arduino.h>

#include <SensirionI2CSen5x.h>
#include <SensirionI2CScd4x.h>
#include <Wire.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_attr.h"
#include "esp_log.h"

#include "sensor_task.h"

#include "../gui/gui.h"
#include "../config/config.h"



static const char* TAG = "sensor_task";


extern SemaphoreHandle_t mutex;



/**
 * @brief Main task for sensors
 *
 */
void sensor_task(void *pvParameter){

    uint16_t error;
    char errorMessage[256];

    
    SensirionI2CSen5x sen5x;
    SensirionI2CScd4x scd4x;

    /* -------- init SEN55 -------- */
    sen5x.begin(Wire1);

    error = sen5x.deviceReset();
    if (error) {
        errorToString(error, errorMessage, 256);
        log_e("Error trying to execute deviceReset(): %d  %s", error, errorMessage);
    }

    float tempOffset = -2.2;
    error = sen5x.setTemperatureOffsetSimple(tempOffset);
    if (error) {
        errorToString(error, errorMessage, 256);
        log_e("Error trying to execute setTemperatureOffsetSimple(): %d  %s", error, errorMessage);
    } 

    error = sen5x.startMeasurement();
    if (error) {
        errorToString(error, errorMessage, 256);
        log_e("Error trying to execute startMeasurement(): %d  %s", error, errorMessage);
    }


    /* -------- init SCD41 -------- */
    scd4x.begin(Wire1);

    error = scd4x.stopPeriodicMeasurement();
    if (error) {
        errorToString(error, errorMessage, 256);
        log_e("Error trying to execute stopPeriodicMeasurement(): %d  %s", error, errorMessage);
    }

    error = scd4x.startPeriodicMeasurement();
    if (error) {
        errorToString(error, errorMessage, 256);
        log_e("Error trying to execute startPeriodicMeasurement(): %d  %s", error, errorMessage);
    }



    for(;;) {

        /* -------- read SEN55 -------- */
        float massConcentrationPm1p0;
        float massConcentrationPm2p5;
        float massConcentrationPm4p0;
        float massConcentrationPm10p0;
        float ambientHumidity;
        float ambientTemperature;
        float vocIndex;
        float noxIndex;

        error = sen5x.readMeasuredValues(
            massConcentrationPm1p0, massConcentrationPm2p5, massConcentrationPm4p0, massConcentrationPm10p0, 
            ambientHumidity, ambientTemperature, 
            vocIndex, noxIndex);

        if (error) {
            errorToString(error, errorMessage, 256);
            log_e("Error trying to execute readMeasuredValues(): %d  %s", error, errorMessage);
        } else {
            log_i("Pm1p0: %f   Pm2p5: %f   Pm4p0: %f   Pm10p0: %f", massConcentrationPm1p0, massConcentrationPm2p5, massConcentrationPm4p0, massConcentrationPm10p0);
            log_i("VocIndex: %f   NoxIndex: %f   Temperature: %f   Humidity: %f", vocIndex, noxIndex, ambientTemperature, ambientHumidity);

            xSemaphoreTake(mutex, portMAX_DELAY);
            disp_sen5x(ambientTemperature, ambientHumidity, massConcentrationPm1p0, massConcentrationPm2p5, massConcentrationPm4p0, massConcentrationPm10p0, vocIndex, noxIndex);
            xSemaphoreGive(mutex);
        }

        /* -------- read SCD41 -------- */
        uint16_t co2;
        float temperature;
        float humidity;
        bool isDataReady = false;

        do {
            vTaskDelay(100 / portTICK_PERIOD_MS);
            error = scd4x.getDataReadyFlag(isDataReady);
            if (error) {
                errorToString(error, errorMessage, 256);
                log_e("Error trying to execute getDataReadyFlag(): %d  %s", error, errorMessage);
            }
        }
        while (!isDataReady);

        error = scd4x.readMeasurement(co2, temperature, humidity);
        if (error) {
            errorToString(error, errorMessage, 256);
            log_e("Error trying to execute readMeasurement(): %d  %s", error, errorMessage);
        } else if (co2 == 0) {
            log_e("Invalid sample detected, skipping.");
        } else {
            log_i("Co2: %d    Temperature: %f    Humidity: %f", co2, temperature, humidity);
            
            xSemaphoreTake(mutex, portMAX_DELAY);
            disp_scd4x(co2);
            xSemaphoreGive(mutex);
        }

        vTaskDelay(1000 * 60 / portTICK_PERIOD_MS); // Every 60 seconds
    }

}