#ifndef OPENWEATHERDATA_H
#define OPENWEATHERDATA_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include <optional>
#include <stdexcept>
#include <regex>
#include <functional>
#include <vector>


template<typename T, typename F>
std::vector<T> from_list(F f, const JsonArray& x);

template <typename T, typename F>
std::optional<T> from_optional(F f, const JsonObject& x);



class Alert {
public:
    Alert(String _sender_name, String _event, tm _start, tm _end, String _description, std::vector<String> _tags);

    const String& get_sender_name() const;
    const String& get_event() const;
    const tm& get_start() const;
    const tm& get_end() const;
    const String& get_description() const;
    const std::vector<String>& get_tags() const;

    static Alert from_json(JsonObject alert);

    void logValues();

private:
    String sender_name;
    String event;
    tm start;
    tm end;
    String description;
    std::vector<String> tags;
};



class Rain {
public:
    Rain(double _one_hour);

    const double& get_one_hour() const;

    static Rain from_json(JsonObject rain);

    void logValues();
    
private:
    double one_hour;
};



class Snow {
public:
    Snow(double _one_hour);

    const double & get_one_hour() const;

    static Snow from_json(JsonObject snow);

    void logValues();

private:
    double one_hour;
};



class Weather {
public:
    Weather(int64_t _condition, String _main, String _description, String _icon);

    const int64_t & get_condition() const;
    const String & get_main() const;
    const String & get_description() const;
    const String & get_icon() const;

    static Weather from_json(JsonObject weather);

    void logValues();

private:
    int64_t condition;
    String main;
    String description;
    String icon;
};


class Current {
public:
    Current(tm _dt, tm _sunrise, tm _sunset, double _temp, double _feels_like,
            int64_t _pressure, int64_t _humidity, double _dew_point, double _uvi, int64_t _clouds,
            int64_t _visibility, double _wind_speed, int64_t _wind_deg, std::optional<double> _wind_gust,
            std::vector<Weather> _weather, std::optional<Rain> _rain, std::optional<Snow> _snow);

    const tm & get_dt() const;
    const tm & get_sunrise() const;
    const tm & get_sunset() const;
    const double & get_temp() const;
    const double & get_feels_like() const;
    const int64_t & get_pressure() const;
    const int64_t & get_humidity() const;
    const double & get_dew_point() const;
    const double & get_uvi() const;
    const int64_t & get_clouds() const;
    const int64_t & get_visibility() const;
    const double & get_wind_speed() const;
    const int64_t & get_wind_deg() const;
    const std::optional<double> & get_wind_gust() const;
    const std::vector<Weather> & get_weather() const;
    const std::optional<Rain> & get_rain() const;
    const std::optional<Snow> & get_snow() const;

    static Current from_json(JsonObject current);

    void logValues();

private:
    tm dt;
    tm sunrise;
    tm sunset;
    double temp;
    double feels_like;
    int64_t pressure;
    int64_t humidity;
    double dew_point;
    double uvi;
    int64_t clouds;
    int64_t visibility;
    double wind_speed;
    int64_t wind_deg;
    std::optional<double> wind_gust;
    std::vector<Weather> weather;
    std::optional<Rain> rain;
    std::optional<Snow> snow;
};


class FeelsLike {
public:
     FeelsLike(double _day, double _night, double _eve, double _morn);

    const double & get_day() const;
    const double & get_night() const;
    const double & get_eve() const;
    const double & get_morn() const;

    static FeelsLike from_json(JsonObject feels_like);

    void logValues();

private:
    double day;
    double night;
    double eve;
    double morn;
};

class Temp {
public:
    Temp(double _day, double _min, double _max, double _night, double _eve, double _morn);

    const double & get_day() const;
    const double & get_min() const;
    const double & get_max() const;
    const double & get_night() const;
    const double & get_eve() const;
    const double & get_morn() const;

    static Temp from_json(JsonObject temp);

    void logValues();

private:
    double day;
    double min;
    double max;
    double night;
    double eve;
    double morn;
};

class Daily {
public:
    Daily(tm _dt, tm _sunrise, tm _sunset, tm _moonrise, tm _moonset, double _moon_phase,
          Temp _temp, FeelsLike _feels_like, int64_t _pressure, int64_t _humidity, double _dew_point,
          double _wind_speed, int64_t _wind_deg, double _wind_gust, std::vector<Weather> _weather,
          int64_t _clouds, double _pop, double _uvi, std::optional<double> _rain, std::optional<double> _snow);

