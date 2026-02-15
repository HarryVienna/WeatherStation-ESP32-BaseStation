#include <math.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_log.h"

#include "nvs/preferences.h"

#include "task/clock_task.h"
#include "task/wifiscan_task.h"
#include "task/wificonnect_task.h"
#include "task/sensor_scd41_task.h"
#include "task/sensor_sen55_task.h"
#include "task/weather_task.h"
#include "task/brightness_task.h"

#include "wifi/network.h"

#include "config/config.h"
#include "gui.h"
#include "lvgl/lv_hourly_chart.h"
#include "lvgl/lv_daily_chart.h"
#include "lvgl/lv_screenshot.h"

static const char* TAG = "GUI";

extern SemaphoreHandle_t lvgl_mux;

#define NUM_ICONS 28

typedef struct
{
  const uint8_t icon;
  const lv_img_dsc_t *icon_image;
} icon_mapping_t;

icon_mapping_t icon_mapping_day[] = {
      {0, &img_0d},
      {1, &img_1d},
      {2, &img_2},
      {3, &img_3},
      {45, &img_45},
      {48, &img_48},
      {51, &img_51},
      {53, &img_53},
      {55, &img_55},
      {56, &img_56},
      {57, &img_57},
      {61, &img_61},
      {63, &img_63},
      {65, &img_65},
      {66, &img_66},
      {67, &img_67},
      {71, &img_71},
      {73, &img_73},
      {75, &img_75},
      {77, &img_77},
      {80, &img_80},
      {81, &img_81},
      {82, &img_82},
      {85, &img_85},
      {86, &img_86},
      {95, &img_95},
      {96, &img_96},
      {99, &img_99}};

icon_mapping_t icon_mapping_night[] = {
      {0, &img_0n},
      {1, &img_1n},
      {2, &img_2},
      {3, &img_3},
      {45, &img_45},
      {48, &img_48},
      {51, &img_51},
      {53, &img_53},
      {55, &img_55},
      {56, &img_56},
      {57, &img_57},
      {61, &img_61},
      {63, &img_63},
      {65, &img_65},
      {66, &img_66},
      {67, &img_67},
      {71, &img_71},
      {73, &img_73},
      {75, &img_75},
      {77, &img_77},
      {80, &img_80},
      {81, &img_81},
      {82, &img_82},
      {85, &img_85},
      {86, &img_86},
      {95, &img_95},
      {96, &img_96},
      {99, &img_99}};      

const char *regionNames[] = {
    "Africa", "America", "Antarctica", "Arctic", "Asia", "Atlantic", "Australia", "Europe", "Indian", "Pacific"};

