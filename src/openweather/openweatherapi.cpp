
#include <Arduino.h>
#include "openweatherapi.h"
#include "test/test01.h"


// Konstruktor, der die API-Parameter speichert
OpenWeatherApi::OpenWeatherApi(const String& url, const String& lat, const String& lon,
                const String& units, const String& lang, const String& appid) {
    url_ = url;
    lat_ = lat;
    lon_ = lon;
    units_ = units;
    lang_ = lang;
    appid_ = appid;
}


// Funktion, die die API-Anfrage sendet und die Wetterdaten als OpenWeatherData-Objekt zurückgibt
OpenWeatherData OpenWeatherApi::getdata() {
    //Serial.println("Calling API");

    // https://api.openweathermap.org/data/3.0/onecall?lat=48.214110&lon=16.323219&units=metric&lang=de&appid=abdb6964a969eeceed4f072180f1d3a7
    
    String apiURL = url_ + "?lat=" + lat_ + "&lon=" + lon_ + "&units=" + units_ + "&lang=" + lang_ + "&appid=" + appid_;
    //Serial.println(apiURL);

    http.begin(apiURL);
    int httpResponseCode = http.GET();

    BasicJsonDocument<SpiRamAllocator> doc(65536);
    DeserializationError error = deserializeJson(doc, http.getStream());

    http.end();

    if (error) {
        Serial.print("deserializeJson() failed: ");
        Serial.println(error.c_str());
    }

    JsonObject root = doc.as<JsonObject>();
    
    OpenWeatherData data = OpenWeatherData::from_json(root);

    return data;

}

OpenWeatherData OpenWeatherApi::getdata_test() {
    Serial.println("Calling API");

    BasicJsonDocument<SpiRamAllocator> doc(65536);
    DeserializationError error = deserializeJson(doc, json_test);

    
    JsonObject root = doc.as<JsonObject>();

    OpenWeatherData data = OpenWeatherData::from_json(root);

    return data;

}
