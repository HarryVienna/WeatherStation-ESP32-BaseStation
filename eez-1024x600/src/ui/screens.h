#ifndef EEZ_LVGL_UI_SCREENS_H
#define EEZ_LVGL_UI_SCREENS_H

#include <lvgl.h>

#ifdef __cplusplus
extern "C" {
#endif

// Screens

enum ScreensEnum {
    _SCREEN_ID_FIRST = 1,
    SCREEN_ID_SETUP_SCREEN = 1,
    SCREEN_ID_WEATHERSTATION_SCREEN = 2,
    _SCREEN_ID_LAST = 2
};

typedef struct _objects_t {
    lv_obj_t *setup_screen;
    lv_obj_t *weatherstation_screen;
    lv_obj_t *panel_setup;
    lv_obj_t *panel_networks;
    lv_obj_t *dropdown_networks;
    lv_obj_t *button_scan;
    lv_obj_t *label_scan;
    lv_obj_t *panel_password;
    lv_obj_t *text_area_password;
    lv_obj_t *button_connect;
    lv_obj_t *label_scan_1;
    lv_obj_t *panel_app_id;
    lv_obj_t *text_area_app_id;
    lv_obj_t *panel_location;
    lv_obj_t *text_area_latitude;
    lv_obj_t *text_area_longitude;
    lv_obj_t *text_area_hoehe;
    lv_obj_t *panel_timezone;
    lv_obj_t *dropdown_region;
    lv_obj_t *dropdown_city;
    lv_obj_t *panel_base;
    lv_obj_t *text_area_basis;
    lv_obj_t *panel_sensors;
    lv_obj_t *text_area_sensor_name1;
    lv_obj_t *text_area_sensor_name2;
    lv_obj_t *text_area_sensor_name3;
    lv_obj_t *panel_start;
    lv_obj_t *button_starten;
    lv_obj_t *label_scan_2;
    lv_obj_t *keyboard_text;
    lv_obj_t *keyboard_numeric;
    lv_obj_t *current;
    lv_obj_t *obj0;
    lv_obj_t *date_time;
    lv_obj_t *wifi;
    lv_obj_t *weather_icon;
    lv_obj_t *temp_current;
    lv_obj_t *temp_current_unit;
    lv_obj_t *clouds_current;
    lv_obj_t *uv_current;
    lv_obj_t *wind_direction_current_icon;
    lv_obj_t *wind_speed_current;
    lv_obj_t *wind_gust_current;
    lv_obj_t *sunrise_current;
    lv_obj_t *sunset_current;
    lv_obj_t *base;
    lv_obj_t *obj1;
    lv_obj_t *name_base;
    lv_obj_t *temp_base;
    lv_obj_t *obj2;
    lv_obj_t *humidity_base;
    lv_obj_t *panel_pm;
    lv_obj_t *pm10;
    lv_obj_t *pm4;
    lv_obj_t *pm2p5;
    lv_obj_t *pm1;
    lv_obj_t *panel_voc;
    lv_obj_t *voc;
    lv_obj_t *panel_nox;
    lv_obj_t *nox;
    lv_obj_t *panel_co2;
    lv_obj_t *co2;
    lv_obj_t *sensor0;
    lv_obj_t *obj3;
    lv_obj_t *name0;
    lv_obj_t *temp0;
    lv_obj_t *obj4;
    lv_obj_t *hunidity0;
    lv_obj_t *pressure0;
    lv_obj_t *line_status0;
    lv_obj_t *volt0;
    lv_obj_t *update0;
    lv_obj_t *sensor1;
    lv_obj_t *obj5;
    lv_obj_t *name1;
    lv_obj_t *temp1;
    lv_obj_t *obj6;
    lv_obj_t *hunidity1;
    lv_obj_t *pressure1;
    lv_obj_t *line_status1;
    lv_obj_t *volt1;
    lv_obj_t *update1;
    lv_obj_t *sensor2;
    lv_obj_t *obj7;
    lv_obj_t *name2;
    lv_obj_t *temp2;
    lv_obj_t *obj8;
    lv_obj_t *hunidity2;
    lv_obj_t *pressure2;
    lv_obj_t *line_status2;
    lv_obj_t *volt2;
    lv_obj_t *update2;
    lv_obj_t *hourly;
    lv_obj_t *hourly_chart;
    lv_obj_t *daily;
    lv_obj_t *daily_chart;
} objects_t;

extern objects_t objects;

void create_screen_setup_screen();
void tick_screen_setup_screen();

void create_screen_weatherstation_screen();
void tick_screen_weatherstation_screen();

void tick_screen_by_id(enum ScreensEnum screenId);
void tick_screen(int screen_index);

void create_screens();

#ifdef __cplusplus
}
#endif

#endif /*EEZ_LVGL_UI_SCREENS_H*/