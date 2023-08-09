#include <Arduino.h>
#include <Wire.h>
#include <lv_conf.h>
#include <lvgl.h>
#include <mutex>
#include "config/config.h"
#include "wifi/network.h"
#include "openweather/openweatherapi.h"
#include "gui/gui.h"
#include "task/clock_task.h"
#include "task/weather_task.h"
#include "task/sensor_task.h"
#include "task/brightness_task.h"

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

  wifi_start();
  esp_now_start();

  gui_start(); 


          

  xTaskCreatePinnedToCore(
                    clock_task,   /* Task function. */
                    "Clock Task", /* String with name of task. */
                    4096,         /* Stack size in bytes. */
                    NULL,         /* Parameter passed as input of the task */
                    1,            /* Priority of the task. */
                    NULL,         /* Task handle. */
                    1);           /* Clock task on core 0*/      
 
  xTaskCreatePinnedToCore(
                    sensor_task,   /* Task function. */
                    "Sensor Task", /* String with name of task. */
                    4096,         /* Stack size in bytes. */
                    NULL,         /* Parameter passed as input of the task */
                    1,           /* Priority of the task. */
                    NULL,         /* Task handle. */
                    1);                                                               

  xTaskCreatePinnedToCore(
                    brightness_task,   
                    "Brightness Task", 
                    4096,        
                    NULL,         
                    1,           
                    NULL,         
                    1); 
                    
  xTaskCreatePinnedToCore(
                    weather_task,   
                    "Weather Task", 
                    16384,        
                    NULL,         
                    1,           
                    NULL,         
                    1);    

  
  //delay(10000);
  //gui_screenshot();   
                     
}



void loop()
{
  xSemaphoreTake(mutex, portMAX_DELAY);
  lv_timer_handler();
  xSemaphoreGive(mutex);

  delay(5);
}