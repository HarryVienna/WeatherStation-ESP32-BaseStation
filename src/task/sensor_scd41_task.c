#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

#include "scd4x_i2c.h"
#include "sensirion_common.h"
#include "sensirion_i2c_hal.h"

#include "gui/gui.h"

static const char* TAG = "sensor_scd41_task";

extern SemaphoreHandle_t lvgl_mux;

/**
 * @brief     Task for reading sensor data and displaying information
 *
 * @param     pvParameter   Pointer to task parameters (not used in this function)
 *
 * @details   Initializes and reads data from two sensors (SCD41).
 *            Monitors and retrieves sensor measurements, displaying them periodically.
 */
void sensor_scd41_task(void *pvParameter) {
    ESP_LOGI(TAG, "Start Sensor SCD41 task");
    
    int16_t error = 0;

    //sensirion_i2c_hal_init();

    // Clean up potential SCD40 states
    scd4x_wake_up();
    scd4x_stop_periodic_measurement();
    scd4x_reinit();

    uint16_t serial_0;
    uint16_t serial_1;
    uint16_t serial_2;
    error = scd4x_get_serial_number(&serial_0, &serial_1, &serial_2);
    if (error) {
        ESP_LOGE(TAG, "Error executing scd4x_get_serial_number(): %i", error);
    } else {
        ESP_LOGI(TAG, "serial: 0x%04x%04x%04x", serial_0, serial_1, serial_2);
    }

    // Start Measurement

    error = scd4x_start_periodic_measurement();
    if (error) {
        ESP_LOGE(TAG, "Error executing scd4x_start_periodic_measurement(): %i", error);
    }

    for (;;) {
        // Read Measurement
        vTaskDelay(pdMS_TO_TICKS(1000 * 10));
        bool data_ready_flag = false;
        error = scd4x_get_data_ready_flag(&data_ready_flag);
        if (error) {
            ESP_LOGE(TAG, "Error executing scd4x_get_data_ready_flag(): %i", error);
            continue;
        }
        if (!data_ready_flag) {
            continue;
        }

        uint16_t co2;
        int32_t temperature;
        int32_t humidity;
        error = scd4x_read_measurement(&co2, &temperature, &humidity);
        if (error) {
            ESP_LOGE(TAG, "Error executing scd4x_read_measurement(): %i", error);
        } else if (co2 == 0) {
            ESP_LOGE(TAG, "Invalid sample detected, skipping.");
        } else {
            ESP_LOGI(TAG, "CO2: %u   Temperature: %.1f m°C   Humidity: %.1f mRH", co2, temperature/1000.0f, humidity/1000.0f);

            xSemaphoreTakeRecursive(lvgl_mux, portMAX_DELAY);
            disp_scd4x(co2);
            xSemaphoreGiveRecursive(lvgl_mux);
        }
    }

    

    vTaskDelete(NULL); // Delete the task when done
}