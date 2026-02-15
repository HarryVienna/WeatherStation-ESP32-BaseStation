/**
 * @file lv_common.c
 *
 */

/*********************
 *      INCLUDES
 *********************/
#include "lv_common.h"

#include "../config/config.h"

/*********************
 *      DEFINES
 *********************/


/**********************
 *  STATIC PROTOTYPES
 **********************/

/**********************
 *  STATIC VARIABLES
 **********************/

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

/**
 * Get the mapped of a number given an input and output range
 * @param x integer which mapped value should be calculated
 * @param min_in min input range
 * @param max_in max input range
 * @param min_out max output range
 * @param max_out max output range
 * @return the mapped number
 */
int32_t lv_map_float(float x, int32_t min_in, int32_t max_in, int32_t min_out, int32_t max_out)
{
    if(max_in >= min_in && x >= max_in) return max_out;
    if(max_in >= min_in && x <= min_in) return min_out;

    if(max_in <= min_in && x <= max_in) return max_out;
    if(max_in <= min_in && x >= min_in) return min_out;

    /**
     * The equation should be:
     *   ((x - min_in) * delta_out) / delta in) + min_out
     * To avoid rounding error reorder the operations:
     *   (x - min_in) * (delta_out / delta_min) + min_out
     */

    int32_t delta_in = max_in - min_in;
    int32_t delta_out = max_out - min_out;

    if(delta_in == 0) return min_out;

    return (int32_t)(((x - min_in) * delta_out) / delta_in + min_out);
}


float cubicInterpolation(lv_temp_t points[], int numPoints, float x) {

    if (points == NULL || numPoints < 1) {
        return 0.0f;
    }
    if (numPoints == 1) {
        return points[0].y;
    }

    int i = 0;
    while (i < numPoints - 1 && points[i + 1].x < x) {
        i++;
    }

    float x0 = points[i].x;
    float x1 = points[i + 1].x;

    if (x1 == x0) {
        return (points[i].y + points[i + 1].y) / 2.0f;
    }
    float y0 = points[i].y;
    float y1 = points[i + 1].y;

    float m0, m1;
    if (i > 0) {
        float xDiffLeft = x0 - points[i - 1].x;
        float yDiffLeft = y0 - points[i - 1].y;
        float xDiffRight = points[i + 1].x - x0;
        float yDiffRight = points[i + 1].y - y0;
        m0 = (yDiffLeft * xDiffRight + yDiffRight * xDiffLeft) / (xDiffLeft + xDiffRight);
    }
    else {
        m0 = (y1 - y0) / (x1 - x0);
    }

    if (i < numPoints - 2) {
        float xDiffLeft = x1 - x0;
        float yDiffLeft = y1 - y0;
        float xDiffRight = points[i + 2].x - x1;
        float yDiffRight = points[i + 2].y - y1;
        m1 = (yDiffLeft * xDiffRight + yDiffRight * xDiffLeft) / (xDiffLeft + xDiffRight);
    }
    else {
        m1 = (y1 - y0) / (x1 - x0);
    }

    float t = (x - x0) / (x1 - x0);
    float t2 = t * t;
    float t3 = t2 * t;

    float a = 2.0f * t3 - 3.0f * t2 + 1.0f;
    float b = t3 - 2.0f * t2 + t;
    float c = -2.0f * t3 + 3.0f * t2;
    float d = t3 - t2;

    float interpolatedY = a * y0 + b * m0 + c * y1 + d * m1;
    return interpolatedY;
}

lv_color_t map_dewpoint_to_color(float value) {

    // Definiere die Farbwerte
    lv_color_t colors[] = {
        lv_color_hex(COLOR_BLUE),    // <= 12
        lv_color_hex(COLOR_ORANGE),  // ~ 14
        lv_color_hex(COLOR_RED)
    };
    int num_colors = sizeof(colors) / sizeof(colors[0]);

    float min_dew = 13.0;
    float max_dew = 18.0;

    if (value <= min_dew) {
        return colors[0];
    } else if (value >= max_dew) {
        return colors[num_colors - 1];
    } else {
        // Berechne den Index des Arrays für Werte zwischen `min` und `max`
        float scale = (value - min_dew) / (max_dew - min_dew);
        int index = (int)(scale * (num_colors - 1));

        return colors[index];
    }
}

lv_opa_t map_value_to_opacity(uint8_t value) {

    if (value <= 1) {
        return LV_OPA_0;
    } else if (value <= 2) {
        return LV_OPA_10;
    } else if (value <= 5) {
        return LV_OPA_20;
    } else if (value <= 15) {
        return LV_OPA_40;
    } else if (value <= 25) {
        return LV_OPA_10;
    } else if (value <= 35) {
        return LV_OPA_50;
    } else if (value <= 45) {
        return LV_OPA_60;
    } else if (value <= 55) {
        return LV_OPA_70;
    } else if (value <= 60) {
        return LV_OPA_80;
    } else if (value <= 85) {
        return LV_OPA_90;
    } else {
        return LV_OPA_100;
    }
}

/**********************
 *   STATIC FUNCTIONS
 **********************/
