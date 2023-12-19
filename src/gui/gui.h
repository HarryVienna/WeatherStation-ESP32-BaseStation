#ifndef GUI_H
#define GUI_H

#include "../openweather/openweatherdata.h"

void gui_start();
void gui_screenshot();

void disp_date_time(char* date_time);
void disp_sensor_data(uint8_t sensor_nr, double temperature, double humidity, double pressure, uint32_t voltage, char* date_time);
void disp_scd4x(uint16_t co2);
void disp_sen5x(float ambientTemperature, float ambientHumidity, float massConcentrationPm1p0, float massConcentrationPm2p5, float massConcentrationPm4p0, float massConcentrationPm10p0, float vocIndex, float noxIndex);
void disp_weather(OpenWeatherData data);
void set_brightness(uint8_t brightness);

void disp_wifi_networks(String allNetworks);
void disp_connect_status(bool is_connected);
void disp_disable_scanbutton(bool is_disabled);
void disp_disable_connectbutton(bool is_disabled);

void set_cities(const char* region);
void set_labels(String name_base, String name_sensor_1, String name_sensor_2, String name_sensor_3);

void disp_wifi_networks(String allNetworks);
void disp_connect_status(bool is_connected);
void start_tasks();

#endif