/**
 * @file lv_daily_chart.c
 *
 */

/*********************
 *      INCLUDES
 *********************/
#include <Arduino.h>
#include "lv_daily_chart.h"
#include "lv_common.h"
#include "../config/config.h"

#include "misc/lv_assert.h"
#include "draw/lv_draw.h"
#include "misc/lv_math.h"
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <float.h>

/*********************
 *      DEFINES
 *********************/
#define MY_CLASS &lv_daily_chart_class

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/
static void lv_daily_chart_constructor(const lv_obj_class_t * class_p, lv_obj_t * obj);
static void lv_daily_chart_event(const lv_obj_class_t * class_p, lv_event_t * e);


/**********************
 *  STATIC VARIABLES
 **********************/
const lv_obj_class_t lv_daily_chart_class = {
    .constructor_cb = lv_daily_chart_constructor,
    .event_cb = lv_daily_chart_event,
    //.width_def = LV_SIZE_CONTENT,
    //.height_def = LV_SIZE_CONTENT,
    .width_def = LV_PCT(100),
    .height_def = LV_DPI_DEF * 2,    
    .instance_size = sizeof(lv_daily_chart_t),
    .base_class = &lv_obj_class
};

/**********************
 *      MACROS
 **********************/


/**********************
 *   GLOBAL FUNCTIONS
 **********************/

lv_obj_t * lv_daily_chart_create(lv_obj_t * parent)
{
    LV_LOG_INFO("begin");
    lv_obj_t * obj = lv_obj_class_create_obj(MY_CLASS, parent);
    lv_obj_class_init_obj(obj);
    return obj;
}

void lv_daily_chart_refresh(lv_obj_t * obj)
{
    LV_ASSERT_OBJ(obj, MY_CLASS);

    lv_obj_invalidate(obj);
}

/*=====================
 * Setter functions
 *====================*/

void lv_daily_chart_set_data(lv_obj_t * obj, const lv_daily_data *data)
{
    LV_ASSERT_OBJ(obj, MY_CLASS);

    if (obj == NULL || data == NULL) {
        // Handle invalid input parameters
        return;
    }

    lv_daily_chart_t * chart = (lv_daily_chart_t *)obj;

    float max_temp = DBL_MIN;
    float min_temp = DBL_MAX;

    for (int i = 0; i < NUM_DAYS; i++) {
        chart->data_array[i] = data[i];

        if (data[i].high_temp > max_temp) { 
            max_temp = data[i].high_temp;
        }
        if (data[i].low_temp < min_temp) {
            min_temp = data[i].low_temp;
        }
    }

    chart->min_temp = (int32_t)(5.0 * floor(min_temp / 5.0)); // next upper 5 value
    chart->max_temp = (int32_t)(5.0 * ceil(max_temp / 5.0));  // next lower 5 value
    chart->ticks_temp = (chart->max_temp - chart->min_temp) / 5 + 1; 
    if (chart->ticks_temp == 2) { // e.g. 10° and 15°
        chart->ticks_temp = 6;    // -->  10°, 11°, 12°, 13°, 14°, 15°
    }

    chart->max_precipitation = MAX_DAILY_PRECIPITATION;  // Precipitation is fix
    chart->min_precipitation = 0;
    chart->ticks_precipitation = 5;

    chart->has_data = true;

    lv_obj_invalidate(obj);
}



/**********************
 *   STATIC FUNCTIONS
 **********************/

static void lv_daily_chart_constructor(const lv_obj_class_t * class_p, lv_obj_t * obj)
{
    LV_UNUSED(class_p);
    LV_TRACE_OBJ_CREATE("begin");

    lv_daily_chart_t * chart = (lv_daily_chart_t *)obj;

    chart->has_data = false;

    LV_TRACE_OBJ_CREATE("finished");
}


static void draw_daily_y_ticks(lv_obj_t * obj, lv_draw_ctx_t * draw_ctx, lv_daily_chart_axis_t axis)
{
    //LV_LOG_WARN("---------------------");   
    lv_daily_chart_t * chart  = (lv_daily_chart_t *)obj;

    int32_t ticks_cnt;
    int32_t min;
    int32_t max;

    lv_point_t p1;
    lv_point_t p2;

    lv_coord_t h     = lv_obj_get_content_height(obj);
    lv_coord_t w     = lv_obj_get_content_width(obj);
    
    lv_coord_t y_ofs = obj->coords.y1;

    lv_coord_t label_gap;
    lv_coord_t x_ofs;
    if(axis == LV_DAILY_CHART_AXIS_PRIMARY_Y) {
        ticks_cnt = chart->ticks_temp;
        min = chart->min_temp;
        max = chart->max_temp;

        label_gap = lv_obj_get_style_pad_left(obj, LV_PART_TICKS);
        x_ofs = obj->coords.x1;
    }
    else {
        ticks_cnt = chart->ticks_precipitation;
        min = chart->min_precipitation;
        max = chart->max_precipitation;

        label_gap = lv_obj_get_style_pad_right(obj, LV_PART_TICKS);
        x_ofs = obj->coords.x2;
    }

    //LV_LOG_WARN("y_ofs %d %d %d  ", y_ofs, obj->coords.y1 , pad_top);
    //LV_LOG_WARN("x_ofs %d %d  ", x_ofs, label_gap);
    //LV_LOG_WARN("h %d  ", h);

    lv_draw_label_dsc_t label_dsc;
    lv_draw_label_dsc_init(&label_dsc);
    lv_obj_init_draw_label_dsc(obj, LV_PART_TICKS, &label_dsc);

    lv_obj_draw_part_dsc_t part_draw_dsc;
    lv_obj_draw_dsc_init(&part_draw_dsc, draw_ctx);
    part_draw_dsc.class_p = MY_CLASS;
    part_draw_dsc.type = LV_DAILY_CHART_DRAW_PART_TICK_LABEL;
    part_draw_dsc.part = LV_PART_TICKS;
    part_draw_dsc.label_dsc = &label_dsc;

    //uint32_t total_tick_num = (ticks_cnt - 1);
 
    for(uint32_t i = 0; i < ticks_cnt; i++) {
        p2.y = p1.y = y_ofs + (int32_t)(h * i) / (ticks_cnt - 1);
        p2.x = p1.x = x_ofs;

        part_draw_dsc.p1 = &p1;
        part_draw_dsc.p2 = &p2;

        int32_t tick_value = lv_map(ticks_cnt - 1 - i, 0, (ticks_cnt - 1), min, max);
        part_draw_dsc.value = tick_value;

        char buf[LV_DAILY_CHART_LABEL_MAX_TEXT_LENGTH];
        lv_snprintf(buf, sizeof(buf), "%d", tick_value);
        part_draw_dsc.text = buf;
        part_draw_dsc.text_length = LV_DAILY_CHART_LABEL_MAX_TEXT_LENGTH;

        lv_event_send(obj, LV_EVENT_DRAW_PART_BEGIN, &part_draw_dsc);

        lv_point_t size;
        lv_txt_get_size(&size, part_draw_dsc.text, label_dsc.font, label_dsc.letter_space, label_dsc.line_space, LV_COORD_MAX, LV_TEXT_FLAG_NONE);

        lv_area_t a;
        a.y1 = p2.y - size.y / 2;
        a.y2 = p2.y + size.y / 2;

        if(axis == LV_DAILY_CHART_AXIS_PRIMARY_Y) {
            a.x1 = p2.x - size.x - label_gap;
            a.x2 = p2.x - label_gap;
        }
        else {
            a.x1 = p2.x + label_gap;
            a.x2 = p2.x + size.x + label_gap;
        }

        lv_draw_label(draw_ctx, &label_dsc, &a, part_draw_dsc.text, NULL);

        lv_event_send(obj, LV_EVENT_DRAW_PART_END, &part_draw_dsc);
    }
}

