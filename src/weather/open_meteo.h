#ifndef OPEN_METEO_H
#define OPEN_METEO_H

#ifdef __cplusplus
extern "C" {
#endif

#include <time.h>

typedef struct {
    double temperature_2m;          // Air temperature at 2 meters above ground
    double dew_point_2m;            // Dew point temperature at 2 meters above ground
    int relative_humidity_2m;       // Relative humidity at 2 meters above ground
    double apparent_temperature;    // Apparent temperature in °C
    int is_day;                     // Day indicator (1 = day, 0 = night)
    int weather_code;               // Weather code (WMO code)
    int cloud_cover;                // Cloud cover in %
    double wind_speed_10m;          // Wind speed at 10 meters in km/h
    int wind_direction_10m;         // Wind direction at 10 meters in degrees
    double wind_gusts_10m;          // Wind gusts at 10 meters in km/h
    double uv_index;                // UV index
} current_weather_data_t;

typedef struct {
    struct tm time;                   // Store the time in a struct tm
    double temperature_2m;            // Air temperature at 2 meters above ground
    double dew_point_2m;              // Dew point temperature at 2 meters above ground
    double precipitation_probability; // Precipitation probability in %
    double rain;                      // Rain amount in mm
    double showers;                   // Shower amount in mm
    double snowfall;                  // Snowfall amount in cm
    double wind_speed_10m;            // Wind speed in km/h
    double wind_gusts_10m;            // Wind gusts in km/h
    double sunshine_duration;         // Sunshine duration in seconds
    double cloud_cover;               // Total cloud cover
    bool is_day;                      // Boolean to indicate if it's day or night
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
    struct tm sunrise;          // Store the sunrise in a struct tm
    struct tm sunset;           // Store the sunset in a struct tm
} daily_weather_data_t;

#ifdef __cplusplus
}
#endif

#endif