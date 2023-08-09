#include "openweatherdata.h"

template<typename T, typename F>
std::vector<T> from_list(F f, const JsonArray& x) {
    std::vector<T> result;
    for (const auto& elem : x) {
        result.push_back(f(elem));
    }
    return result;
}

template <typename T, typename F>
std::optional<T> from_optional(F f, const JsonObject& x) {
    if (x.isNull())
    {
        return std::nullopt;
    }
    else
    {
        return f(x);
    }
}

template <typename T>
std::optional<T> from_optional(const JsonVariant& x) {
    if (x.isNull())
    {
        return std::nullopt;
    }
    else
    {
        return x.as<T>();
    }
}


Alert::Alert(String _sender_name, String _event, tm _start, tm _end, String _description, std::vector<String> _tags)
: sender_name(_sender_name), event(_event), start(_start), end(_end), description(_description), tags(_tags) {}

const String& Alert::get_sender_name() const { return sender_name; }
const String& Alert::get_event() const { return event; }
const tm& Alert::get_start() const { return start; }
const tm& Alert::get_end() const { return end; }
const String& Alert::get_description() const { return description; }
const std::vector<String>& Alert::get_tags() const { return tags; }

Alert Alert::from_json(JsonObject alert) {
    time_t time;
    struct tm start;
    struct tm end;

    String sender_name = alert["sender_name"].as<String>();
    String event = alert["event"].as<String>();
    time = alert["start"].as<time_t>();
    localtime_r(&time, &start);
    time = alert["end"].as<time_t>();
    localtime_r(&time, &end);
    String description = alert["description"].as<String>(); 
    std::vector<String> tags = from_list<String>([](const JsonVariant& value) {
        return value.as<String>();
    }, alert["tags"]);

    return Alert(sender_name, event, start, end, description, tags);
}

void Alert::logValues() {
    Serial.print("Alert");
    Serial.print("sender_name:"); Serial.print(sender_name); Serial.print("  ");
    Serial.print("event: "); Serial.print(event); Serial.print("  ");
    Serial.print("start: "); Serial.print(&start,"%H:%M"); Serial.print("  ");
    Serial.print("end: "); Serial.print(&end,"%H:%M"); Serial.print("  ");
    Serial.print("description: "); Serial.print(description); Serial.println("  ");
}



Rain::Rain(double _one_hour)
    : one_hour(_one_hour) {}

const double & Rain::get_one_hour() const { return one_hour; }

Rain Rain::from_json(JsonObject rain) {

    double one_hour = rain["1h"].as<double>();

    return Rain(one_hour);
}

void Rain::logValues() {
    Serial.print("Rain::one_hour: "); Serial.println(one_hour);
}



Snow::Snow(double _one_hour)
    : one_hour(_one_hour) {}

const double & Snow::get_one_hour() const { return one_hour; }

Snow Snow::from_json(JsonObject snow) {

    double one_hour = snow["1h"].as<double>();

    return Snow(one_hour);
}

void Snow::logValues() {
    Serial.print("Snow::one_hour: "); Serial.println(one_hour);
}



Weather::Weather(int64_t _condition, String _main, String _description, String _icon)
    : condition(_condition), main(_main), description(_description), icon(_icon) {}

const int64_t & Weather::get_condition() const { return condition; }
const String & Weather::get_main() const { return main; }
const String & Weather::get_description() const { return description; }
const String & Weather::get_icon() const { return icon; }

Weather Weather::from_json(JsonObject weather) {

    int64_t condition = weather["id"].as<int64_t>();
    String main = weather["main"].as<String>();
    String description = weather["description"].as<String>();
    String icon = weather["icon"].as<String>();

    return Weather(condition, main, description, icon);
}

void Weather::logValues() {

    Serial.print("Weather");
    Serial.print("condition: "); Serial.print(condition); Serial.print("  ");
    Serial.print("main: "); Serial.print(main); Serial.print("  ");
    Serial.print("description: "); Serial.print(description); Serial.print("  ");
    Serial.print("icon: "); Serial.print(icon); Serial.println("  ");

}



