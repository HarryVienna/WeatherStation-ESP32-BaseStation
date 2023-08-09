#ifndef OPENWEATHERAPI_H
#define OPENWEATHERAPI_H

#include <Arduino.h>
#include <HTTPClient.h> // Bibliothek für HTTP-Anfragen
#include <ArduinoJson.h>

#include "openweatherdata.h"

struct SpiRamAllocator {
  void* allocate(size_t size) {
    return heap_caps_malloc(size, MALLOC_CAP_SPIRAM);
  }

  void deallocate(void* pointer) {
    heap_caps_free(pointer);
  }

  void* reallocate(void* ptr, size_t new_size) {
    return heap_caps_realloc(ptr, new_size, MALLOC_CAP_SPIRAM);
  }
};



class OpenWeatherApi {

private:
    String url_; // API-URL
    String lat_; // Breitengrad
    String lon_; // Längengrad
    String units_; // Einheitensystem (z.B. "metric" für Celsius)
    String lang_; // Sprache (z.B. "en" für Englisch)
    String appid_; // API-Schlüssel

    HTTPClient http;

public:

    // Konstruktor, der die API-Parameter speichert
    OpenWeatherApi(const String& url, const String& lat, const String& lon,
                   const String& units, const String& lang, const String& appid);

    // Funktion, die die API-Anfrage sendet und die Wetterdaten als OpenWeatherData-Objekt zurückgibt
    OpenWeatherData getdata();
    OpenWeatherData getdata_test();

};

#endif