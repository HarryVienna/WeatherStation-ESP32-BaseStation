#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

#include "sen5x_i2c.h"
#include "sensirion_common.h"
#include "sensirion_i2c_hal.h"

#include "gui/gui.h"


static const char* TAG = "sensor_task";

extern SemaphoreHandle_t lvgl_mux;

/**
 * @brief     Task for scanning nearby Wi-Fi networks
 *
 * @param     pvParameter   Pointer to task parameters (not used in this function)
 *
 * @details   Initiates Wi-Fi scanning to discover nearby networks and their signal strengths.
 *            Prints the scanned networks and their information.
 *            Displays the scanned Wi-Fi networks on the device's display.
 *            Deletes the task once the scan is complete.
 */
void sensor_task(void *pvParameter) {
    ESP_LOGI(TAG, "Start Sensor task");
    
    int16_t error = 0;

    error = sen5x_device_reset();
    if (error) {
        printf("Error executing sen5x_device_reset(): %i\n", error);
    }

    unsigned char serial_number[32];
    uint8_t serial_number_size = 32;
    error = sen5x_get_serial_number(serial_number, serial_number_size);
    if (error) {
        printf("Error executing sen5x_get_serial_number(): %i\n", error);
    } else {
        printf("Serial number: %s\n", serial_number);
    }

    unsigned char product_name[32];
    uint8_t product_name_size = 32;
    error = sen5x_get_product_name(product_name, product_name_size);
    if (error) {
        printf("Error executing sen5x_get_product_name(): %i\n", error);
    } else {
        printf("Product name: %s\n", product_name);
    }

    uint8_t firmware_major;
    uint8_t firmware_minor;
    bool firmware_debug;
    uint8_t hardware_major;
    uint8_t hardware_minor;
    uint8_t protocol_major;
    uint8_t protocol_minor;
    error = sen5x_get_version(&firmware_major, &firmware_minor, &firmware_debug,
                              &hardware_major, &hardware_minor, &protocol_major,
                              &protocol_minor);

    if (error) {
        printf("Error executing sen5x_get_version(): %i\n", error);
    } else {
        printf("Firmware: %u.%u, Hardware: %u.%u\n", firmware_major,
               firmware_minor, hardware_major, hardware_minor);
    }

    // set a temperature offset - supported by SEN54 and SEN55 sensors
    //
    // By default, the temperature and humidity outputs from the sensor
    // are compensated for the modules self-heating. If the module is
    // designed into a device, the temperature compensation might need
    // to be adapted to incorporate the change in thermal coupling and
    // self-heating of other device components.
    //
    // A guide to achieve optimal performance, including references
    // to mechanical design-in examples can be found in the app note
    // “SEN5x – Temperature Compensation Instruction” at www.sensirion.com.
    // Please refer to those application notes for further information
    // on the advanced compensation settings used in
    // `sen5x_set_temperature_offset_parameters`,
    // `sen5x_set_warm_start_parameter` and
    // `sen5x_set_rht_acceleration_mode`.
    //
    // Adjust temp_offset in degrees celsius to account for additional
    // temperature offsets exceeding the SEN module's self heating.
    float temp_offset = 0.0f;
    int16_t default_slope = 0;
    uint16_t default_time_constant = 0;
    error = sen5x_set_temperature_offset_parameters(
        (int16_t)(200 * temp_offset), default_slope, default_time_constant);
    if (error) {
        printf(
            "Error executing sen5x_set_temperature_offset_parameters(): %i\n",
            error);
    } else {
        printf("Temperature Offset set to %.2f °C (SEN54/SEN55 only)\n",
               temp_offset);
    }

    // Start Measurement
    error = sen5x_start_measurement();
    if (error) {
        printf("Error executing sen5x_start_measurement(): %i\n", error);
    }

    for (int i = 0; i < 60; i++) {
        // Read Measurement
        sensirion_i2c_hal_sleep_usec(1000000);

        uint16_t mass_concentration_pm1p0;
        uint16_t mass_concentration_pm2p5;
        uint16_t mass_concentration_pm4p0;
        uint16_t mass_concentration_pm10p0;
        int16_t ambient_humidity;
        int16_t ambient_temperature;
        int16_t voc_index;
        int16_t nox_index;

        error = sen5x_read_measured_values(
            &mass_concentration_pm1p0, &mass_concentration_pm2p5,
            &mass_concentration_pm4p0, &mass_concentration_pm10p0,
            &ambient_humidity, &ambient_temperature, &voc_index, &nox_index);

        if (error) {
            printf("Error executing sen5x_read_measured_values(): %i\n", error);
        } else {
            printf("Mass concentration pm1p0: %.1f µg/m³\n",
                   mass_concentration_pm1p0 / 10.0f);
            printf("Mass concentration pm2p5: %.1f µg/m³\n",
                   mass_concentration_pm2p5 / 10.0f);
            printf("Mass concentration pm4p0: %.1f µg/m³\n",
                   mass_concentration_pm4p0 / 10.0f);
            printf("Mass concentration pm10p0: %.1f µg/m³\n",
                   mass_concentration_pm10p0 / 10.0f);
            if (ambient_humidity == 0x7fff) {
                printf("Ambient humidity: n/a\n");
            } else {
                printf("Ambient humidity: %.1f %%RH\n",
                       ambient_humidity / 100.0f);
            }
            if (ambient_temperature == 0x7fff) {
                printf("Ambient temperature: n/a\n");
            } else {
                printf("Ambient temperature: %.1f °C\n",
                       ambient_temperature / 200.0f);
            }
            if (voc_index == 0x7fff) {
                printf("Voc index: n/a\n");
            } else {
                printf("Voc index: %.1f\n", voc_index / 10.0f);
            }
            if (nox_index == 0x7fff) {
                printf("Nox index: n/a\n");
            } else {
                printf("Nox index: %.1f\n", nox_index / 10.0f);
            }
        }
    }

    error = sen5x_stop_measurement();
    if (error) {
        printf("Error executing sen5x_stop_measurement(): %i\n", error);
    }

    

    vTaskDelete(NULL); // Delete the task when done
}