Current::Current(tm _dt, tm _sunrise, tm _sunset, double _temp, double _feels_like,
        int64_t _pressure, int64_t _humidity, double _dew_point, double _uvi, int64_t _clouds,
        int64_t _visibility, double _wind_speed, int64_t _wind_deg, std::optional<double> _wind_gust,
        std::vector<Weather> _weather, std::optional<Rain> _rain, std::optional<Snow> _snow)
        : dt(_dt), sunrise(_sunrise), sunset(_sunset), temp(_temp), feels_like(_feels_like),
            pressure(_pressure), humidity(_humidity), dew_point(_dew_point), uvi(_uvi), clouds(_clouds),
            visibility(_visibility), wind_speed(_wind_speed), wind_deg(_wind_deg), wind_gust(_wind_gust),
            weather(_weather), rain(_rain), snow(_snow) {}


const tm & Current::get_dt() const { return dt; }
const tm & Current::get_sunrise() const { return sunrise; }
const tm & Current::get_sunset() const { return sunset; }
const double & Current::get_temp() const { return temp; }
const double & Current::get_feels_like() const { return feels_like; }
const int64_t & Current::get_pressure() const { return pressure; }
const int64_t & Current::get_humidity() const { return humidity; }
const double & Current::get_dew_point() const { return dew_point; }
const double & Current::get_uvi() const { return uvi; }
const int64_t & Current::get_clouds() const { return clouds; }
const int64_t & Current::get_visibility() const { return visibility; }
const double & Current::get_wind_speed() const { return wind_speed; }
const int64_t & Current::get_wind_deg() const { return wind_deg; }
const std::optional<double> & Current::get_wind_gust() const { return wind_gust; }
const std::vector<Weather> & Current::get_weather() const { return weather; }
const std::optional<Rain> & Current::get_rain() const { return rain; }
const std::optional<Snow> & Current::get_snow() const { return snow; }

Current Current::from_json(JsonObject current) {
    time_t time;
    struct tm dt;
    struct tm sunrise;
    struct tm sunset;

    time = current["dt"].as<time_t>();
    localtime_r(&time, &dt);
    time = current["sunrise"].as<time_t>();
    localtime_r(&time, &sunrise);
    time = current["sunset"].as<time_t>();
    localtime_r(&time, &sunset);
    double temp = current["temp"].as<double>();
    double feels_like = current["feels_like"].as<double>();
    int64_t pressure = current["pressure"].as<int64_t>();
    int64_t humidity = current["humidity"].as<int64_t>();
    double dew_point = current["dew_point"].as<double>();
    double uvi = current["uvi"].as<double>();
    int64_t clouds = current["clouds"].as<int64_t>();
    int64_t visibility = current["visibility"].as<int64_t>();
    double wind_speed = current["wind_speed"].as<double>();
    int64_t wind_deg = current["wind_deg"].as<int64_t>();
    std::optional<double> wind_gust = from_optional<double>(current["wind_gust"]);
    std::vector<Weather> weather = from_list<Weather>([](const JsonObject& value) {return Weather::from_json(value);}, current["weather"]);
    std::optional<Rain> rain = from_optional<Rain>([](const JsonObject& value) { return  Rain::from_json(value); }, current["rain"]);
    std::optional<Snow> snow = from_optional<Snow>([](const JsonObject& value) { return  Snow::from_json(value); }, current["snow"]);

    return Current(dt, sunrise, sunset, temp, feels_like,
        pressure, humidity, dew_point, uvi, clouds,
        visibility, wind_speed, wind_deg, wind_gust,
        weather, rain, snow);
}

void Current::logValues() {
    Serial.print("Current");
    Serial.print("dt: "); Serial.print(&dt,"%H:%M"); Serial.print("  ");
    Serial.print("sunrise: "); Serial.print(&sunrise,"%H:%M"); Serial.print("  ");
    Serial.print("sunset: "); Serial.print(&sunset,"%H:%M"); Serial.print("  ");
    Serial.print("temp: "); Serial.print(temp); Serial.print("  ");
    Serial.print("feels_like: "); Serial.print(feels_like); Serial.print("  ");
    Serial.print("pressure: "); Serial.print(pressure); Serial.print("  ");
    Serial.print("humidity: "); Serial.print(humidity); Serial.print("  ");
    Serial.print("dew_point: "); Serial.print(dew_point); Serial.print("  ");
    Serial.print("uvi: "); Serial.print(uvi); Serial.print("  ");
    Serial.print("clouds: "); Serial.print(clouds); Serial.print("  ");
    Serial.print("visibility: "); Serial.print(visibility); Serial.print("  ");
    Serial.print("wind_speed: "); Serial.print(wind_speed); Serial.print("  ");
    Serial.print("wind_deg: "); Serial.print(wind_deg); Serial.print("  ");
    if (wind_gust) {
        Serial.print("wind_gust: "); Serial.print(wind_gust.value()); Serial.println("  ");
    }

    for(Weather w : weather) {
        w.logValues();
    }

    if (rain) {
        rain.value().logValues();
    }

    if (snow) {
        snow.value().logValues();
    }

}