    const tm & get_dt() const;
    const tm & get_sunrise() const;
    const tm & get_sunset() const;
    const tm & get_moonrise() const;
    const tm & get_moonset() const;
    const double & get_moon_phase() const;
    const Temp & get_temp() const;
    const FeelsLike & get_feels_like() const;
    const int64_t & get_pressure() const;
    const int64_t & get_humidity() const;
    const double & get_dew_point() const;
    const double & get_wind_speed() const;
    const int64_t & get_wind_deg() const;
    const double & get_wind_gust() const;
    const std::vector<Weather> & get_weather() const;
    const int64_t & get_clouds() const;
    const double & get_pop() const;
    const double & get_uvi() const;
    const std::optional<double> & get_rain() const;
    const std::optional<double> & get_snow() const;

    static Daily from_json(JsonObject daily);

    void logValues();

private:
    tm dt;
    tm sunrise;
    tm sunset;
    tm moonrise;
    tm moonset;
    double moon_phase;
    Temp temp;
    FeelsLike feels_like;
    int64_t pressure;
    int64_t humidity;
    double dew_point;
    double wind_speed;
    int64_t wind_deg;
    double wind_gust;
    std::vector<Weather> weather;
    int64_t clouds;
    double pop;
    double uvi;
    std::optional<double> rain;
    std::optional<double> snow;
};


class Hourly {
public:
    Hourly(tm _dt, double _temp, double _feels_like, int64_t _pressure, int64_t _humidity, 
           double _dew_point, double _uvi, int64_t _clouds, int64_t _visibility, double _wind_speed, 
           int64_t _wind_deg, double _wind_gust, std::vector<Weather> _weather, double _pop, 
           std::optional<Rain> _rain, std::optional<Snow> _snow);

    const tm & get_dt() const;
    const double & get_temp() const;
    const double & get_feels_like() const;
    const int64_t & get_pressure() const;
    const int64_t & get_humidity() const;
    const double & get_dew_point() const;
    const double & get_uvi() const;
    const int64_t & get_clouds() const;
    const int64_t & get_visibility() const;
    const double & get_wind_speed() const;
    const int64_t & get_wind_deg() const;
    const double & get_wind_gust() const;
    const std::vector<Weather> & get_weather() const;
    const double & get_pop() const;
    const std::optional<Rain> & get_rain() const;
    const std::optional<Snow> & get_snow() const;

    static Hourly from_json(JsonObject hourly);

    void logValues();

private:
    tm dt;
    double temp;
    double feels_like;
    int64_t pressure;
    int64_t humidity;
    double dew_point;
    double uvi;
    int64_t clouds;
    int64_t visibility;
    double wind_speed;
    int64_t wind_deg;
    double wind_gust;
    double pop;
    std::vector<Weather> weather;
    std::optional<Rain> rain;
    std::optional<Snow> snow;
};

class Minutely {
public:
    Minutely(tm _dt, double _precipitation);

    const tm& get_dt() const;
    const double& get_precipitation() const;

    static Minutely from_json(JsonObject minutely);

    void logValues();

private:
    tm dt;
    double precipitation;
};


class OpenWeatherData {
public:
    OpenWeatherData(double _lat, double _lon, String _timezone, int64_t _timezone_offset,
                    Current _current, std::vector<Minutely> _minutely, std::vector<Hourly> _hourly,
                    std::vector<Daily> _daily, std::vector<Alert> _alerts);
                
    const double& get_lat() const;
    const double& get_lon() const;
    const String& get_timezone() const;
    const int64_t& get_timezone_offset() const;
    const Current& get_current() const;
    const std::vector<Minutely>& get_minutely() const;
    const std::vector<Hourly>& get_hourly() const;
    const std::vector<Daily>& get_daily() const;
    const std::vector<Alert>& get_alerts() const;

    static OpenWeatherData from_json(JsonObject open_weather_data);

    void logValues();

private:
    double lat;
    double lon;
    String timezone;
    int64_t timezone_offset;
    Current current;
    std::vector<Minutely> minutely;
    std::vector<Hourly> hourly;
    std::vector<Daily> daily;
    std::vector<Alert> alerts;
};

#endif