const char *cityData[][3] = {
    {"Africa", "(GMT) Casablanca", "WET0WEST,M3.5.0,M10.5.0/3"},
    {"Africa", "(GMT +01:00) West Central Africa", "WAT-1"},
    {"Africa", "(GMT +02:00) Harare, Pretoria", "CAT-2"},
    {"Africa", "(GMT +02:00) Windhoek", "WAT-1WAST,M9.1.0,M4.1.0"},
    {"Africa", "(GMT +02:00) Cairo", "EET-2"},
    {"Africa", "(GMT +03:00) Nairobi", "EAT-3"},

    {"America", "(GMT -03:00) Buenos Aires", "ART3"},
    {"America", "(GMT -03:00) Brasilia", "BRT3BRST,M10.3.0/0,M2.3.0/0"},
    {"America", "(GMT -03:00) Greenland", "WGT3WGST,M3.5.0/-2,M10.5.0/-1"},
    {"America", "(GMT -03:00) Montevideo", "UYT3"},
    {"America", "(GMT -03:30) Newfoundland", "NST3:30NDT,M3.2.0,M11.1.0"},
    {"America", "(GMT -03:00) Cayenne, Fortaleza", "GFT3"},
    {"America", "(GMT -04:00) Atlantic Time (Canada)", "AST4ADT,M3.2.0,M11.1.0"},
    {"America", "(GMT -04:00) Cuiaba", "AMT4AMST,M10.3.0/0,M2.3.0/0"},
    {"America", "(GMT -04:00) Santiago", "CLT3"},
    {"America", "(GMT -04:00) Asuncion", "PYT4PYST,M10.1.0/0,M3.4.0/0"},
    {"America", "(GMT -04:00) Georgetown, La Paz, Manaus, San Juan", "BOT4"},
    {"America", "(GMT -04:30) Caracas", "VET4:30"},
    {"America", "(GMT -05:00) Eastern Time (US & Canada)", "EST5EDT,M3.2.0,M11.1.0"},
    {"America", "(GMT -05:00) Bogota, Lima, Quito", "COT5"},
    {"America", "(GMT -05:00) Indiana (East)", "EST5EDT,M3.2.0,M11.1.0"},
    {"America", "(GMT -06:00) Saskatchewan", "CST6"},
    {"America", "(GMT -06:00) Central America", "CST6"},
    {"America", "(GMT -06:00) Guadalajara, Mexico City, Monterrey", "CST6CDT,M4.1.0,M10.5.0"},
    {"America", "(GMT -06:00) Central Time (US & Canada)", "CST6CDT,M3.2.0,M11.1.0"},
    {"America", "(GMT -07:00) Chihuahua, La Paz, Mazatlan", "MST7MDT,M4.1.0,M10.5.0"},
    {"America", "(GMT -07:00) Mountain Time (US & Canada)", "MST7MDT,M3.2.0,M11.1.0"},
    {"America", "(GMT -07:00) Arizona", "MST7"},
    {"America", "(GMT -08:00) Baja California", "PST8PDT,M3.2.0,M11.1.0"},
    {"America", "(GMT -08:00) Pacific Time (US & Canada)", "PST8PDT,M3.2.0,M11.1.0"},
    {"America", "(GMT -09:00) Alaska", "AKST9AKDT,M3.2.0,M11.1.0"},
    {"America", "(GMT -10:00) Hawaii-Aleutian", "HST10HDT,M3.2.0,M11.1.0"},

    {"Antarctica", "(GMT +12:00) McMurdo", "NZST-12NZDT,M9.5.0,M4.1.0/3"},
    {"Antarctica", "(GMT +11:00) Macquarie", "MIST-11"},
    {"Antarctica", "(GMT +10:00) DumontDUrville", "DDUT-10"},
    {"Antarctica", "(GMT +08:00) Casey", "AWST-8"},
    {"Antarctica", "(GMT +07:00) Davis", "DAVT-7"},
    {"Antarctica", "(GMT +06:00) Vostok", "VOST-6"},
    {"Antarctica", "(GMT +05:00) Mawson", "MAWT-5"},
    {"Antarctica", "(GMT +03:00) Syowa", "SYOT-3"},
    {"Antarctica", "(GMT -04:00) Palmer", "CLT3"},
    {"Antarctica", "(GMT -04:00) Rothera", "ROTT3"},

    {"Arctic", "(GMT +01:00) Longyearbyen", "CET-1CEST,M3.5.0,M10.5.0/3"},

    {"Asia", "(GMT +12:00) Petropavlovsk-Kamchatsky", "PETT-12"},
    {"Asia", "(GMT +11:00) Magadan", "MAGT-1"},
    {"Asia", "(GMT +10:00) Vladivostok", "VLAT-10"},
    {"Asia", "(GMT +09:00) Yakutsk", "YAKT-9"},
    {"Asia", "(GMT +09:00) Osaka, Sapporo, Tokyo", "JST-9"},
    {"Asia", "(GMT +09:00) Seoul", "KST-9"},
    {"Asia", "(GMT +08:00) Kuala Lumpur, Singapore", "SGT-8"},
    {"Asia", "(GMT +08:00) Ulaanbaatar", "ULAT-8ULAST,M3.5.6,M9.5.6/0"},
    {"Asia", "(GMT +08:00) Taipei", "CST-8"},
    {"Asia", "(GMT +08:00) Irkutsk", "IRKT-8"},
    {"Asia", "(GMT +08:00) Beijing, Chongqing, Hong Kong, Urumqi", "HKT-8"},
    {"Asia", "(GMT +07:00) Bangkok, Hanoi, Jakarta", "=WIB-7"},
    {"Asia", "(GMT +07:00) Krasnoyarsk", "KRAT-7"},
    {"Asia", "(GMT +06:30) Yangon (Rangoon)", "UNK-6:30"},
    {"Asia", "(GMT +06:00) Novosibirsk", "NOVT-6"},
    {"Asia", "(GMT +06:00) Astana", "(GMT-4"},
    {"Asia", "(GMT +06:00) Dhaka", "BDT-6"},
    {"Asia", "(GMT +05:45) Kathmandu", "NPT-5:45"},
    {"Asia", "(GMT +05:30) Sri Jayawardenepura", "IST-5:30"},
    {"Asia", "(GMT +05:30) Chennai, Kolkata, Mumbai, New Delhi", "IST-5:30"},
    {"Asia", "(GMT +05:00) Tashkent", "UZT-5"},
    {"Asia", "(GMT +05:00) Islamabad, Karachi", "PKT-5"},
    {"Asia", "(GMT +05:00) Ekaterinburg", "YEKT-5"},
    {"Asia", "(GMT +04:00) Tbilisi", "GET-4"},
    {"Asia", "(GMT +04:00) Yerevan", "AMT-4"},
    {"Asia", "(GMT +04:00) Baku", "AZT-4AZST,M3.5.0/4,M10.5.0/5"},
    {"Asia", "(GMT +04:00) Abu Dhabi, Muscat", "GST-4"},
    {"Asia", "(GMT +04:30) Kabul", "AFT-4:30"},
    {"Asia", "(GMT +03:30) Tehran", "IRST-3:30IRDT,80/0,264/0"},
    {"Asia", "(GMT +03:00) Baghdad", "AST-3"},
    {"Asia", "(GMT +03:00) Kuwait, Riyadh", "AST-3"},
    {"Asia", "(GMT +02:00) Damascus", "EET-2EEST,M3.5.5/0,M10.5.5/0"},
    {"Asia", "(GMT +02:00) Beirut", "EET-2EEST,M3.5.0/0,M10.5.0/0"},
    {"Asia", "(GMT +02:00) Amman", "EET-2EEST,M3.5.4/24,M10.5.5/1"},
    {"Asia", "(GMT +02:00) Jerusalem", "IST-2IDT,M3.4.4/26,M10.5.0"},

    {"Atlantic", "(GMT) Monrovia, Reykjavik", "GMT0"},
    {"Atlantic", "(GMT -01:00) Azores", "AZOT1AZOST,M3.5.0/0,M10.5.0/1"},
    {"Atlantic", "(GMT -01:00) Cape Verde Is.", "CVT1"},

    {"Australia", "(GMT +10:00) Hobart", "AEST-10AEDT,M10.1.0,M4.1.0/3"},
    {"Australia", "(GMT +10:00) Brisbane", "AEST-10"},
    {"Australia", "(GMT +10:00) Canberra, Melbourne, Sydney", "AEST-10AEDT,M10.1.0,M4.1.0/3"},
    {"Australia", "(GMT +09:30) Adelaide", "ACST-9:30ACDT,M10.1.0,M4.1.0/3"},
    {"Australia", "(GMT +09:30) Darwin", "ACST-9:30"},
    {"Australia", "(GMT +08:00) Perth", "AWST-8"},

    {"Europe", "(GMT) Dublin, Edinburgh, Lisbon, London", "GMT0BST,M3.5.0/1,M10.5.0"},
    {"Europe", "(GMT +01:00) Belgrade, Bratislava, Budapest, Ljubljana, Prague", "CET-1CEST,M3.5.0,M10.5.0/3"},
    {"Europe", "(GMT +01:00) Sarajevo, Skopje, Warsaw, Zagreb", "CET-1CEST,M3.5.0,M10.5.0/3"},
    {"Europe", "(GMT +01:00) Brussels, Copenhagen, Madrid, Paris", "CET-1CEST,M3.5.0,M10.5.0/3"},
    {"Europe", "(GMT +01:00) Amsterdam, Berlin, Bern, Rome, Stockholm, Vienna", "CET-1CEST,M3.5.0,M10.5.0/3"},
    {"Europe", "(GMT +02:00) Chisinau", "EET-2EEST,M3.5.0,M10.5.0/3"},
    {"Europe", "(GMT +02:00) Helsinki, Kyiv, Riga, Sofia, Tallinn, Vilnius", "EET-2EEST,M3.5.0/3,M10.5.0/4"},
    {"Europe", "(GMT +02:00) Athens, Bucharest", "EET-2EEST,M3.5.0/3,M10.5.0/4"},
    {"Europe", "(GMT +03:00) Minsk", "MSK-3"},
    {"Europe", "(GMT +03:00) Moscow, St. Petersburg, Volgograd", "MSK-3"},
    {"Europe", "(GMT +03:00) Istanbul", "EET-3"},

    {"Indian", "(GMT +04:00) Port Louis", "MUT-4"},
    {"Indian", "(GMT +07:00) Christmas Island", "CXT-7"},

    {"Pacific", "(GMT +13:00) Nuku'alofa", "TOT-13"},
    {"Pacific", "(GMT +12:00) Fiji, Marshall Is.", "FJT-12FJST,M11.1.0,M1.3.0/3"},
    {"Pacific", "(GMT +12:00) Auckland, Wellington", "NZST-12NZDT,M9.5.0,M4.1.0/3"},
    {"Pacific", "(GMT +11:00) Solomon Is., New Caledonia", "SBT-11"},
    {"Pacific", "(GMT +10:00) Guam, Port Moresby", "PGT-10"},
    {"Pacific", "(GMT -11:00) Samoa", "WSST-13WSDT,M9.5.0/3,M4.1.0/4"},
    {"Pacific", "(GMT -10:00) Hawaii", "HST10"},
    {"Pacific", "(GMT -05:00) Easter Island", "EAST5"}};



/**
 * @brief     Calculate sea level pressure based on provided parameters
 *
 * @param     pressure      Atmospheric pressure at the measurement point (in hPa)
 * @param     temperature   Temperature at the measurement point (in Celsius)
 * @param     altitude      Altitude above sea level (in meters)
 *
 * @return    float         Sea level pressure calculated based on the parameters (in hPa)
 *
 * @details   Calculates and estimates the sea level pressure using the barometric formula.
 *            Incorporates constants and calculations to adjust the pressure for altitude and temperature.
 */
float calc_sea_level_pressure(float pressure, float temperature, uint16_t altitude)
{
  // https://de.wikipedia.org/wiki/Barometrische_H%C3%B6henformel

  // Konstanten
  float g = 9.80665;  // Schwerebeschleunigung in m / s^2
  float R = 287.05;   // Gaskonstante trockener Luft (= R/M)  in m^2/(s²K)
  float a = 0.0065;   // vertikaler Temperaturgradient
  float C_h = 0.12;   // Beiwert zur Berücksichtigung der mittleren Dampfdruckänderung K/hPa
  float T_0 = 273.15; // Celsius to Kelvin

  float E; // Dampfdruck des Wasserdampfanteils (in hPa)

  if (temperature < 9.1)
  {
    E = 5.6402 * (-0.0916 + exp(0.06 * temperature));
  }
  else
  {
    E = 18.2194 * (1.0463 + exp(-0.0666 * temperature));
  }

  // Luftdruck auf Meereshöhe berechnen
  float p = pressure * exp(altitude * g / (R * (temperature + T_0 + C_h * E + a * (altitude / 2))));

  return p;
}

// -------- Weatherstation Screen --------

void disp_wifi_status(bool status)
{
  if (status)
  {
    lv_img_set_src(objects.wifi, &img_wifi_on);
  }
  else
  {
    lv_img_set_src(objects.wifi, &img_wifi_off);
  }
}

void disp_date_time(char *date_time)
{
  lv_label_set_text(objects.date_time, date_time);
}

void disp_sensor_data(uint8_t sensor_nr, double temperature, double humidity, double pressure, uint32_t voltage, char *date_time)
{

  nvs_handle_t nvs_handle;
  nvs_open("weatherstation", NVS_READONLY, &nvs_handle);

  const char* height_c = get_string_from_nvs(nvs_handle, "height", "0");

  nvs_close(nvs_handle);

  uint16_t height = atol(height_c);

  pressure = calc_sea_level_pressure(pressure, temperature, height);

  char temperature_value[16];
  char humidity_value[16];
  char pressure_value[16];
  char voltage_value[16];

  sprintf(temperature_value, "%.1f", temperature);
  sprintf(humidity_value, "%.1f", humidity);
  sprintf(pressure_value, "%.0f", pressure);
  sprintf(voltage_value, "%.1f V", voltage / 1000.0);

  float voltage_rounded;
  sscanf(voltage_value, "%f", &voltage_rounded);

  switch (sensor_nr)
  {
  case 0:
    lv_label_set_text(objects.temp0, temperature_value);
    lv_label_set_text(objects.hunidity0, humidity_value);
    lv_label_set_text(objects.pressure0, pressure_value);
    lv_label_set_text(objects.volt0, voltage_value);
    lv_label_set_text(objects.update0, date_time);

    if (voltage_rounded >= 4.0)
    {
      lv_obj_set_style_bg_color(objects.line_status0, lv_color_hex(COLOR_GREEN), LV_PART_MAIN | LV_STATE_DEFAULT);
    }
    else if (voltage_rounded >= 3.8)
    {
      lv_obj_set_style_bg_color(objects.line_status0, lv_color_hex(COLOR_LIGHTGREEN), LV_PART_MAIN | LV_STATE_DEFAULT);
    }
    else if (voltage_rounded >= 3.6)
    {
      lv_obj_set_style_bg_color(objects.line_status0, lv_color_hex(COLOR_ORANGE), LV_PART_MAIN | LV_STATE_DEFAULT);
    }
    else
    {
      lv_obj_set_style_bg_color(objects.line_status0, lv_color_hex(COLOR_RED), LV_PART_MAIN | LV_STATE_DEFAULT);
    }
    break;
  case 1:
    lv_label_set_text(objects.temp1, temperature_value);
    lv_label_set_text(objects.hunidity1, humidity_value);
    lv_label_set_text(objects.pressure1, pressure_value);
    lv_label_set_text(objects.volt1, voltage_value);
    lv_label_set_text(objects.update1, date_time);
    if (voltage_rounded >= 4.0)
    {
      lv_obj_set_style_bg_color(objects.line_status1, lv_color_hex(COLOR_GREEN), LV_PART_MAIN | LV_STATE_DEFAULT);
    }
    else if (voltage_rounded >= 3.8)
    {
      lv_obj_set_style_bg_color(objects.line_status1, lv_color_hex(COLOR_LIGHTGREEN), LV_PART_MAIN | LV_STATE_DEFAULT);
    }
    else if (voltage_rounded >= 3.6)
    {
      lv_obj_set_style_bg_color(objects.line_status1, lv_color_hex(COLOR_ORANGE), LV_PART_MAIN | LV_STATE_DEFAULT);
    }
    else
    {
      lv_obj_set_style_bg_color(objects.line_status1, lv_color_hex(COLOR_RED), LV_PART_MAIN | LV_STATE_DEFAULT);
    }
    break;
  case 2:
    lv_label_set_text(objects.temp2, temperature_value);
    lv_label_set_text(objects.hunidity2, humidity_value);
    lv_label_set_text(objects.pressure2, pressure_value);
    lv_label_set_text(objects.volt2, voltage_value);
    lv_label_set_text(objects.update2, date_time);
    if (voltage_rounded >= 4.0)
    {
      lv_obj_set_style_bg_color(objects.line_status2, lv_color_hex(COLOR_GREEN), LV_PART_MAIN | LV_STATE_DEFAULT);
    }
    else if (voltage_rounded >= 3.8)
    {
      lv_obj_set_style_bg_color(objects.line_status2, lv_color_hex(COLOR_LIGHTGREEN), LV_PART_MAIN | LV_STATE_DEFAULT);
    }
    else if (voltage_rounded >= 3.6)
    {
      lv_obj_set_style_bg_color(objects.line_status2, lv_color_hex(COLOR_ORANGE), LV_PART_MAIN | LV_STATE_DEFAULT);
    }
    else
    {
      lv_obj_set_style_bg_color(objects.line_status2, lv_color_hex(COLOR_RED), LV_PART_MAIN | LV_STATE_DEFAULT);
    }
    break;
  default:
    break;
  }
}

void disp_scd4x(uint16_t co2)
{
  if (co2 <= 600)
  {
    lv_obj_set_style_bg_color(objects.co2, lv_color_hex(COLOR_GREEN), LV_PART_MAIN | LV_STATE_DEFAULT);
  }
  else if (co2 <= 1000)
  {
    lv_obj_set_style_bg_color(objects.co2, lv_color_hex(COLOR_LIGHTGREEN), LV_PART_MAIN | LV_STATE_DEFAULT);
  }
  else if (co2 <= 1500)
  {
    lv_obj_set_style_bg_color(objects.co2, lv_color_hex(COLOR_YELLOW), LV_PART_MAIN | LV_STATE_DEFAULT);
  }
  else if (co2 <= 1900)
  {
    lv_obj_set_style_bg_color(objects.co2, lv_color_hex(COLOR_ORANGE), LV_PART_MAIN | LV_STATE_DEFAULT);
  }
  else
  {
    lv_obj_set_style_bg_color(objects.co2, lv_color_hex(COLOR_RED), LV_PART_MAIN | LV_STATE_DEFAULT);
  }
}

void disp_sen5x(float ambientTemperature, float ambientHumidity, float massConcentrationPm1p0, float massConcentrationPm2p5, float massConcentrationPm4p0, float massConcentrationPm10p0, float vocIndex, float noxIndex)
{

  if (!isnan(ambientTemperature))
  {
    char temp[8];
    sprintf(temp, "%.1f", ambientTemperature);
    lv_label_set_text(objects.temp_base, temp);
  }

  if (!isnan(ambientHumidity))
  {
    char humidity[8];
    sprintf(humidity, "%.1f", ambientHumidity);
    lv_label_set_text(objects.humidity_base, humidity);
  }

  if (massConcentrationPm1p0 <= 11.6)
  {
    lv_obj_set_style_bg_color(objects.pm1, lv_color_hex(COLOR_GREEN), LV_PART_MAIN | LV_STATE_DEFAULT);
  }
  else if (massConcentrationPm1p0 <= 32)
  {
    lv_obj_set_style_bg_color(objects.pm1, lv_color_hex(COLOR_LIGHTGREEN), LV_PART_MAIN | LV_STATE_DEFAULT);
  }
  else if (massConcentrationPm1p0 <= 50)
  {
    lv_obj_set_style_bg_color(objects.pm1, lv_color_hex(COLOR_YELLOW), LV_PART_MAIN | LV_STATE_DEFAULT);
  }
  else if (massConcentrationPm1p0 <= 68)
  {
    lv_obj_set_style_bg_color(objects.pm1, lv_color_hex(COLOR_ORANGE), LV_PART_MAIN | LV_STATE_DEFAULT);
  }
  else
  {
    lv_obj_set_style_bg_color(objects.pm1, lv_color_hex(COLOR_RED), LV_PART_MAIN | LV_STATE_DEFAULT);
  }

  if (massConcentrationPm2p5 <= 13)
  {
    lv_obj_set_style_bg_color(objects.pm2p5, lv_color_hex(COLOR_GREEN), LV_PART_MAIN | LV_STATE_DEFAULT);
  }
  else if (massConcentrationPm2p5 <= 35)
  {
    lv_obj_set_style_bg_color(objects.pm2p5, lv_color_hex(COLOR_LIGHTGREEN), LV_PART_MAIN | LV_STATE_DEFAULT);
  }
  else if (massConcentrationPm2p5 <= 55)
  {
    lv_obj_set_style_bg_color(objects.pm2p5, lv_color_hex(COLOR_YELLOW), LV_PART_MAIN | LV_STATE_DEFAULT);
  }
  else if (massConcentrationPm2p5 <= 75)
  {
    lv_obj_set_style_bg_color(objects.pm2p5, lv_color_hex(COLOR_ORANGE), LV_PART_MAIN | LV_STATE_DEFAULT);
  }
  else
  {
    lv_obj_set_style_bg_color(objects.pm2p5, lv_color_hex(COLOR_RED), LV_PART_MAIN | LV_STATE_DEFAULT);
  }

  if (massConcentrationPm4p0 <= 14.4)
  {
    lv_obj_set_style_bg_color(objects.pm4, lv_color_hex(COLOR_GREEN), LV_PART_MAIN | LV_STATE_DEFAULT);
  }
  else if (massConcentrationPm4p0 <= 38)
  {
    lv_obj_set_style_bg_color(objects.pm4, lv_color_hex(COLOR_LIGHTGREEN), LV_PART_MAIN | LV_STATE_DEFAULT);
  }
  else if (massConcentrationPm4p0 <= 60)
  {
    lv_obj_set_style_bg_color(objects.pm4, lv_color_hex(COLOR_YELLOW), LV_PART_MAIN | LV_STATE_DEFAULT);
  }
  else if (massConcentrationPm4p0 <= 82)
  {
    lv_obj_set_style_bg_color(objects.pm4, lv_color_hex(COLOR_ORANGE), LV_PART_MAIN | LV_STATE_DEFAULT);
  }
  else
  {
    lv_obj_set_style_bg_color(objects.pm4, lv_color_hex(COLOR_RED), LV_PART_MAIN | LV_STATE_DEFAULT);
  }

  if (massConcentrationPm10p0 <= 20)
  {
    lv_obj_set_style_bg_color(objects.pm10, lv_color_hex(COLOR_GREEN), LV_PART_MAIN | LV_STATE_DEFAULT);
  }
  else if (massConcentrationPm10p0 <= 50)
  {
    lv_obj_set_style_bg_color(objects.pm10, lv_color_hex(COLOR_LIGHTGREEN), LV_PART_MAIN | LV_STATE_DEFAULT);
  }
  else if (massConcentrationPm10p0 <= 80)
  {
    lv_obj_set_style_bg_color(objects.pm10, lv_color_hex(COLOR_YELLOW), LV_PART_MAIN | LV_STATE_DEFAULT);
  }
  else if (massConcentrationPm10p0 <= 110)
  {
    lv_obj_set_style_bg_color(objects.pm10, lv_color_hex(COLOR_ORANGE), LV_PART_MAIN | LV_STATE_DEFAULT);
  }
  else
  {
    lv_obj_set_style_bg_color(objects.pm10, lv_color_hex(COLOR_RED), LV_PART_MAIN | LV_STATE_DEFAULT);
  }

  if (vocIndex <= 50)
  {
    lv_obj_set_style_bg_color(objects.voc, lv_color_hex(COLOR_GREEN), LV_PART_MAIN | LV_STATE_DEFAULT);
  }
  else if (vocIndex <= 150)
  {
    lv_obj_set_style_bg_color(objects.voc, lv_color_hex(COLOR_LIGHTGREEN), LV_PART_MAIN | LV_STATE_DEFAULT);
  }
  else if (vocIndex <= 250)
  {
    lv_obj_set_style_bg_color(objects.voc, lv_color_hex(COLOR_YELLOW), LV_PART_MAIN | LV_STATE_DEFAULT);
  }
  else if (vocIndex <= 400)
  {
    lv_obj_set_style_bg_color(objects.voc, lv_color_hex(COLOR_ORANGE), LV_PART_MAIN | LV_STATE_DEFAULT);
  }
  else
  {
    lv_obj_set_style_bg_color(objects.voc, lv_color_hex(COLOR_RED), LV_PART_MAIN | LV_STATE_DEFAULT);
  }

  if (noxIndex <= 1)
  {
    lv_obj_set_style_bg_color(objects.nox, lv_color_hex(COLOR_GREEN), LV_PART_MAIN | LV_STATE_DEFAULT);
  }
  else if (noxIndex <= 20)
  {
    lv_obj_set_style_bg_color(objects.nox, lv_color_hex(COLOR_LIGHTGREEN), LV_PART_MAIN | LV_STATE_DEFAULT);
  }
  else if (noxIndex <= 150)
  {
    lv_obj_set_style_bg_color(objects.nox, lv_color_hex(COLOR_YELLOW), LV_PART_MAIN | LV_STATE_DEFAULT);
  }
  else if (noxIndex <= 300)
  {
    lv_obj_set_style_bg_color(objects.nox, lv_color_hex(COLOR_ORANGE), LV_PART_MAIN | LV_STATE_DEFAULT);
  }
  else
  {
    lv_obj_set_style_bg_color(objects.nox, lv_color_hex(COLOR_RED), LV_PART_MAIN | LV_STATE_DEFAULT);
  }
}

