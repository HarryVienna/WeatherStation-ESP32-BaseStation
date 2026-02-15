#ifndef EEZ_LVGL_UI_EVENTS_H
#define EEZ_LVGL_UI_EVENTS_H

#include <lvgl.h>

#ifdef __cplusplus
extern "C" {
#endif

extern void action_event_setup_screen_loaded(lv_event_t * e);
extern void action_event_wifi_scan(lv_event_t * e);
extern void action_event_wifi_connect(lv_event_t * e);
extern void action_event_weatherstation_start(lv_event_t * e);
extern void action_event_text_area_app_id(lv_event_t * e);
extern void action_event_text_area_password(lv_event_t * e);
extern void action_event_text_area_latitude(lv_event_t * e);
extern void action_event_text_area_longitude(lv_event_t * e);
extern void action_event_text_area_hoehe(lv_event_t * e);
extern void action_event_text_area_basis(lv_event_t * e);
extern void action_event_text_area_sensor_name1(lv_event_t * e);
extern void action_event_text_area_sensor_name2(lv_event_t * e);
extern void action_event_text_area_sensor_name3(lv_event_t * e);
extern void action_event_keyboard_text(lv_event_t * e);
extern void action_event_keyboard_numeric(lv_event_t * e);

#ifdef __cplusplus
}
#endif

#endif /*EEZ_LVGL_UI_EVENTS_H*/