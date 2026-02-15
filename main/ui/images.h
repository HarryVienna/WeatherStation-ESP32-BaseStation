#ifndef EEZ_LVGL_UI_IMAGES_H
#define EEZ_LVGL_UI_IMAGES_H

#include <lvgl.h>

#ifdef __cplusplus
extern "C" {
#endif

extern const lv_img_dsc_t img_wifi_off;
extern const lv_img_dsc_t img_wifi_on;
extern const lv_img_dsc_t img_cloud;
extern const lv_img_dsc_t img_uv;
extern const lv_img_dsc_t img_arrow;
extern const lv_img_dsc_t img_sunrise;
extern const lv_img_dsc_t img_sunset;
extern const lv_img_dsc_t img_humidity;
extern const lv_img_dsc_t img_pressure;
extern const lv_img_dsc_t img_battery;
extern const lv_img_dsc_t img_clock;
extern const lv_img_dsc_t img_0d;
extern const lv_img_dsc_t img_0n;
extern const lv_img_dsc_t img_1d;
extern const lv_img_dsc_t img_1n;
extern const lv_img_dsc_t img_2;
extern const lv_img_dsc_t img_3;
extern const lv_img_dsc_t img_45;
extern const lv_img_dsc_t img_48;
extern const lv_img_dsc_t img_51;
extern const lv_img_dsc_t img_53;
extern const lv_img_dsc_t img_55;
extern const lv_img_dsc_t img_56;
extern const lv_img_dsc_t img_57;
extern const lv_img_dsc_t img_61;
extern const lv_img_dsc_t img_63;
extern const lv_img_dsc_t img_65;
extern const lv_img_dsc_t img_66;
extern const lv_img_dsc_t img_67;
extern const lv_img_dsc_t img_71;
extern const lv_img_dsc_t img_73;
extern const lv_img_dsc_t img_75;
extern const lv_img_dsc_t img_77;
extern const lv_img_dsc_t img_80;
extern const lv_img_dsc_t img_81;
extern const lv_img_dsc_t img_82;
extern const lv_img_dsc_t img_85;
extern const lv_img_dsc_t img_86;
extern const lv_img_dsc_t img_95;
extern const lv_img_dsc_t img_96;
extern const lv_img_dsc_t img_99;

#ifndef EXT_IMG_DESC_T
#define EXT_IMG_DESC_T
typedef struct _ext_img_desc_t {
    const char *name;
    const lv_img_dsc_t *img_dsc;
} ext_img_desc_t;
#endif

extern const ext_img_desc_t images[41];

#ifdef __cplusplus
}
#endif

#endif /*EEZ_LVGL_UI_IMAGES_H*/