void disp_weather(current_weather_data_t *current_weather, hourly_weather_data_t *hourly_weather, daily_weather_data_t *daily_weather) {

  // Current data
  char temp[8];
  char clouds[8];
  char uv_index[8];
  char wind_speed[8];
  char wind_gust[8];
  char str_sunrise[8];
  char str_sunset[8];

  icon_mapping_t *icon_mapping;
  if (current_weather->is_day) {
    icon_mapping = icon_mapping_day;
  }
  else {
    icon_mapping = icon_mapping_night;
  }

  for (uint8_t i = 0; i < NUM_ICONS; i++)
  {
    if (current_weather->weather_code == icon_mapping[i].icon)
    {
      ESP_LOGI(TAG, "Weather code %d", current_weather->weather_code);
      lv_img_set_src(objects.weather_icon, icon_mapping[i].icon_image);
      break;
    }
  }

  sprintf(temp, "%.1f", current_weather->temperature_2m);
  lv_label_set_text(objects.temp_current, temp);

  sprintf(clouds, "%d", current_weather->cloud_cover);
  lv_label_set_text(objects.clouds_current, clouds);

  sprintf(uv_index, "%d", (int) round(current_weather->uv_index));
  lv_label_set_text(objects.uv_current, uv_index);

  sprintf(wind_speed, "%.1f", current_weather->wind_speed_10m);
  lv_label_set_text(objects.wind_speed_current, wind_speed);

  sprintf(wind_gust, "%.1f", current_weather->wind_gusts_10m);
  lv_label_set_text(objects.wind_gust_current, wind_gust);

  lv_img_set_angle(objects.wind_direction_current_icon, current_weather->wind_direction_10m * 10);

  struct tm time_sunrise = daily_weather[0].sunrise;
  struct tm time_sunrset = daily_weather[0].sunset;
  strftime(str_sunrise, sizeof(str_sunrise), "%H:%M", &time_sunrise);
  lv_label_set_text(objects.sunrise_current, str_sunrise);
  strftime(str_sunset, sizeof(str_sunset), "%H:%M", &time_sunrset);
  lv_label_set_text(objects.sunset_current, str_sunset);

  // Hourly data
  lv_hourly_data hourly_data[NUM_HOURS];

  for (int i = 0; i < NUM_HOURS; i++)
  {
    hourly_data[i].dt = hourly_weather[i].time;
    hourly_data[i].temp = hourly_weather[i].temperature_2m;
    hourly_data[i].dew = hourly_weather[i].dew_point_2m;
    hourly_data[i].rain = hourly_weather[i].rain + hourly_weather[i].showers;
    hourly_data[i].snow = hourly_weather[i].snowfall * 10.0f / 7.0f;  // See docu from open-meteo.com  snow -> water
    hourly_data[i].pop = hourly_weather[i].precipitation_probability;
    //hourly_data[i].pop = (hourly_weather[i].precipitation_probability * 80.0f / 100.0f + 20.0f) / 100.0f;  // Map 0-100 to 25-100 for better visualisation
    hourly_data[i].sun = hourly_weather[i].sunshine_duration / 3600.0f;
    //hourly_data[i].sun = hourly_weather[i].is_day ? (100.0f - source_data[i].cloud_cover) / 100.0f : 0;
  }

  lv_hourly_chart_set_data(objects.hourly_chart, hourly_data);
  lv_hourly_chart_refresh(objects.hourly_chart);

  // Daily data
  lv_daily_data daily_data[NUM_DAYS];

  for (int i = 0; i < NUM_DAYS; i++)
  {
    daily_data[i].dt = daily_weather[i].time;
    daily_data[i].low_temp = daily_weather[i].temperature_2m_min;
    daily_data[i].high_temp = daily_weather[i].temperature_2m_max;
    daily_data[i].rain = daily_weather[i].rain_sum + daily_weather[i].showers_sum;
    daily_data[i].snow = daily_weather[i].snowfall_sum * 10.0f / 7.0f;  // See docu from open-meteo.com  snow -> water
    daily_data[i].pop = daily_weather[i].precipitation_probability_max;
    daily_data[i].sun = daily_weather[i].sunshine_duration / daily_weather[i].daylight_duration;
  }

  lv_daily_chart_set_data(objects.daily_chart, daily_data);
  lv_daily_chart_refresh(objects.daily_chart);
}


