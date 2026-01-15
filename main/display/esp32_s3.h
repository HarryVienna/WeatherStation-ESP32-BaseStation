#ifndef ESP32_S3_H
#define ESP32_S3_H

#include <stdio.h>

#include "esp_lcd_touch_gt911.h"
#include "esp_lcd_panel_ops.h"
#include "esp_lcd_panel_rgb.h"
#include "esp_timer.h"

#ifdef __cplusplus
extern "C" {
#endif

// Function declarations
void init_display(void);

void init_backlight(void);

void set_backlight_brightness(uint16_t brightness);

void init_touch(esp_lcd_touch_handle_t *touch_handle);

void init_lcd(esp_lcd_panel_handle_t *panel_handle);

void lcd_off(esp_lcd_panel_handle_t *panel_handle);

void init_lvgl(esp_lcd_panel_handle_t panel_handle, esp_lcd_touch_handle_t touch_handle);

bool lvgl_port_lock(int timeout_ms);

void lvgl_port_unlock(void);

#ifdef __cplusplus
}
#endif

#endif /* ESP32_S3_H */