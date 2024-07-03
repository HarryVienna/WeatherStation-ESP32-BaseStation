#ifndef OPEN_METEO_H
#define OPEN_METEO_H

#ifdef __cplusplus
extern "C" {
#endif

#include <time.h>

typedef struct {
    struct tm time;           // Store the time in a struct tm
    double temperature_2m;    // Temperature in °C
    double precipitation_probability; // Precipitation probability in %
    double rain;             // Rain amount in mm
    double showers;          // Shower amount in mm
    double snowfall;         // Snowfall amount in cm
    double wind_speed_10m;   // Wind speed in km/h
    double wind_gusts_10m;   // Wind gusts in km/h
    double sunshine_duration; // Sunshine duration in seconds
} hourly_weather_data_t;

typedef struct {
    struct tm time;             // Store the time in a struct tm
    double temperature_2m_max;  // Maximum temperature in °C
    double temperature_2m_min;  // Minimum temperature in °C
    double daylight_duration;   // Daylight duration in seconds
    double sunshine_duration;   // Sunshine duration in seconds
    double rain_sum;            // Total rain amount in mm
    double showers_sum;         // Total showers amount in mm
    double snowfall_sum;        // Total snowfall amount in cm
    double precipitation_probability_max; // Maximum precipitation probability in %
    double wind_speed_10m_max;  // Maximum wind speed in km/h
    double wind_gusts_10m_max;  // Maximum wind gusts in km/h
} daily_weather_data_t;

#ifdef __cplusplus
}
#endif

#endif