FeelsLike::FeelsLike(double _day, double _night, double _eve, double _morn)
        : day(_day), night(_night), eve(_eve), morn(_morn) {}

const double & FeelsLike::get_day() const { return day; }
const double & FeelsLike::get_night() const { return night; }
const double & FeelsLike::get_eve() const { return eve; }
const double & FeelsLike::get_morn() const { return morn; }

FeelsLike FeelsLike::from_json(JsonObject feels_like) {

    double day = feels_like["day"].as<double>();
    double night = feels_like["night"].as<double>();
    double eve = feels_like["eve"].as<double>();
    double morn = feels_like["morn"].as<double>();

    return FeelsLike(day, night, eve, morn);
}

void FeelsLike::logValues() {
    Serial.print("FeelsLike");
    Serial.print("day: "); Serial.print(day); Serial.print("  ");
    Serial.print("night: "); Serial.print(night); Serial.print("  ");
    Serial.print("eve: "); Serial.print(eve); Serial.print("  ");
    Serial.print("morn: "); Serial.print(morn); Serial.println("  ");
}


Temp::Temp(double _day, double _min, double _max, double _night, double _eve, double _morn)
    : day(_day), min(_min), max(_max), night(_night), eve(_eve), morn(_morn) {}


const double & Temp::get_day() const { return day; }
const double & Temp::get_min() const { return min; }
const double & Temp::get_max() const { return max; }
const double & Temp::get_night() const { return night; }
const double & Temp::get_eve() const { return eve; }
const double & Temp::get_morn() const { return morn; }

Temp Temp::from_json(JsonObject temp) {

    double day = temp["day"].as<double>();
    double min = temp["min"].as<double>();
    double max = temp["max"].as<double>();
    double night = temp["night"].as<double>();
    double eve = temp["eve"].as<double>();
    double morn = temp["morn"].as<double>();

    return Temp(day, min, max, night, eve, morn);
}

void Temp::logValues() {
    Serial.print("Temp");
    Serial.print("day: "); Serial.print(day); Serial.print("  ");
    Serial.print("min: "); Serial.print(min); Serial.print("  ");
    Serial.print("max: "); Serial.print(max); Serial.print("  ");
    Serial.print("night: "); Serial.print(night); Serial.print("  ");
    Serial.print("eve: "); Serial.print(eve); Serial.print("  ");
    Serial.print("morn: "); Serial.print(morn); Serial.println("  ");
}


Daily::Daily(tm _dt, tm _sunrise, tm _sunset, tm _moonrise, tm _moonset, double _moon_phase,
        Temp _temp, FeelsLike _feels_like, int64_t _pressure, int64_t _humidity, double _dew_point,
        double _wind_speed, int64_t _wind_deg, double _wind_gust, std::vector<Weather> _weather,
        int64_t _clouds, double _pop, double _uvi, std::optional<double> _rain, std::optional<double> _snow) 
        : dt(_dt), sunrise(_sunrise), sunset(_sunset), moonrise(_moonrise), moonset(_moonset), moon_phase(_moon_phase),
        temp(_temp), feels_like(_feels_like), pressure(_pressure), humidity(_humidity), dew_point(_dew_point),
        wind_speed(_wind_speed), wind_deg(_wind_deg), wind_gust(_wind_gust), weather(_weather),
        clouds(_clouds), pop(_pop), uvi(_uvi), rain(_rain), snow(_snow) {}

const tm & Daily::get_dt() const { return dt; }
const tm & Daily::get_sunrise() const { return sunrise; }
const tm & Daily::get_sunset() const { return sunset; }
const tm & Daily::get_moonrise() const { return moonrise; }
const tm & Daily::get_moonset() const { return moonset; }
const double & Daily::get_moon_phase() const { return moon_phase; }
const Temp & Daily::get_temp() const { return temp; }
const FeelsLike & Daily::get_feels_like() const { return feels_like; }
const int64_t & Daily::get_pressure() const { return pressure; }
const int64_t & Daily::get_humidity() const { return humidity; }
const double & Daily::get_dew_point() const { return dew_point; }
const double & Daily::get_wind_speed() const { return wind_speed; }
const int64_t & Daily::get_wind_deg() const { return wind_deg; }
const double & Daily::get_wind_gust() const { return wind_gust; }
const std::vector<Weather> & Daily::get_weather() const { return weather; }
const int64_t & Daily::get_clouds() const { return clouds; }
const double & Daily::get_pop() const { return pop; }
const double & Daily::get_uvi() const { return uvi; }
const std::optional<double> & Daily::get_rain() const { return rain; }
const std::optional<double> & Daily::get_snow() const { return snow; }

