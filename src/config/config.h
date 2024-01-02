#ifndef CONFIG_H
#define CONFIG_H

#define ADC_PIN 17   // This pin works better for the ADC since there is no pullup resister in the display board                                                                                                                                                                                                        

#define I2C_SCL 13
#define I2C_SDA 12
#define I2C_FREQ 100000


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
#define COLOR_DARKBLUE 0x00007F
#define COLOR_GREY 0x2E2E2E
#define COLOR_BLACK 0x000000

static const char* DAY_NAMES[7] = { "So", "Mo", "Di", "Mi" , "Do" , "Fr" , "Sa" };

#endif