/*
 * Copyright (c) 2017 Andrej Krutak <dev@andree.sk>
 * Copyright (c) 2018 Ruslan V. Uss <unclerus@gmail.com>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. Neither the name of the copyright holder nor the names of itscontributors
 *    may be used to endorse or promote products derived from this software without
 *    specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/**
 * @file bh1750.c
 *
 * @ingroup bh1750 ESP-IDF driver for BH1750 light sensor
 *
 * ESP-IDF driver for BH1750 light sensor
 *
 * Datasheet: ROHM Semiconductor bh1750fvi-e.pdf
 *
 * Ported from esp-open-rtos
 *
 * Copyright (c) 2017 Andrej Krutak <dev@andree.sk>\n
 * Copyright (c) 2018 Ruslan V. Uss <unclerus@gmail.com>
 *
 * BSD Licensed as described in the file LICENSE
 */
#include <stdio.h>
#include <esp_log.h>
#include "bh1750.h"


static const char *TAG = "bh1750";


static esp_err_t write_reg_i2c(bh_1750_t *sensor, uint8_t *data, uint8_t len) {
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();

    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (sensor->i2c_addr << 1) | I2C_MASTER_WRITE, true); // Write address and set for writing
    i2c_master_write(cmd, data, len, true); // Write data
    i2c_master_stop(cmd);
    esp_err_t ret = i2c_master_cmd_begin(sensor->i2c_port, cmd, portMAX_DELAY);
    i2c_cmd_link_delete(cmd);
    return ret;
}

static esp_err_t read_reg_i2c(bh_1750_t *sensor, uint8_t *data, uint8_t len) {
    if (len == 0) {
        return ESP_OK; // No data to read
    }

    i2c_cmd_handle_t cmd = i2c_cmd_link_create();

    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (sensor->i2c_addr << 1) | I2C_MASTER_READ, true); // Set for reading
    i2c_master_read(cmd, data, len, I2C_MASTER_LAST_NACK); // Read data with NACK on last byte
    i2c_master_stop(cmd);
    esp_err_t ret = i2c_master_cmd_begin(sensor->i2c_port, cmd, portMAX_DELAY);
    i2c_cmd_link_delete(cmd);
    return ret;
}

esp_err_t bh1750_init(bh_1750_t *sensor, i2c_port_t i2c_port, uint8_t i2c_addr) {
  sensor->i2c_port = i2c_port;
  sensor->i2c_addr = i2c_addr;
  return ESP_OK;
}


esp_err_t bh1750_power_down(bh_1750_t *sensor)
{
    uint8_t cmd = POWER_DOWN;
    write_reg_i2c(sensor, &cmd, 1);

    return ESP_OK;
}

esp_err_t bh1750_power_on(bh_1750_t *sensor)
{
    uint8_t cmd = POWER_UP;
    write_reg_i2c(sensor, &cmd, 1);

    return ESP_OK;
}

esp_err_t bh1750_send_opcode(bh_1750_t *sensor, bh1750_opcode_t opcode)
{
    uint8_t cmd = opcode;
    write_reg_i2c(sensor, &cmd, 1);

    vTaskDelay(pdMS_TO_TICKS(180));

    return ESP_OK;
}

esp_err_t bh1750_set_measure_time(bh_1750_t *sensor, uint8_t time)
{
    if (time < 31 || time > 254) {
        return ESP_FAIL;
    }

    uint8_t cmd_high = CHANGE_MEASURE_TIME_HIGH | (time >> 5);
    uint8_t cmd_low = CHANGE_MEASURE_TIME_LOW | (time & 0x1f);

    write_reg_i2c(sensor, &cmd_high, 1);
    write_reg_i2c(sensor, &cmd_low, 1);

    vTaskDelay(pdMS_TO_TICKS(180));

    return ESP_OK;
}

esp_err_t bh1750_read(bh_1750_t *sensor, uint16_t *lux)
{
    uint8_t buf[2];

    read_reg_i2c(sensor, buf, 2);

    *lux = buf[0] << 8 | buf[1];
    *lux = (*lux * 10) / 12; // = division by 1.2

    return ESP_OK;
}

