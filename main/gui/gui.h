#ifndef GUI_H
#define GUI_H

#include "lvgl.h"
#include "ui/ui.h"
#include "display/esp32_s3.h"
#include "weather/open_meteo.h"

void disp_wifi_status(bool status);
void disp_date_time(char* date_time);
void disp_sensor_data(uint8_t sensor_nr, double temperature, double humidity, double pressure, uint32_t voltage, char* date_time);
void disp_scd4x(uint16_t co2);
void disp_sen5x(float ambientTemperature, float ambientHumidity, float massConcentrationPm1p0, float massConcentrationPm2p5, float massConcentrationPm4p0, float massConcentrationPm10p0, float vocIndex, float noxIndex);
void disp_weather(current_weather_data_t *current_weather, hourly_weather_data_t *hourly_weather, daily_weather_data_t *daily_weather);
void set_brightness(uint16_t brightness);

void disp_wifi_networks(char* allNetworks);
void disp_disable_scanbutton(bool is_disabled);
void disp_disable_connectbutton(bool is_disabled);
void disp_connect_status(bool is_connected);

//void set_cities(const char* region);



#endif /* GUI_H */