void set_brightness(uint16_t brightness)
{
  set_backlight_brightness(brightness);
}

// -------- Setup Screen --------

void disp_wifi_networks(char* allNetworks)
{
  lv_dropdown_clear_options(objects.dropdown_networks);
  lv_dropdown_set_options(objects.dropdown_networks, allNetworks);
}

void disp_disable_scanbutton(bool is_disabled)
{
  if (is_disabled)
  {
    lv_obj_add_state( objects.button_scan, LV_STATE_DISABLED ); 
  }
  else
  {
    lv_obj_clear_state( objects.button_scan, LV_STATE_DISABLED ); 
  }
}

void disp_disable_connectbutton(bool is_disabled)
{
  if (is_disabled)
  {
    lv_obj_add_state( objects.button_connect, LV_STATE_DISABLED ); 
  }
  else
  {
    lv_obj_clear_state( objects.button_connect, LV_STATE_DISABLED ); 
  }
}

void disp_connect_status(bool is_connected)
{
  if (is_connected)
  {
    lv_obj_set_style_bg_color(objects.text_area_password, lv_color_hex(COLOR_LIGHTGREEN), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(objects.text_area_password, 128, LV_PART_MAIN | LV_STATE_DEFAULT);
  }
  else
  {
    lv_obj_set_style_bg_color(objects.text_area_password, lv_color_hex(COLOR_ORANGE), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(objects.text_area_password, 128, LV_PART_MAIN | LV_STATE_DEFAULT);
  }
}

void set_cities(const char *region)
{
  lv_dropdown_clear_options(objects.dropdown_city);
  for (size_t i = 0; i < sizeof(cityData) / sizeof(cityData[0]); i++)
  {
    if (strcmp(cityData[i][0], region) == 0)
    {
      // Found a matching region, split city names and add them to the dropdown
      const char *cities = cityData[i][1];
      lv_dropdown_add_option(objects.dropdown_city, cities, LV_DROPDOWN_POS_LAST);
    }
  }
}

void set_labels() {

  nvs_handle_t nvs_handle;
  nvs_open("weatherstation", NVS_READONLY, &nvs_handle);

  char*  name_base = get_string_from_nvs(nvs_handle, "name_base", "");
  char*  name_sensor_1 = get_string_from_nvs(nvs_handle, "name_sensor_1", "");
  char*  name_sensor_2 =get_string_from_nvs(nvs_handle, "name_sensor_2", "");
  char*  name_sensor_3 = get_string_from_nvs(nvs_handle, "name_sensor_3", "");

  lv_label_set_text(objects.name_base, name_base);
  lv_label_set_text(objects.name0, name_sensor_1);
  lv_label_set_text(objects.name1, name_sensor_2);
  lv_label_set_text(objects.name2, name_sensor_3);

  nvs_close(nvs_handle);
}

void start_tasks()
{
 
  xTaskCreatePinnedToCore(
      clock_task,  
      "Clock Task",
      4096,  
      NULL,   
      1,     
      NULL, 
      1);  

  xTaskCreatePinnedToCore(
      weather_task,
      "Weather Task",
      16384,
      NULL,
      1,
      NULL,
      1);      

  xTaskCreatePinnedToCore(
      sensor_sen55_task,    
      "Sensor SEN55 Task",  
      4096,          
      NULL,           
      1,       
      NULL,       
      1);

  xTaskCreatePinnedToCore(
      sensor_scd41_task,    
      "Sensor SCD41 Task",  
      4096,          
      NULL,           
      1,       
      NULL,       
      1);

  xTaskCreatePinnedToCore(
      brightness_task,
      "Brightness Task",
      4096,
      NULL,
      1,
      NULL,
      1);


}

// -------- LVGL Events --------

void action_event_setup_screen_loaded(lv_event_t *e)
{

  nvs_handle_t nvs_handle;
  nvs_open("weatherstation", NVS_READONLY, &nvs_handle);

  char* ssid = get_string_from_nvs(nvs_handle, "ssid", "");
  char* password = get_string_from_nvs(nvs_handle, "password", "");
  char* appid = get_string_from_nvs(nvs_handle, "appid", "");
  char* latitude = get_string_from_nvs(nvs_handle, "latitude", "");
  char* longitude =get_string_from_nvs(nvs_handle, "longitude", "");
  char* height = get_string_from_nvs(nvs_handle, "height", "");
  uint8_t region_id = get_uint8_from_nvs(nvs_handle, "region", 0);
  uint8_t city_id = get_uint8_from_nvs(nvs_handle, "city", 0);
  char* name_base = get_string_from_nvs(nvs_handle, "name_base", "");
  char* name_sensor_1 = get_string_from_nvs(nvs_handle, "name_sensor_1", "");
  char* name_sensor_2 =get_string_from_nvs(nvs_handle, "name_sensor_2", "");
  char* name_sensor_3 = get_string_from_nvs(nvs_handle, "name_sensor_3", "");

  nvs_close(nvs_handle);

  if (strcmp(ssid, "") != 0)
  {
    lv_dropdown_clear_options(objects.dropdown_networks);
    lv_dropdown_add_option(objects.dropdown_networks, ssid, LV_DROPDOWN_POS_LAST);
  }
  lv_textarea_set_text(objects.text_area_password, password);
  lv_textarea_set_text(objects.text_area_app_id, appid);

  lv_textarea_set_text(objects.text_area_latitude, latitude);
  lv_textarea_set_text(objects.text_area_longitude, longitude);
  lv_textarea_set_text(objects.text_area_hoehe, height);

  // fill the region names
  for (size_t i = 0; i < sizeof(regionNames) / sizeof(regionNames[0]); i++)
  {
    lv_dropdown_add_option(objects.dropdown_region, regionNames[i], LV_DROPDOWN_POS_LAST);
  }
  lv_dropdown_set_selected(objects.dropdown_region, region_id); // set the selected region id
  // get the region name
  char region[64];
  lv_dropdown_get_selected_str(objects.dropdown_region, region, sizeof(region));
  // fill the city list
  set_cities(region);
  // set the selected city id
  lv_dropdown_set_selected(objects.dropdown_city, city_id);

  lv_textarea_set_text(objects.text_area_basis, name_base);
  lv_textarea_set_text(objects.text_area_sensor_name1, name_sensor_1);
  lv_textarea_set_text(objects.text_area_sensor_name2, name_sensor_2);
  lv_textarea_set_text(objects.text_area_sensor_name3, name_sensor_3);
}

void action_event_wifi_scan(lv_event_t *e)
{
    xTaskCreatePinnedToCore(
        wifiscan_task,   // Task function
        "WiFiScan Task", // Task name
        16000,            // Stack size (bytes)
        NULL,            // Task input parameter
        16,              // Task priority
        NULL,            // Task handle
        0                // Core to run the task on (0 or 1)
    );
 
}

void action_event_wifi_connect(lv_event_t *e)
{
  char network[64];
  lv_dropdown_get_selected_str(objects.dropdown_networks, network, sizeof(network));
  const char *password = lv_textarea_get_text(objects.text_area_password);

  // Allocate memory for ssid and password in the struct
  local_wifi_sta_config_t *wifiParams = (local_wifi_sta_config_t *)malloc(sizeof(local_wifi_sta_config_t));
  wifiParams->ssid = strdup(network);
  wifiParams->password = strdup(password);

  xTaskCreatePinnedToCore(
      wificonnect_task,   // Task function
      "WiFiConnect Task", // Task name
      4096,               // Stack size (bytes)
      wifiParams,         // Task input parameter
      16,                 // Task priority
      NULL,               // Task handle
      0                   // Core to run the task on (0 or 1)
  );

}

void event_timezone_value_changed(lv_event_t *e)
{
  int selectedRegion = lv_dropdown_get_selected(objects.dropdown_region);
  ESP_LOGI(TAG,"selectedRegion: %d", selectedRegion);

  char region[64];
  lv_dropdown_get_selected_str(objects.dropdown_region, region, sizeof(region));
  ESP_LOGI(TAG,"region: %s", region);

  set_cities(region);

  lv_dropdown_set_selected(objects.dropdown_city, 0);
}


void action_event_weatherstation_start(lv_event_t *e)
{
  // Store preferences
  nvs_handle_t nvs_handle;
  nvs_open("weatherstation", NVS_READWRITE, &nvs_handle);

  char ssid[64];
  lv_dropdown_get_selected_str(objects.dropdown_networks, ssid, sizeof(ssid));
  put_string_to_nvs(nvs_handle, "ssid", ssid);

  const char* password = lv_textarea_get_text(objects.text_area_password);
  put_string_to_nvs(nvs_handle, "password", password);

  const char* appid = lv_textarea_get_text(objects.text_area_app_id);
  put_string_to_nvs(nvs_handle, "appid", appid);

  const char* latitude = lv_textarea_get_text(objects.text_area_latitude);
  put_string_to_nvs(nvs_handle, "latitude", latitude);

  const char* longitude = lv_textarea_get_text(objects.text_area_longitude);
  put_string_to_nvs(nvs_handle, "longitude", longitude);

  const char* height = lv_textarea_get_text(objects.text_area_hoehe);
  put_string_to_nvs(nvs_handle, "height", height);

  uint8_t region_id = lv_dropdown_get_selected(objects.dropdown_region);
  put_uint8_to_nvs(nvs_handle, "region", region_id);

  uint8_t city_id = lv_dropdown_get_selected(objects.dropdown_city);
  put_uint8_to_nvs(nvs_handle, "city", city_id);

  const char* tz = NULL;
  const char *region = regionNames[region_id];
  for (size_t i = 0; i < sizeof(cityData) / sizeof(cityData[0]); i++)
  {
    if (strcmp(cityData[i][0], region) == 0)
    {
      tz = cityData[i + city_id][2];
      break;
    }
  }
  put_string_to_nvs(nvs_handle, "tz", tz);

  const char* name_base = lv_textarea_get_text(objects.text_area_basis);
  put_string_to_nvs(nvs_handle, "name_base", name_base);

  const char* name_sensor_1 = lv_textarea_get_text(objects.text_area_sensor_name1);
  put_string_to_nvs(nvs_handle, "name_sensor_1", name_sensor_1);

  const char* name_sensor_2 = lv_textarea_get_text(objects.text_area_sensor_name2);
  put_string_to_nvs(nvs_handle, "name_sensor_2", name_sensor_2);

  const char* name_sensor_3 = lv_textarea_get_text(objects.text_area_sensor_name3);
  put_string_to_nvs(nvs_handle, "name_sensor_3", name_sensor_3);

  nvs_close(nvs_handle);

  
  set_labels();
  wifi_start();
  esp_now_start();
  start_tasks();

  // Start Weatherstation
  loadScreen(SCREEN_ID_WEATHERSTATION_SCREEN);


}

void action_event_text_area_password(lv_event_t * e)
{
    lv_event_code_t event_code = lv_event_get_code(e);

    if(event_code == LV_EVENT_FOCUSED) {

      lv_obj_clear_flag(objects.keyboard_text, LV_OBJ_FLAG_HIDDEN);
      lv_keyboard_set_textarea(objects.keyboard_text, objects.text_area_password);

      lv_obj_set_x(objects.keyboard_text, -38);
      lv_obj_set_y(objects.keyboard_text, -155);
    }
    if(event_code == LV_EVENT_DEFOCUSED) {
      lv_obj_add_flag(objects.keyboard_text, LV_OBJ_FLAG_HIDDEN);
    }
}

void action_event_text_area_app_id(lv_event_t * e)
{
    lv_event_code_t event_code = lv_event_get_code(e);

    if(event_code == LV_EVENT_FOCUSED) {

      lv_obj_clear_flag(objects.keyboard_text, LV_OBJ_FLAG_HIDDEN);
      lv_keyboard_set_textarea(objects.keyboard_text, objects.text_area_app_id);

      lv_obj_set_x(objects.keyboard_text, -38);
      lv_obj_set_y(objects.keyboard_text, -87);
    }
    if(event_code == LV_EVENT_DEFOCUSED) {
      lv_obj_add_flag(objects.keyboard_text, LV_OBJ_FLAG_HIDDEN);
    }
}

void action_event_text_area_latitude(lv_event_t * e)
{
    lv_event_code_t event_code = lv_event_get_code(e);

    if(event_code == LV_EVENT_FOCUSED) {

      lv_obj_clear_flag(objects.keyboard_numeric, LV_OBJ_FLAG_HIDDEN);
      lv_keyboard_set_textarea(objects.keyboard_numeric, objects.text_area_latitude);

      lv_obj_set_x(objects.keyboard_numeric, 99);
      lv_obj_set_y(objects.keyboard_numeric, 277);
    }
    if(event_code == LV_EVENT_DEFOCUSED) {
      lv_obj_add_flag(objects.keyboard_numeric, LV_OBJ_FLAG_HIDDEN);
    }
}

void action_event_text_area_longitude(lv_event_t * e)
{
    lv_event_code_t event_code = lv_event_get_code(e);

    if(event_code == LV_EVENT_FOCUSED) {

      lv_obj_clear_flag(objects.keyboard_numeric, LV_OBJ_FLAG_HIDDEN);
      lv_keyboard_set_textarea(objects.keyboard_numeric, objects.text_area_longitude);

      lv_obj_set_x(objects.keyboard_numeric, 478);
      lv_obj_set_y(objects.keyboard_numeric, 277);
    }
    if(event_code == LV_EVENT_DEFOCUSED) {
      lv_obj_add_flag(objects.keyboard_numeric, LV_OBJ_FLAG_HIDDEN);
    }
}

void action_event_text_area_hoehe(lv_event_t * e)
{
    lv_event_code_t event_code = lv_event_get_code(e);

    if(event_code == LV_EVENT_FOCUSED) {

      lv_obj_clear_flag(objects.keyboard_numeric, LV_OBJ_FLAG_HIDDEN);
      lv_keyboard_set_textarea(objects.keyboard_numeric, objects.text_area_hoehe);

      lv_obj_set_x(objects.keyboard_numeric, 691);
      lv_obj_set_y(objects.keyboard_numeric, 277);
    }
    if(event_code == LV_EVENT_DEFOCUSED) {
      lv_obj_add_flag(objects.keyboard_numeric, LV_OBJ_FLAG_HIDDEN);
    }
}

void action_event_text_area_basis(lv_event_t * e)
{
    lv_event_code_t event_code = lv_event_get_code(e);

    if(event_code == LV_EVENT_FOCUSED) {

      lv_obj_clear_flag(objects.keyboard_text, LV_OBJ_FLAG_HIDDEN);
      lv_keyboard_set_textarea(objects.keyboard_text, objects.text_area_basis);

      lv_obj_set_x(objects.keyboard_text, 9);
      lv_obj_set_y(objects.keyboard_text, -234);
    }
    if(event_code == LV_EVENT_DEFOCUSED) {
      lv_obj_add_flag(objects.keyboard_text, LV_OBJ_FLAG_HIDDEN);
    }
}

void action_event_text_area_sensor_name1(lv_event_t * e)
{
    lv_event_code_t event_code = lv_event_get_code(e);

    if(event_code == LV_EVENT_FOCUSED) {

      lv_obj_clear_flag(objects.keyboard_text, LV_OBJ_FLAG_HIDDEN);
      lv_keyboard_set_textarea(objects.keyboard_text, objects.text_area_sensor_name1);

      lv_obj_set_x(objects.keyboard_text, 60);
      lv_obj_set_y(objects.keyboard_text, -166);
    }
    if(event_code == LV_EVENT_DEFOCUSED) {
      lv_obj_add_flag(objects.keyboard_text, LV_OBJ_FLAG_HIDDEN);
    }
}

void action_event_text_area_sensor_name2(lv_event_t * e)
{
    lv_event_code_t event_code = lv_event_get_code(e);

    if(event_code == LV_EVENT_FOCUSED) {

      lv_obj_clear_flag(objects.keyboard_text, LV_OBJ_FLAG_HIDDEN);
      lv_keyboard_set_textarea(objects.keyboard_text, objects.text_area_sensor_name2);

      lv_obj_set_x(objects.keyboard_text, 60);
      lv_obj_set_y(objects.keyboard_text, -166);
    }
    if(event_code == LV_EVENT_DEFOCUSED) {
      lv_obj_add_flag(objects.keyboard_text, LV_OBJ_FLAG_HIDDEN);
    }
}

void action_event_text_area_sensor_name3(lv_event_t * e)
{
    lv_event_code_t event_code = lv_event_get_code(e);

    if(event_code == LV_EVENT_FOCUSED) {

      lv_obj_clear_flag(objects.keyboard_text, LV_OBJ_FLAG_HIDDEN);
      lv_keyboard_set_textarea(objects.keyboard_text, objects.text_area_sensor_name3);

      lv_obj_set_x(objects.keyboard_text, 60);
      lv_obj_set_y(objects.keyboard_text, -166);
    }
    if(event_code == LV_EVENT_DEFOCUSED) {
      lv_obj_add_flag(objects.keyboard_text, LV_OBJ_FLAG_HIDDEN);
    }
}

void action_event_keyboard_text(lv_event_t * e)
{
    lv_event_code_t event_code = lv_event_get_code(e);

    if(event_code == LV_EVENT_CANCEL) {
      lv_obj_add_flag(objects.keyboard_text, LV_OBJ_FLAG_HIDDEN);
    }
    if(event_code == LV_EVENT_READY) {
      lv_obj_add_flag(objects.keyboard_text, LV_OBJ_FLAG_HIDDEN);
    }
}

void action_event_keyboard_numeric(lv_event_t * e)
{
    lv_event_code_t event_code = lv_event_get_code(e);

    if(event_code == LV_EVENT_CANCEL) {
      lv_obj_add_flag(objects.keyboard_numeric, LV_OBJ_FLAG_HIDDEN);
    }
    if(event_code == LV_EVENT_READY) {
      lv_obj_add_flag(objects.keyboard_numeric, LV_OBJ_FLAG_HIDDEN);
    }
}