#include "sdkconfig.h"
#include "esp_log.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"

#include "driver/ledc.h"
#include "driver/gpio.h"
#include "driver/i2c.h"

#include "lvgl.h"
#include "esp32_s3.h"

#include "config/config.h"

// #define CONFIG_DOUBLE_FB 1

#if CONFIG_DOUBLE_FB
#define LCD_NUM_FB             2
#else
#define LCD_NUM_FB             1
#endif

static const char* TAG = "DISPLAY";

static void touchpad_read(lv_indev_drv_t *indev_driver, lv_indev_data_t *data);
static void lvgl_flush_cb(lv_disp_drv_t *drv, const lv_area_t *area, lv_color_t *color_map);
static bool on_vsync_event(esp_lcd_panel_handle_t panel, const esp_lcd_rgb_panel_event_data_t *event_data, void *user_data);

SemaphoreHandle_t lvgl_mux;
SemaphoreHandle_t sem_vsync_end;
SemaphoreHandle_t sem_gui_ready;

/**
 * @brief Initialize Display
 *
 * This function initializes the LCD display, including the backlight, touch panel, LCD panel, and LVGL library.
 */
void init_display(void)
{
    
    esp_lcd_panel_handle_t lcd = NULL;           // LCD panel handle
    esp_lcd_touch_handle_t tp = NULL;            // LCD touch panel handle

    init_backlight();
    init_touch(&tp);
    init_lcd(&lcd);
    init_lvgl(lcd, tp);

}

/**
 * @brief Initialize LCD Backlight
 *
 * This function initializes the PWM timer and channel configurations for controlling the LCD backlight.
 */
void init_backlight(void) {
    ledc_timer_config_t ledc_timer = {
        .duty_resolution = PWM_RESOLUTION,
        .freq_hz = PWM_FREQ,
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .timer_num = LEDC_TIMER,
        .clk_cfg = LEDC_AUTO_CLK
    };
    ledc_channel_config_t ledc_channel = {
        .channel = LEDC_CHANNEL,
        .duty = 0,
        .hpoint = 0,
        .intr_type = LEDC_INTR_DISABLE,
        .gpio_num = LEDC_PIN_NUM_BK_LIGHT,
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .timer_sel = LEDC_TIMER,
        .flags.output_invert = LEDC_OUTPUT_INVERT
    };

    ESP_LOGI(TAG, "Initializing LCD backlight");
    ledc_timer_config(&ledc_timer);
    ledc_channel_config(&ledc_channel);

    set_backlight_brightness(64);
}

/**
 * @brief Set LCD Backlight Brightness
 *
 * This function sets the brightness level of the LCD backlight.
 *
 * @param[in] brightness The brightness level to set (0-255).
 */
void set_backlight_brightness(uint8_t brightness) {
    //ESP_LOGI(TAG, "Setting LCD backlight brightness to %d", brightness);

    ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL, brightness);
    ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL);
}


/**
 * @brief Initialize Touch Driver
 *
 * This function installs the touch driver, configures the I2C interface for touch communication,
 * initializes the touch controller, and creates a touch handle for touch input.
 *
 * @param[out] touch_handle Pointer to the handle for the initialized touch controller.
 */
void init_touch(esp_lcd_touch_handle_t *touch_handle) {

    ESP_LOGI(TAG, "Install Touch driver");
    
    const i2c_config_t i2c_conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = I2C_SDA,
        .sda_pullup_en = GPIO_PULLUP_DISABLE,
        .scl_io_num = I2C_SCL,
        .scl_pullup_en = GPIO_PULLUP_DISABLE,
        .master.clk_speed = I2C_CLK_SPEED_HZ
    };
    ESP_LOGI(TAG, "i2c_param_config");
    i2c_param_config(I2C_NUM, &i2c_conf);
    ESP_LOGI(TAG, "i2c_driver_install");
    i2c_driver_install(I2C_NUM, i2c_conf.mode, 0, 0, 0);

    /* Initialize touch */
    const esp_lcd_touch_config_t tp_cfg = {
        .x_max = LCD_H_RES,
        .y_max = LCD_V_RES,
        .rst_gpio_num = I2C_RST, 
        .int_gpio_num = GPIO_NUM_NC,
        .levels = {
            .reset = 0,
            .interrupt = 0,
        },
        .flags = {
            .swap_xy = 0,
            .mirror_x = 1,
            .mirror_y = 1,
        },
    };
    esp_lcd_panel_io_handle_t tp_io_handle = NULL;

    const esp_lcd_panel_io_i2c_config_t tp_io_config = ESP_LCD_TOUCH_IO_I2C_GT911_CONFIG();

    ESP_LOGI(TAG, "Create LCD panel IO handle");
    esp_lcd_new_panel_io_i2c((esp_lcd_i2c_bus_handle_t)I2C_NUM, &tp_io_config, &tp_io_handle);
    ESP_LOGI(TAG, "Create a new GT911 touch driver");
    esp_lcd_touch_new_i2c_gt911(tp_io_handle, &tp_cfg, touch_handle);
}