static void draw_daily_x_ticks(lv_obj_t * obj, lv_draw_ctx_t * draw_ctx)
{
    lv_daily_chart_t * chart  = (lv_daily_chart_t *)obj;

    lv_point_t p1;
    lv_point_t p2;

    lv_coord_t w     = lv_obj_get_content_width(obj);

    lv_coord_t label_gap = lv_obj_get_style_pad_bottom(obj, LV_PART_TICKS);
    lv_coord_t block_gap = lv_obj_get_style_pad_column(obj, LV_PART_MAIN);  /*Gap between the columns on ~adjacent X*/

    lv_draw_label_dsc_t label_dsc;
    lv_draw_label_dsc_init(&label_dsc);
    lv_obj_init_draw_label_dsc(obj, LV_PART_TICKS, &label_dsc);

    lv_coord_t x_ofs = obj->coords.x1;
    lv_coord_t y_ofs = obj->coords.y2;

    lv_obj_draw_part_dsc_t part_draw_dsc;
    lv_obj_draw_dsc_init(&part_draw_dsc, draw_ctx);
    part_draw_dsc.class_p = MY_CLASS;
    part_draw_dsc.type = LV_DAILY_CHART_DRAW_PART_TICK_LABEL;
    part_draw_dsc.part = LV_PART_TICKS;
    part_draw_dsc.label_dsc = &label_dsc;

    lv_coord_t block_w = (w + block_gap) / (NUM_DAYS);

    x_ofs += (block_w - block_gap) / 2;
    w -= block_w - block_gap;
  
    p1.y = y_ofs;
    for(uint32_t i = 0; i < NUM_DAYS; i++) {

        p2.x = p1.x = x_ofs + (int32_t)(w * i) / (NUM_DAYS - 1);
        p2.y = p1.y;

        part_draw_dsc.p1 = &p1;
        part_draw_dsc.p2 = &p2;

        char day_name[4];
        struct tm dt = chart->data_array[i].dt;
        sprintf(day_name, "%s", DAY_NAMES[dt.tm_wday]);

        part_draw_dsc.label_dsc = &label_dsc;
        part_draw_dsc.text = day_name;
        part_draw_dsc.text_length = LV_DAILY_CHART_LABEL_MAX_TEXT_LENGTH;

        lv_event_send(obj, LV_EVENT_DRAW_PART_BEGIN, &part_draw_dsc);

        lv_point_t size;
        lv_txt_get_size(&size, part_draw_dsc.text, label_dsc.font, label_dsc.letter_space, label_dsc.line_space, LV_COORD_MAX, LV_TEXT_FLAG_NONE);

        lv_area_t a;
        a.x1 = (p2.x - size.x / 2);
        a.x2 = (p2.x + size.x / 2);

        a.y1 = p2.y + label_gap;
        a.y2 = a.y1 + size.y;

        lv_draw_label(draw_ctx, &label_dsc, &a, part_draw_dsc.text, NULL);      

        lv_event_send(obj, LV_EVENT_DRAW_PART_END, &part_draw_dsc);
    }
}

static void draw_daily_div_lines(lv_obj_t * obj, lv_draw_ctx_t * draw_ctx)
{
    //LV_LOG_WARN("---------------------");   

    lv_daily_chart_t * chart  = (lv_daily_chart_t *)obj;

    lv_point_t p1;
    lv_point_t p2;

    lv_coord_t h     = lv_obj_get_content_height(obj);
    lv_coord_t w     = lv_obj_get_content_width(obj);

    lv_coord_t y_ofs = obj->coords.y1;
    lv_coord_t x_ofs = obj->coords.x1;

    int32_t ticks_cnt = chart->ticks_temp;

    lv_draw_line_dsc_t line_dsc;
    lv_draw_line_dsc_init(&line_dsc);
    lv_obj_init_draw_line_dsc(obj, LV_PART_ITEMS, &line_dsc);
    line_dsc.color = lv_color_hex(0x00000);
    line_dsc.opa = LV_OPA_50;
    line_dsc.width = 2;
    
    lv_obj_draw_part_dsc_t part_draw_dsc;
    lv_obj_draw_dsc_init(&part_draw_dsc, draw_ctx);
    part_draw_dsc.part = LV_PART_ITEMS;
    part_draw_dsc.class_p = MY_CLASS;
    part_draw_dsc.type = LV_DAILY_CHART_DRAW_PART_DIV_LINE_HOR;
    part_draw_dsc.line_dsc = &line_dsc;

    for(uint32_t i = 0; i < ticks_cnt; i++) {
        p2.y = p1.y = y_ofs + (int32_t)(h * i) / (ticks_cnt - 1);
        p1.x = x_ofs;
        p2.x = x_ofs + w;

        part_draw_dsc.p1 = &p1;
        part_draw_dsc.p2 = &p2;
        //LV_LOG_WARN("y_ofs %d %d %d %d ", p1.x, p1.y, p2.x, p2.y );

        lv_event_send(obj, LV_EVENT_DRAW_PART_BEGIN, &part_draw_dsc);
        lv_draw_line(draw_ctx, &line_dsc, &p1, &p2);
        lv_event_send(obj, LV_EVENT_DRAW_PART_END, &part_draw_dsc);
    }
}

