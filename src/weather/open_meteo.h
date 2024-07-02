#ifndef OPEN_METEO_H
#define OPEN_METEO_H

#ifdef __cplusplus
extern "C" {
#endif

#include <time.h>

typedef struct {
    struct tm time;           // Store the time in a struct tm
    double temperature_2m;    // Temperature in °C
    double relative_humidity_2m; // Relative humidity in %
    double precipitation_probability; // Precipitation probability in %
    double rain;             // Rain amount in mm
    double showers;          // Shower amount in mm
    double snowfall;         // Snowfall amount in cm
    int weather_code;        // WMO weather code
    double wind_speed_10m;   // Wind speed in km/h
    double wind_gusts_10m;   // Wind gusts in km/h
    double uv_index;         // UV index
    bool is_day;             // Boolean to indicate if it's day or night
    double sunshine_duration; // Sunshine duration in seconds
} hourly_weather_data_t;

#ifdef __cplusplus
}
#endif

#endif