/**
 * @brief Initialize RGB LCD Panel
 *
 * This function installs the RGB LCD panel driver, creates semaphores for synchronization,
 * configures the RGB LCD panel with the provided parameters, registers event callbacks,
 * resets and initializes the RGB LCD panel.
 *
 * @param[out] panel_handle Pointer to the handle for the initialized RGB LCD panel.
 */
void init_lcd(esp_lcd_panel_handle_t *panel_handle) {
    
    ESP_LOGI(TAG, "Install RGB LCD panel driver");

    sem_vsync_end = xSemaphoreCreateBinary();
    sem_gui_ready = xSemaphoreCreateBinary();

    esp_lcd_rgb_panel_config_t panel_config = {
        .data_width = 16, // RGB565 in parallel mode, thus 16bit in width
        .psram_trans_align = 64,
        .num_fbs = LCD_NUM_FB,
        .clk_src = LCD_CLK_SRC_DEFAULT,
        .disp_gpio_num = PIN_NUM_DISP_EN,
        .pclk_gpio_num = PIN_NUM_PCLK,
        .vsync_gpio_num = PIN_NUM_VSYNC,
        .hsync_gpio_num = PIN_NUM_HSYNC,
        .de_gpio_num = PIN_NUM_DE,
        .data_gpio_nums = {
            PIN_NUM_DATA0,
            PIN_NUM_DATA1,
            PIN_NUM_DATA2,
            PIN_NUM_DATA3,
            PIN_NUM_DATA4,
            PIN_NUM_DATA5,
            PIN_NUM_DATA6,
            PIN_NUM_DATA7,
            PIN_NUM_DATA8,
            PIN_NUM_DATA9,
            PIN_NUM_DATA10,
            PIN_NUM_DATA11,
            PIN_NUM_DATA12,
            PIN_NUM_DATA13,
            PIN_NUM_DATA14,
            PIN_NUM_DATA15,
        },
        .timings = {
            .pclk_hz = LCD_PIXEL_CLOCK_HZ,
            .h_res = LCD_H_RES,
            .v_res = LCD_V_RES,
            .hsync_back_porch = HSYNC_BACK_PORCH,
            .hsync_front_porch = HSYNC_FRONT_PORCH,
            .hsync_pulse_width = HSYNC_PULSE_WIDTH,
            .vsync_back_porch = VSYNC_BACK_PORCH,
            .vsync_front_porch = VSYNC_FRONT_PORCH,
            .vsync_pulse_width = VSYNC_PULSE_WIDTH,
            .flags.pclk_active_neg = true,
        },
        .flags.fb_in_psram = true, // allocate frame buffer in PSRAM
    };
    ESP_LOGI(TAG, "Create RGB LCD panel");
    esp_lcd_new_rgb_panel(&panel_config, panel_handle);

    ESP_LOGI(TAG, "Register event callbacks");
    esp_lcd_rgb_panel_event_callbacks_t cbs = {
        .on_vsync = on_vsync_event,
    };
    esp_lcd_rgb_panel_register_event_callbacks(*panel_handle, &cbs, NULL);


    ESP_LOGI(TAG, "Initialize RGB LCD panel");
    esp_lcd_panel_reset(*panel_handle);
    esp_lcd_panel_init(*panel_handle);

    // Rotate Display 180°
    esp_lcd_panel_mirror(*panel_handle, true, true);
    esp_lcd_panel_swap_xy(*panel_handle, false);

}

/**
 * @brief Initialize LVGL Library
 *
 * This function initializes the LVGL library, allocates separate draw buffers from PSRAM,
 * registers the display driver and input device driver to LVGL, creates a semaphore for
 * LVGL synchronization, and starts the LVGL port task.
 *
 * @param[in] panel_handle Handle to the LCD panel associated with LVGL.
 * @param[in] touch_handle Handle to the touchpad device associated with LVGL.
 */
void init_lvgl(esp_lcd_panel_handle_t panel_handle, esp_lcd_touch_handle_t touch_handle) {

    ESP_LOGI(TAG, "Initialize LVGL library");

    lvgl_mux = xSemaphoreCreateRecursiveMutex();

    static lv_disp_draw_buf_t disp_buf; // contains internal graphic buffer(s) called draw buffer(s)
    static lv_disp_drv_t disp_drv;      // contains callback functions
    
    lv_init();

    void *buf1 = NULL;
    void *buf2 = NULL;

    #if CONFIG_DOUBLE_FB
        ESP_LOGI(TAG, "Use frame buffers as LVGL draw buffers");
        ESP_ERROR_CHECK(esp_lcd_rgb_panel_get_frame_buffer(panel_handle, 2, &buf1, &buf2));
        // initialize LVGL draw buffers
        lv_disp_draw_buf_init(&disp_buf, buf1, buf2, LCD_H_RES * LCD_V_RES);
    #else
        ESP_LOGI(TAG, "Allocate separate LVGL draw buffers from PSRAM");
        buf1 = heap_caps_malloc(LCD_H_RES * 100 * sizeof(lv_color_t), MALLOC_CAP_SPIRAM);
        assert(buf1);
        // initialize LVGL draw buffers
        lv_disp_draw_buf_init(&disp_buf, buf1, buf2, LCD_H_RES * 100);
    #endif 

    ESP_LOGI(TAG, "Register display driver to LVGL");
    lv_disp_drv_init(&disp_drv);
    disp_drv.hor_res = LCD_H_RES;
    disp_drv.ver_res = LCD_V_RES;
    disp_drv.flush_cb = lvgl_flush_cb;
    disp_drv.draw_buf = &disp_buf;
    disp_drv.user_data = panel_handle;
    #if CONFIG_DOUBLE_FB
        disp_drv.full_refresh = true; // the full_refresh mode can maintain the synchronization between the two frame buffers
    #endif
    lv_disp_drv_register(&disp_drv);

    ESP_LOGI(TAG, "Register input device driver to LVGL");
    static lv_indev_drv_t indev_drv;
    lv_indev_drv_init(&indev_drv);
    indev_drv.type = LV_INDEV_TYPE_POINTER;
    indev_drv.read_cb = touchpad_read;
    indev_drv.user_data = touch_handle;
    lv_indev_drv_register(&indev_drv);
}

