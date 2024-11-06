#ifndef LV_SCREENSHOT_H
#define LV_SCREENSHOT_H

#ifdef __cplusplus
extern "C" {
#endif


#include <stdint.h>

typedef struct {
    uint32_t initial_delay;
    uint32_t task_delay;
} screenshot_task_params_t;

typedef struct {
    uint16_t file_type;      // File type identifier (BM)
    uint32_t file_size;      // Size of the BMP file in bytes
    uint16_t reserved1;     // Reserved; must be zero
    uint16_t reserved2;     // Reserved; must be zero
    uint32_t offset_data;    // Offset from beginning of file to the image data
} __attribute__((packed)) bitmap_fileheader_t;

typedef struct {
    uint32_t size;           // Size of this header (40 bytes)
    int32_t  width;          // Image width in pixels
    int32_t  height;         // Image height in pixels
    uint16_t planes;         // Number of color planes 
    uint16_t bit_count;      // Bits per pixel (24 for 24-bit RGB)
    uint32_t compression;    // Compression method (0 for none)
    uint32_t size_image;     // Size of the image data in bytes
    int32_t  x_pels_per_meter; // Horizontal resolution in pixels per meter
    int32_t  y_pels_per_meter; // Vertical resolution in pixels per meter
    uint32_t clr_used;       // Number of colors in the color palette
    uint32_t clr_important;  // Number of important colors used
} __attribute__((packed)) bitmap_infoheader_t;


void start_screenshot(uint32_t initial_delay, uint32_t task_delay);


#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /*LV_SCREENSHOT_H*/