static void draw_daily_clouds(lv_obj_t * obj, lv_draw_ctx_t * draw_ctx)
{
    lv_daily_chart_t * chart  = (lv_daily_chart_t *)obj;

    lv_point_t p1;
    lv_point_t p2;

    lv_area_t col_area;

    lv_coord_t w        = lv_obj_get_content_width(obj);
    lv_coord_t h        = lv_obj_get_content_height(obj);
    lv_coord_t pad_col  = lv_obj_get_style_pad_column(obj, LV_PART_MAIN);  /*Gap on left and right side of the bars*/

    lv_coord_t x_ofs = obj->coords.x1;
    lv_coord_t y_ofs = obj->coords.y1;

    lv_draw_rect_dsc_t col_dsc;
    lv_draw_rect_dsc_init(&col_dsc);
    lv_obj_init_draw_rect_dsc(obj, LV_PART_ITEMS, &col_dsc);
    col_dsc.bg_color = lv_color_hex(0xFAF02F);

    lv_obj_draw_part_dsc_t part_draw_dsc;
    lv_obj_draw_dsc_init(&part_draw_dsc, draw_ctx);
    part_draw_dsc.class_p = MY_CLASS;
    part_draw_dsc.type = LV_DAILY_CHART_DRAW_PART_CLOUDS;
    part_draw_dsc.part = LV_PART_ITEMS;

    p1.y = y_ofs;
    p2.y = y_ofs + h;

    for(uint32_t i = 0; i < NUM_DAYS; i++) { 
        p1.x = x_ofs + (int32_t)(w * i / NUM_DAYS) + pad_col;
        p2.x = x_ofs + (int32_t)(w * (i+1) / NUM_DAYS) - pad_col;

        col_area.x1 = p1.x;
        col_area.y1 = p1.y;
        col_area.x2 = p2.x;
        col_area.y2 = p2.y;

        col_dsc.bg_opa = (100 - chart->data_array[i].clouds) * 255 / 100; // We display sun, not clouds. Sun is 100 - clouds

        part_draw_dsc.draw_area = &col_area;
        part_draw_dsc.rect_dsc = &col_dsc;

        lv_event_send(obj, LV_EVENT_DRAW_PART_BEGIN, &part_draw_dsc);
        lv_draw_rect(draw_ctx, &col_dsc, &col_area);
        lv_event_send(obj, LV_EVENT_DRAW_PART_END, &part_draw_dsc);
    }
}


