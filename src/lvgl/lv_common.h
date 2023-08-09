/**
 * @file lv_common.h
 *
 */

#ifndef LV_COMMON_H
#define LV_COMMON_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/
#include "lv_conf_internal.h"
#include <stdint.h>


/**********************
 *      TYPEDEFS
 **********************/

typedef struct {
    float x;
    float y;
} lv_temp_t;

/*********************
 *      DEFINES
 *********************/


/**
 * Get the mapped of a number given an input and output range
 * @param x integer which mapped value should be calculated
 * @param min_in min input range
 * @param max_in max input range
 * @param min_out max output range
 * @param max_out max output range
 * @return the mapped number
 */
int32_t lv_map_float(float x, int32_t min_in, int32_t max_in, int32_t min_out, int32_t max_out);

float cubicInterpolation(lv_temp_t points[], int numPoints, float x);

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif
