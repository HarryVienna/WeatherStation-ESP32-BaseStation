#include "esp_log.h"

#include "gui.h"

#include "task/wifiscan_task.h"
#include "task/sensor_task.h"

static const char* TAG = "GUI";

void disp_wifi_networks(char* allNetworks)
{
  lv_dropdown_clear_options(ui_DropdownNetworks);
  lv_dropdown_set_options(ui_DropdownNetworks, allNetworks);
}

void disp_disable_scanbutton(bool is_disabled)
{
  if (is_disabled)
  {
    lv_obj_add_state( ui_ButtonScan, LV_STATE_DISABLED ); 
  }
  else
  {
    lv_obj_clear_state( ui_ButtonScan, LV_STATE_DISABLED ); 
  }
}

// -------- LVGL Events --------

void event_screen_loaded(lv_event_t *e)
{

}

void event_wifi_scan(lv_event_t *e)
{
    xTaskCreatePinnedToCore(
        wifiscan_task,   // Task function
        "WiFiScan Task", // Task name
        16000,            // Stack size (bytes)
        NULL,            // Task input parameter
        16,              // Task priority
        NULL,            // Task handle
        0                // Core to run the task on (0 or 1)
    );
 
}

void event_wifi_connect(lv_event_t *e)
{
 
}

void event_value_changed(lv_event_t *e)
{
 
}

void event_weatherstation_start(lv_event_t *e)
{
 
    xTaskCreatePinnedToCore(
        sensor_task,   // Task function
        "Sensor Task", // Task name
        16000,            // Stack size (bytes)
        NULL,            // Task input parameter
        16,              // Task priority
        NULL,            // Task handle
        0                // Core to run the task on (0 or 1)
    );
}