static void draw_daily_temp(lv_obj_t * obj, lv_draw_ctx_t * draw_ctx)
{
    //LV_LOG_WARN("---------------------");   

    lv_daily_chart_t * chart  = (lv_daily_chart_t *)obj;

    lv_point_t p1;
    lv_point_t p2;

    lv_coord_t w        = lv_obj_get_content_width(obj);
    lv_coord_t h        = lv_obj_get_content_height(obj);
    lv_coord_t pad_col  = lv_obj_get_style_pad_column(obj, LV_PART_MAIN);  /*Gap on left and right side of the bars*/

    lv_coord_t x_ofs = obj->coords.x1;
    lv_coord_t y_ofs = obj->coords.y1;

    int32_t min = chart->min_temp;
    int32_t max = chart->max_temp;        

    lv_draw_line_dsc_t line_dsc;
    lv_draw_line_dsc_init(&line_dsc);
    lv_obj_init_draw_line_dsc(obj, LV_PART_ITEMS, &line_dsc);
    line_dsc.width = 4;
    line_dsc.round_start = 0;
    line_dsc.round_end = 0;
    line_dsc.raw_end = 0;


    lv_obj_draw_part_dsc_t part_draw_dsc;
    lv_obj_draw_dsc_init(&part_draw_dsc, draw_ctx);
    part_draw_dsc.class_p = MY_CLASS;
    part_draw_dsc.type = LV_DAILY_CHART_DRAW_PART_TEMP;
    part_draw_dsc.part = LV_PART_ITEMS;
    part_draw_dsc.line_dsc = &line_dsc;

    for(uint32_t i = 0; i < NUM_DAYS; i++) { 
        p1.x = x_ofs + (int32_t)(w * i / NUM_DAYS) + pad_col;
        p2.x = x_ofs + (int32_t)(w * (i+1) / NUM_DAYS) - pad_col + 1; // for some reason it is one pixel smaller than clouds, so +1

        // High temperature
        int32_t high_value = h - lv_map_float(chart->data_array[i].high_temp, min, max, 0, h);

        p1.y = p2.y = y_ofs + high_value;

        part_draw_dsc.p1 = &p1;
        part_draw_dsc.p2 = &p2;

        line_dsc.color = lv_color_hex(0xF40000);

        lv_event_send(obj, LV_EVENT_DRAW_PART_BEGIN, &part_draw_dsc);
        lv_draw_line(draw_ctx, &line_dsc, &p1, &p2);
        lv_event_send(obj, LV_EVENT_DRAW_PART_END, &part_draw_dsc);

        // Low temperature
        int32_t low_value = h - lv_map_float(chart->data_array[i].low_temp, min, max, 0, h);

        p1.y = p2.y = y_ofs + low_value;

        part_draw_dsc.p1 = &p1;
        part_draw_dsc.p2 = &p2;

        line_dsc.color = lv_color_hex(0x0000F4);

        lv_event_send(obj, LV_EVENT_DRAW_PART_BEGIN, &part_draw_dsc);
        lv_draw_line(draw_ctx, &line_dsc, &p1, &p2);
        lv_event_send(obj, LV_EVENT_DRAW_PART_END, &part_draw_dsc);
    }
}



