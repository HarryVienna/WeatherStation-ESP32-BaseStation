/**
 * @file lv_daily_chart.h
 *
 */

#ifndef LV_DAILY_CHART_H
#define LV_DAILY_CHART_H

#ifdef __cplusplus
extern "C" {
#endif

#define NUM_DAYS 8
#define MAX_DAILY_PRECIPITATION 20

/*********************
 *      INCLUDES
 *********************/
#include "lv_conf_internal.h"
#include "core/lv_obj.h"
#include <time.h>

/*********************
 *      DEFINES
 *********************/

#define LV_DAILY_CHART_LABEL_MAX_TEXT_LENGTH 16

/**********************
 *      TYPEDEFS
 **********************/
// Daily Weather data structure
typedef struct {
    struct tm dt;
    double low_temp;
    double high_temp;
    double rain;
    double snow;
    double pop;
    int32_t clouds;
} lv_daily_data;


/*Data of line*/
typedef struct {
    lv_obj_t obj;

    lv_daily_data data_array[NUM_DAYS];   
    bool has_data;
    
    int32_t max_temp;
    int32_t min_temp;
    uint32_t ticks_temp;
    int32_t max_precipitation;
    int32_t min_precipitation;
    uint32_t ticks_precipitation;

} lv_daily_chart_t;

/**
 * Enumeration of the axis'
 */
enum {
    LV_DAILY_CHART_AXIS_PRIMARY_Y     = 0x00,
    LV_DAILY_CHART_AXIS_SECONDARY_Y   = 0x01,
    _LV_DAILY_CHART_AXIS_LAST
};
typedef uint8_t lv_daily_chart_axis_t;


/**
 * `type` field in `lv_obj_draw_part_dsc_t` if `class_p = lv_daily_chart_class`
 * Used in `LV_EVENT_DRAW_PART_BEGIN` and `LV_EVENT_DRAW_PART_END`
 */
typedef enum {
    LV_DAILY_CHART_DRAW_PART_DIV_LINE_HOR,     /**< Used for each horizontal division lines*/
    LV_DAILY_CHART_DRAW_PART_TEMP,          /**< Used on line and scatter charts for lines and points*/
    LV_DAILY_CHART_DRAW_PART_CLOUDS,              /**< Used on bar charts for the rectangles*/
    LV_DAILY_CHART_DRAW_PART_PRECIPITATION,
    LV_DAILY_CHART_DRAW_PART_TICK_LABEL,       /**< Used on tick lines and labels*/
} lv_daily_chart_draw_part_type_t;

extern const lv_obj_class_t lv_daily_chart_class;

/**********************
 * GLOBAL PROTOTYPES
 **********************/

/**
 * Create a line object
 * @param parent pointer to an object, it will be the parent of the new line
 * @return pointer to the created line
 */
lv_obj_t * lv_daily_chart_create(lv_obj_t * parent);

/*=====================
 * Setter functions
 *====================*/

/**
 * Set an array of points. The line object will connect these points.
 * @param obj           pointer to a line object
 * @param points        an array of points. Only the address is saved, so the array needs to be alive while the line exists
 * @param point_num     number of points in 'point_a'
 */
void lv_daily_chart_set_data(lv_obj_t * obj, const lv_daily_data *data);


void lv_daily_chart_refresh(lv_obj_t * obj);

/**********************
 *      MACROS
 **********************/

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /*LV_DAILY_CHART_H*/