Daily Daily::from_json(JsonObject daily) {
    time_t time;
    struct tm dt;
    struct tm sunrise;
    struct tm sunset;
    struct tm moonrise;
    struct tm moonset;

    time = daily["dt"].as<time_t>();
    localtime_r(&time, &dt);
    time = daily["sunrise"].as<time_t>();
    localtime_r(&time, &sunrise);
    time = daily["sunset"].as<time_t>();
    localtime_r(&time, &sunset);
    time = daily["moonrise"].as<time_t>();
    localtime_r(&time, &moonrise);
    time = daily["moonset"].as<time_t>();
    localtime_r(&time, &moonset);
    double moon_phase = daily["moon_phase"].as<double>();
    Temp temp = Temp::from_json(daily["temp"]);
    FeelsLike feels_like = FeelsLike::from_json(daily["feels_like"]);
    int64_t pressure = daily["pressure"].as<int64_t>();
    int64_t humidity = daily["humidity"].as<int64_t>();
    double dew_point = daily["dew_point"].as<double>();
    double wind_speed = daily["wind_speed"].as<double>();
    int64_t wind_deg = daily["wind_deg"].as<int64_t>();
    double wind_gust = daily["wind_gust"].as<double>();
    std::vector<Weather> weather = from_list<Weather>([](const JsonObject& value) {
        return Weather::from_json(value);
    }, daily["weather"]);
    int64_t clouds = daily["clouds"].as<int64_t>();
    double pop = daily["pop"].as<double>();
    double uvi = daily["uvi"].as<double>();
    
    std::optional<double> rain = from_optional<double>(daily["rain"]);
    std::optional<double> snow = from_optional<double>(daily["snow"]);
//    std::optional<double> rain = from_optional<double>([](const JsonObject& value) { return value["rain"].as<double>(); }, daily["rain"]);
//    std::optional<double> snow = from_optional<double>([](const JsonObject& value) { return value["snow"].as<double>(); }, daily["snow"]);

    return Daily(dt, sunrise, sunset, moonrise, moonset, moon_phase,
        temp, feels_like, pressure, humidity, dew_point,
        wind_speed, wind_deg, wind_gust, weather,
        clouds, pop, uvi, rain, snow);
        
}

void Daily::logValues() {

    Serial.println("Daily");
    Serial.print("dt: "); Serial.print(&dt,"%H:%M"); Serial.print("  ");
    Serial.print("sunrise: "); Serial.print(&sunrise,"%H:%M"); Serial.print("  ");
    Serial.print("sunset: "); Serial.print(&sunset,"%H:%M"); Serial.print("  ");
    Serial.print("moonrise: "); Serial.print(&moonrise,"%H:%M"); Serial.print("  ");
    Serial.print("moonset: "); Serial.print(&moonset,"%H:%M"); Serial.print("  ");
    Serial.print("moon_phase: "); Serial.print(moon_phase); Serial.print("  ");
    temp.logValues();
    feels_like.logValues();
    Serial.print("pressure: "); Serial.print(pressure); Serial.print("  ");
    Serial.print("humidity: "); Serial.print(humidity); Serial.print("  ");
    Serial.print("dew_point: "); Serial.print(dew_point); Serial.print("  ");
    Serial.print("wind_speed: "); Serial.print(wind_speed); Serial.print("  ");
    Serial.print("wind_deg: "); Serial.print(wind_deg); Serial.print("  ");
    Serial.print("wind_gust: "); Serial.print(wind_gust); Serial.print("  ");
    Serial.print("clouds: "); Serial.print(clouds); Serial.print("  ");
    Serial.print("pop: "); Serial.print(pop); Serial.print("  ");
    Serial.print("uvi: "); Serial.print(uvi); Serial.print("  ");
    if (rain) {
        Serial.print("rain: "); Serial.print(rain.value());Serial.print("  ");
    }
    else {
        Serial.print("No rain  ");
    }
    if (snow) {
        Serial.print("snow: "); Serial.print(snow.value());Serial.print("  ");
    }
    else {
        Serial.print("No snow  ");
    }
    for(Weather w : weather) {
        w.logValues();
    }
}