static void draw_daily_precipitation(lv_obj_t * obj, lv_draw_ctx_t * draw_ctx)
{
    lv_daily_chart_t * chart  = (lv_daily_chart_t *)obj;

    lv_point_t p1;
    lv_point_t p2;

    lv_area_t col_area;

    lv_coord_t w        = lv_obj_get_content_width(obj);
    lv_coord_t h        = lv_obj_get_content_height(obj);
    lv_coord_t pad_col  = lv_obj_get_style_pad_column(obj, LV_PART_MAIN);  /*Gap on left and right side of the bars*/

    lv_coord_t x_ofs = obj->coords.x1;
    lv_coord_t y_ofs = obj->coords.y1;

    lv_draw_rect_dsc_t col_dsc;
    lv_draw_rect_dsc_init(&col_dsc);
    lv_obj_init_draw_rect_dsc(obj, LV_PART_ITEMS, &col_dsc);


    lv_obj_draw_part_dsc_t part_draw_dsc;
    lv_obj_draw_dsc_init(&part_draw_dsc, draw_ctx);
    part_draw_dsc.class_p = MY_CLASS;
    part_draw_dsc.type = LV_DAILY_CHART_DRAW_PART_PRECIPITATION;
    part_draw_dsc.part = LV_PART_ITEMS;


    for(uint32_t i = 0; i < NUM_DAYS; i++) { 
        p1.x = x_ofs + (int32_t)(w * i / NUM_DAYS) + pad_col;
        p2.x = x_ofs + (int32_t)(w * (i+1) / NUM_DAYS) - pad_col;

        float rain = chart->data_array[i].rain;
        float snow = chart->data_array[i].snow;

        if (rain + snow > MAX_DAILY_PRECIPITATION) {
            float factor = MAX_DAILY_PRECIPITATION / (rain + snow);
            rain = rain * factor;
            snow = snow * factor;
        }

        int32_t rain_value = lv_map_float(rain, 0, MAX_DAILY_PRECIPITATION, 0, h);
        int32_t snow_value = lv_map_float(snow, 0, MAX_DAILY_PRECIPITATION, 0, h);


        // Rain
        if (rain_value > 0) {

            p1.y = y_ofs + h - rain_value;
            p2.y = y_ofs + h;

            col_area.x1 = p1.x;
            col_area.y1 = p1.y;
            col_area.x2 = p2.x;
            col_area.y2 = p2.y;

            part_draw_dsc.draw_area = &col_area;
            part_draw_dsc.rect_dsc = &col_dsc;

            lv_event_send(obj, LV_EVENT_DRAW_PART_BEGIN, &part_draw_dsc);
            col_dsc.bg_color = lv_color_hex(0xffffff);
            col_dsc.bg_opa = LV_OPA_100;
            lv_draw_rect(draw_ctx, &col_dsc, &col_area);
            col_dsc.bg_color = lv_color_hex(0x96C6F5);
            col_dsc.bg_opa = chart->data_array[i].pop * 255;
            lv_draw_rect(draw_ctx, &col_dsc, &col_area);
            lv_event_send(obj, LV_EVENT_DRAW_PART_END, &part_draw_dsc);
        }

        // Snow
        if (snow_value > 0) {

            p1.y = y_ofs + h - rain_value - snow_value;
            p2.y = y_ofs + h - rain_value;

            col_area.x1 = p1.x;
            col_area.y1 = p1.y;
            col_area.x2 = p2.x;
            col_area.y2 = p2.y;

            part_draw_dsc.draw_area = &col_area;
            part_draw_dsc.rect_dsc = &col_dsc;

            lv_event_send(obj, LV_EVENT_DRAW_PART_BEGIN, &part_draw_dsc);
            col_dsc.bg_color = lv_color_hex(0xffffff);
            col_dsc.bg_opa = LV_OPA_100;
            lv_draw_rect(draw_ctx, &col_dsc, &col_area);
            col_dsc.bg_color = lv_color_hex(0xEB8DFA);
            col_dsc.bg_opa = chart->data_array[i].pop * 255;
            lv_draw_rect(draw_ctx, &col_dsc, &col_area);
            lv_event_send(obj, LV_EVENT_DRAW_PART_END, &part_draw_dsc);
        }
    }
}

static void lv_daily_chart_event(const lv_obj_class_t * class_p, lv_event_t * e)
{
    LV_UNUSED(class_p);

    lv_res_t res;

    res = lv_obj_event_base(MY_CLASS, e);
    if(res != LV_RES_OK) return;

    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t * obj = lv_event_get_target(e);

    lv_daily_chart_t * chart  = (lv_daily_chart_t *)obj;
    
    if(code == LV_EVENT_DRAW_MAIN) {
        lv_draw_ctx_t * draw_ctx = lv_event_get_draw_ctx(e);

        if (chart->has_data) {
            draw_daily_y_ticks(obj, draw_ctx, LV_DAILY_CHART_AXIS_PRIMARY_Y);
            draw_daily_y_ticks(obj, draw_ctx, LV_DAILY_CHART_AXIS_SECONDARY_Y);
            draw_daily_x_ticks(obj, draw_ctx);

            draw_daily_clouds(obj, draw_ctx);
            draw_daily_precipitation(obj, draw_ctx);
            draw_daily_div_lines(obj, draw_ctx);
            draw_daily_temp(obj, draw_ctx);
        }
    }
    else if(code == LV_EVENT_GET_SELF_SIZE) {
        lv_point_t * p = lv_event_get_param(e);
        p->x = lv_obj_get_content_width(obj);
        p->y = lv_obj_get_content_height(obj);
    }
    else if(code == LV_EVENT_REFR_EXT_DRAW_SIZE) {
        lv_event_set_ext_draw_size(e, 30);
    }
    else if(code == LV_EVENT_SIZE_CHANGED) {
        lv_obj_refresh_self_size(obj);
    }
}

