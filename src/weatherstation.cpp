#include <Arduino.h>
#include <Wire.h>
#include <lv_conf.h>
#include <lvgl.h>
#include <mutex>
#include "config/config.h"
#include "openweather/openweatherapi.h"
#include "gui/gui.h"


SemaphoreHandle_t mutex = xSemaphoreCreateMutex();

void setup()
{
  Serial.begin(115200);

  Serial.println(" __        __         _   _                   _        _   _             ");
  Serial.println(" \\ \\      / /__  __ _| |_| |__   ___ _ __ ___| |_ __ _| |_(_) ___  _ __  ");
  Serial.println("  \\ \\ /\\ / / _ \\/ _` | __| \'_ \\ / _ \\ \'__/ __| __/ _` | __| |/ _ \\| \'_ \\ ");
  Serial.println("   \\ V  V /  __/ (_| | |_| | | |  __/ |  \\__ \\ || (_| | |_| | (_) | | | |");
  Serial.println("    \\_/\\_/ \\___|\\__,_|\\__|_| |_|\\___|_|  |___/\\__\\__,_|\\__|_|\\___/|_| |_|\r\n     ");
  Serial.println("");

  log_i("Total heap: %d", ESP.getHeapSize());
  log_i("Free heap: %d", ESP.getFreeHeap());
  log_i("Total PSRAM: %d", ESP.getPsramSize());
  log_i("Free PSRAM: %d", ESP.getFreePsram());

  //Wire.setTimeOut(10000);
  Wire1.begin(I2C_SDA, I2C_SCL, I2C_FREQ);

  gui_start(); 
                     
}



void loop()
{
  xSemaphoreTake(mutex, portMAX_DELAY);
  lv_timer_handler();
  xSemaphoreGive(mutex);

  delay(5);
}