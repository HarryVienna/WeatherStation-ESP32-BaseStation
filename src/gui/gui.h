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

#endif