/**
 * @brief Touchpad Read Function
 *
 * This function reads touchpad input and updates the LVGL input device data accordingly.
 *
 * @param[in] indev_driver Pointer to the LVGL input device driver structure.
 * @param[out] data Pointer to the LVGL input device data structure to be updated.
 */
static void touchpad_read(lv_indev_drv_t *indev_driver, lv_indev_data_t *data)
{

    esp_lcd_touch_handle_t touch_handle = (esp_lcd_touch_handle_t)indev_driver->user_data;

    uint16_t touchpad_x;
    uint16_t touchpad_y;
    uint16_t touch_strength;
    uint8_t touch_cnt = 0;

    data->state = LV_INDEV_STATE_REL;

    esp_lcd_touch_read_data(touch_handle);
    bool touchpad_pressed = esp_lcd_touch_get_coordinates(touch_handle, &touchpad_x, &touchpad_y, &touch_strength, &touch_cnt, 1);
    if (touchpad_pressed) {
        //ESP_LOGI(TAG, "Touchpad_read %d %d", touchpad_x, touchpad_y);
        data->state = LV_INDEV_STATE_PR;

        /*Set the coordinates*/
        data->point.x = touchpad_x;
        data->point.y = touchpad_y;
    }
}

/**
 * @brief LVGL Flush Callback
 *
 * This callback function is called by LVGL to flush a portion of the display buffer to the physical display.
 * It passes the draw buffer to the LCD panel driver, indicating the area that needs to be updated.
 *
 * @param[in] drv Pointer to the display driver structure.
 * @param[in] area Pointer to the area that needs to be flushed.
 * @param[in] color_map Pointer to the color map containing pixel data to be flushed.
 */
static void lvgl_flush_cb(lv_disp_drv_t *drv, const lv_area_t *area, lv_color_t *color_map)
{
    esp_lcd_panel_handle_t panel_handle = (esp_lcd_panel_handle_t) drv->user_data;

    int offsetx1 = area->x1;
    int offsetx2 = area->x2;
    int offsety1 = area->y1;
    int offsety2 = area->y2;

    // LVGL has finished
    //xSemaphoreGive(sem_gui_ready);
    // Now wait for the VSYNC event. 
    //xSemaphoreTake(sem_vsync_end, portMAX_DELAY);

    // pass the draw buffer to the driver
    esp_lcd_panel_draw_bitmap(panel_handle, offsetx1, offsety1, offsetx2 + 1, offsety2 + 1, color_map);
    lv_disp_flush_ready(drv);
}

/**
 * @brief Handles VSYNC events for an ESP32 LCD panel.
 *
 * This function waits until LVGL has finished its operations, indicated by the
 * `sem_gui_ready` semaphore being taken. Once LVGL is ready, it signals that
 * the VSYNC event has ended by giving the `sem_vsync_end` semaphore.
 *
 * @param[in] panel Handle to the LCD panel associated with the VSYNC event.
 * @param[in] event_data Pointer to a structure containing data related to the VSYNC event.
 * @param[in] user_data User data pointer passed when registering the VSYNC event handler.
 * @return
 *     - `true` if the task was woken up due to handling the VSYNC event, indicating that it's safe
 *       to proceed with flushing the buffer.
 *     - `false` otherwise.
 */
static bool on_vsync_event(esp_lcd_panel_handle_t panel, const esp_lcd_rgb_panel_event_data_t *event_data, void *user_data)
{
    BaseType_t high_task_awoken = pdFALSE;

    // Wait until LVGL has finished 
    //if (xSemaphoreTakeFromISR(sem_gui_ready, &high_task_awoken) == pdTRUE) {
        // Indicate that the VSYNC event has ended, and it's safe to proceed with flushing the buffer.
        xSemaphoreGiveFromISR(sem_vsync_end, &high_task_awoken);
    //}

    return high_task_awoken == pdTRUE;
}