Hourly::Hourly(tm _dt, double _temp, double _feels_like, int64_t _pressure, int64_t _humidity, 
        double _dew_point, double _uvi, int64_t _clouds, int64_t _visibility, double _wind_speed, 
        int64_t _wind_deg, double _wind_gust, std::vector<Weather> _weather, double _pop, 
        std::optional<Rain> _rain, std::optional<Snow> _snow)
        : dt(_dt), temp(_temp), feels_like(_feels_like), pressure(_pressure), humidity(_humidity), 
        dew_point(_dew_point), uvi(_uvi), clouds(_clouds), visibility(_visibility), wind_speed(_wind_speed), 
        wind_deg(_wind_deg), wind_gust(_wind_gust), weather(_weather), pop(_pop), rain(_rain), snow(_snow) {}


const tm & Hourly::get_dt() const { return dt; }
const double & Hourly::get_temp() const { return temp; }
const double & Hourly::get_feels_like() const { return feels_like; }
const int64_t & Hourly::get_pressure() const { return pressure; }
const int64_t & Hourly::get_humidity() const { return humidity; }
const double & Hourly::get_dew_point() const { return dew_point; }
const double & Hourly::get_uvi() const { return uvi; }
const int64_t & Hourly::get_clouds() const { return clouds; }
const int64_t & Hourly::get_visibility() const { return visibility; }
const double & Hourly::get_wind_speed() const { return wind_speed; }
const int64_t & Hourly::get_wind_deg() const { return wind_deg; }
const double & Hourly::get_wind_gust() const { return wind_gust; }
const std::vector<Weather> & Hourly::get_weather() const { return weather; }
const double & Hourly::get_pop() const { return pop; }
const std::optional<Rain> & Hourly::get_rain() const { return rain; }
const std::optional<Snow> & Hourly::get_snow() const { return snow; }

Hourly Hourly::from_json(JsonObject hourly) {
    time_t time;
    struct tm dt;

    time = hourly["dt"].as<time_t>();
    localtime_r(&time, &dt);
    double temp = hourly["temp"].as<double>();
    double feels_like = hourly["feels_like"].as<double>();
    int64_t pressure = hourly["pressure"].as<int64_t>();
    int64_t humidity = hourly["humidity"].as<int64_t>();
    double dew_point = hourly["dew_point"].as<double>();
    double uvi = hourly["uvi"].as<double>();
    int64_t clouds = hourly["clouds"].as<int64_t>();
    int64_t visibility = hourly["visibility"].as<int64_t>();
    double wind_speed = hourly["wind_speed"].as<double>();
    int64_t wind_deg = hourly["wind_deg"].as<int64_t>();
    double wind_gust = hourly["wind_gust"].as<double>();
    std::vector<Weather> weather = from_list<Weather>([](const JsonObject& value) { return Weather::from_json(value);}, hourly["weather"]);
    double pop = hourly["pop"].as<double>();
    std::optional<Rain> rain = from_optional<Rain>([](const JsonObject& value) { return  Rain::from_json(value); }, hourly["rain"]);
    std::optional<Snow> snow = from_optional<Snow>([](const JsonObject& value) { return  Snow::from_json(value); }, hourly["snow"]);

    return Hourly(dt, temp, feels_like, pressure, humidity, 
        dew_point, uvi, clouds, visibility, wind_speed, 
        wind_deg, wind_gust, weather, pop, 
        rain, snow);
        
}

void Hourly::logValues() {

    Serial.println("Hourly");
    Serial.print("dt: "); Serial.print(&dt,"%H:%M"); Serial.print("  ");
    Serial.print("temp: "); Serial.print(temp); Serial.print("  ");
    Serial.print("feels_like: "); Serial.print(feels_like); Serial.print("  ");
    Serial.print("pressure: "); Serial.print(pressure); Serial.print("  ");
    Serial.print("humidity: "); Serial.print(humidity); Serial.print("  ");
    Serial.print("dew_point: "); Serial.print(dew_point); Serial.print("  ");
    Serial.print("uvi: "); Serial.print(uvi); Serial.print("  ");
    Serial.print("clouds: "); Serial.print(clouds); Serial.print("  ");
    Serial.print("visibility: "); Serial.print( visibility); Serial.print("  ");
    Serial.print("wind_speed: "); Serial.print(wind_speed); Serial.print("  ");
    Serial.print("wind_deg: "); Serial.print(wind_deg); Serial.print("  ");
    Serial.print("wind_gust: "); Serial.print(wind_gust); Serial.print("  ");
    Serial.print("pop: "); Serial.print(pop); Serial.println("  ");

    for(Weather w : weather) {
        w.logValues();
    }
    if (rain) {
        rain.value().logValues();
    }
    if (snow) {
        snow.value().logValues();
    }
};



Minutely::Minutely(tm _dt, double _precipitation) : 
            dt(_dt), precipitation(_precipitation) {}

const tm & Minutely::get_dt() const { return dt; }
const double & Minutely::get_precipitation() const { return precipitation; }

Minutely Minutely::from_json(JsonObject minutely) {
    time_t time;
    struct tm dt;

    time = minutely["dt"].as<time_t>();
    localtime_r(&time, &dt);
    double precipitation = minutely["precipitation"].as<double>();
    
    return Minutely(dt, precipitation);
}

void Minutely::logValues() {

    Serial.println("Minutely");
    Serial.print("dt: "); Serial.print(&dt,"%H:%M"); Serial.print("  ");
    Serial.print("precipitation: "); Serial.print(precipitation); Serial.println("  ");
}

OpenWeatherData::OpenWeatherData(double _lat, double _lon, String _timezone, int64_t _timezone_offset,
                Current _current, std::vector<Minutely> _minutely, std::vector<Hourly> _hourly,
                std::vector<Daily> _daily, std::vector<Alert> _alerts) :
                lat(_lat), lon(_lon), timezone(_timezone), timezone_offset(_timezone_offset),
                current(_current), minutely(_minutely), hourly(_hourly), daily(_daily), alerts(_alerts) {}

const double& OpenWeatherData::get_lat() const { return lat; }
const double& OpenWeatherData::get_lon() const { return lon; }
const String& OpenWeatherData::get_timezone() const { return timezone; }
const int64_t& OpenWeatherData::get_timezone_offset() const { return timezone_offset; }
const Current& OpenWeatherData::get_current() const { return current; }
const std::vector<Minutely>& OpenWeatherData::get_minutely() const { return minutely; }
const std::vector<Hourly>& OpenWeatherData::get_hourly() const { return hourly; }
const std::vector<Daily>& OpenWeatherData::get_daily() const { return daily; }
const std::vector<Alert>& OpenWeatherData::get_alerts() const { return alerts; }

OpenWeatherData OpenWeatherData::from_json(JsonObject open_weather_data) {

    double lat = open_weather_data["lat"].as<double>();
    double lon = open_weather_data["lon"].as<double>();
    String timezone = open_weather_data["timezone"].as<String>();
    int64_t timezone_offset = open_weather_data["timezone_offset"].as<int64_t>();
    Current current = Current::from_json(open_weather_data["current"]);
    std::vector<Minutely> minutely = from_list<Minutely>([](const JsonObject& value) {
        return Minutely::from_json(value);
    }, open_weather_data["minutely"]);  
    std::vector<Hourly> hourly = from_list<Hourly>([](const JsonObject& value) {
        return Hourly::from_json(value);
    }, open_weather_data["hourly"]);  
    std::vector<Daily> daily = from_list<Daily>([](const JsonObject& value) {
        return Daily::from_json(value);
    }, open_weather_data["daily"]); 
    std::vector<Alert> alerts = from_list<Alert>([](const JsonObject& value) {
        return Alert::from_json(value);
    }, open_weather_data["alerts"]); 

    return OpenWeatherData(lat, lon, timezone, timezone_offset,
                current, minutely, hourly,
                daily, alerts);
}

void OpenWeatherData::logValues() {

    Serial.println("OpenWeatherData");
    Serial.print("lat: "); Serial.print(lat); Serial.print("  ");
    Serial.print("lon: "); Serial.print(lon); Serial.print("  ");
    Serial.print("timezone: "); Serial.print(timezone); Serial.print("  ");
    Serial.print("timezone_offset: "); Serial.print(timezone_offset); Serial.println("  ");

    for(Minutely m : minutely) {
        m.logValues();
    }
    for(Hourly h : hourly) {
        h.logValues();
    }
    for(Daily d : daily) {
        d.logValues();
    }

    current.logValues();
}

