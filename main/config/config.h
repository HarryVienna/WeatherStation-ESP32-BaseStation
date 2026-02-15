#ifndef CONFIG_H
#define CONFIG_H


// Colors
#define COLOR_RED 0xF40000
#define COLOR_ORANGE 0xF56101
#define COLOR_YELLOW 0xF5C700
#define COLOR_LIGHTYELLOW 0xFAF02F
#define COLOR_LIGHTGREEN 0x95C700
#define COLOR_GREEN 0x027C00
#define COLOR_AQUA 0x2FC7C6
#define COLOR_BABYBLUE 0x96C6F5
#define COLOR_LIGHTBLUE 0x2E61F5
#define COLOR_BLUE 0x0000F4
#define COLOR_PURPLE 0x302E97
#define COLOR_PINK 0xEB8DFA
#define COLOR_DARKPINK 0xFA00AC
#define COLOR_DARKBLUE 0x00007F
#define COLOR_GREY 0x2E2E2E
#define COLOR_BLACK 0x000000
#define COLOR_WHITE 0xFFFFFF

// Day names
static const char* DAY_NAMES[7] = { "So", "Mo", "Di", "Mi" , "Do" , "Fr" , "Sa" };

// WIFI
#define HOST_NAME   "ESP32-WEATHERSTATION"
#define NTP_SERVER  "de.pool.ntp.org"


// PWM Configuration
#define PWM_FREQ               200 
#define PWM_RESOLUTION         LEDC_TIMER_10_BIT 
#define LEDC_CHANNEL           LEDC_CHANNEL_0  
#define LEDC_TIMER             LEDC_TIMER_0 
#define LEDC_PIN_NUM_BK_LIGHT  GPIO_NUM_10
#define LEDC_OUTPUT_INVERT     1

// I2C
#define I2C_SCL          GPIO_NUM_18
#define I2C_SDA          GPIO_NUM_17
#define I2C_RST          GPIO_NUM_38
#define I2C_CLK_SPEED_HZ 100000
#define I2C_NUM          I2C_NUM_0

// LCD
#define LCD_PIXEL_CLOCK_HZ     (16 * 1000 * 1000)

#define PIN_NUM_HSYNC          GPIO_NUM_39
#define PIN_NUM_VSYNC          GPIO_NUM_41
#define PIN_NUM_DE             GPIO_NUM_40
#define PIN_NUM_PCLK           GPIO_NUM_42
#define PIN_NUM_DATA0          GPIO_NUM_8   // B0
#define PIN_NUM_DATA1          GPIO_NUM_3   // B1
#define PIN_NUM_DATA2          GPIO_NUM_46  // B2
#define PIN_NUM_DATA3          GPIO_NUM_9   // B3
#define PIN_NUM_DATA4          GPIO_NUM_1   // B4
#define PIN_NUM_DATA5          GPIO_NUM_5   // G0
#define PIN_NUM_DATA6          GPIO_NUM_6   // G1
#define PIN_NUM_DATA7          GPIO_NUM_7   // G2
#define PIN_NUM_DATA8          GPIO_NUM_15  // G3
#define PIN_NUM_DATA9          GPIO_NUM_16  // G4
#define PIN_NUM_DATA10         GPIO_NUM_4   // G5
#define PIN_NUM_DATA11         GPIO_NUM_45  // R0
#define PIN_NUM_DATA12         GPIO_NUM_48  // R1
#define PIN_NUM_DATA13         GPIO_NUM_47  // R2
#define PIN_NUM_DATA14         GPIO_NUM_21  // R3
#define PIN_NUM_DATA15         GPIO_NUM_14  // R4
#define PIN_NUM_DISP_EN        GPIO_NUM_NC

#define HSYNC_BACK_PORCH  16
#define HSYNC_FRONT_PORCH 80
#define HSYNC_PULSE_WIDTH 4
#define VSYNC_BACK_PORCH  4
#define VSYNC_FRONT_PORCH 22
#define VSYNC_PULSE_WIDTH 4

#define LCD_H_RES         1024
#define LCD_V_RES         600

// LVGL
#define LVGL_TICK_PERIOD_MS 2
#define LVGL_TASK_MAX_DELAY_MS 500
#define LVGL_TASK_MIN_DELAY_MS 10
#define LVGL_TASK_STACK_SIZE (4 * 1024)
#define LVGL_TASK_PRIORITY   0

// Chart Widgets
#define NUM_HOURS 48
#define MAX_HOURLY_PRECIPITATION 5

#define NUM_DAYS 7
#define MAX_DAILY_PRECIPITATION 20

#endif /* CONFIG_H */