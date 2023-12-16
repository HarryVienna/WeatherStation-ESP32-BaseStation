#include <Arduino.h>
#include <Preferences.h>

#include "SPI.h"
#include "SD.h"
#include "FS.h"
#include "WiFi.h"

#include <lv_conf.h>
#include <lvgl.h>
#include "../ui/ui.h"
#include "../gfx/LGFX_ESP32S3_RGB_MakerfabsParallelTFTwithTouch70.h"
#include "../lvgl/lv_hourly_chart.h"
#include "../lvgl/lv_daily_chart.h"
#include "../config/config.h"

#include "gui.h"

#include "wifi/network.h"
#include "task/clock_task.h"
#include "task/weather_task.h"
#include "task/sensor_task.h"
#include "task/brightness_task.h"
#include "task/wifiscan_task.h"
#include "task/wificonnect_task.h"

extern SemaphoreHandle_t mutex;

#define NUM_ICONS 18

typedef struct
{
  const char *icon;
  const lv_img_dsc_t *icon_image;
} icon_mapping_t;

icon_mapping_t icon_mapping[] = {
    {"01d", &ui_img_01d_png},
    {"01n", &ui_img_01n_png},
    {"02d", &ui_img_02d_png},
    {"02n", &ui_img_02n_png},
    {"03d", &ui_img_03d_png},
    {"03n", &ui_img_03n_png},
    {"04d", &ui_img_04d_png},
    {"04n", &ui_img_04n_png},
    {"09d", &ui_img_09d_png},
    {"09n", &ui_img_09n_png},
    {"10d", &ui_img_10d_png},
    {"10n", &ui_img_10n_png},
    {"11d", &ui_img_11d_png},
    {"11n", &ui_img_11n_png},
    {"13d", &ui_img_13d_png},
    {"13n", &ui_img_13n_png},
    {"50d", &ui_img_50d_png},
    {"50n", &ui_img_50n_png}};

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

static const char *TAG = "gui";

static const uint16_t screenWidth = 800;
static const uint16_t screenHeight = 480;

static lv_disp_draw_buf_t draw_buf;
static lv_color_t buf[2][screenWidth * 10];

LGFX gfx;

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

/* Display flushing */
void disp_flush(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p)
{
  if (gfx.getStartCount() == 0)
  { // Processing if not yet started
    gfx.startWrite();
  }
  gfx.pushImageDMA(area->x1, area->y1, area->x2 - area->x1 + 1, area->y2 - area->y1 + 1, (lgfx::rgb565_t *)&color_p->full);
  lv_disp_flush_ready(disp);
}

/*Read the touchpad*/
void touchpad_read(lv_indev_drv_t *indev_driver, lv_indev_data_t *data)
{
  uint16_t touchX, touchY;

  data->state = LV_INDEV_STATE_REL;

  if (gfx.getTouch(&touchX, &touchY))
  {
    data->state = LV_INDEV_STATE_PR;

    /*Set the coordinates*/
    data->point.x = touchX;
    data->point.y = touchY;
  }
}

/**
 * @brief     Capture GUI object screenshot and save as BMP on SD card
 *
 * @details   Captures a screenshot of the specified GUI object, converts it to a BMP format, 
 *            and saves it as a file onto an SD card.
 *            The function uses LVGL's snapshot feature to obtain the image data, which is then
 *            processed to create a BMP file and save it on the SD card.
 *            Assumes LVGL library is used for GUI rendering and an SD card is properly initialized.
 */
void gui_screenshot()
{

  uint8_t SD_SCK = 12;
  uint8_t SD_MISO = 13;
  uint8_t SD_MOSI = 11;
  uint8_t SD_CS = 10;

  constexpr uint16_t LCD_W = 800;
  constexpr uint16_t LCD_H = 480;

  constexpr uint16_t BMP_HEADERSIZE = 0x76;
  constexpr uint8_t BMP_BITS_PER_PIXEL = 32;
  constexpr uint32_t BMP_FILESIZE = uint32_t(BMP_HEADERSIZE + (LCD_W * LCD_H) * BMP_BITS_PER_PIXEL / 8);

  const uint8_t BMP_HEADER[] = {
      'B', 'M',
      /* file size */ BMP_FILESIZE & 0xFF, (BMP_FILESIZE >> 8) & 0xFF, (BMP_FILESIZE >> 16) & 0xFF, (BMP_FILESIZE >> 24) & 0xFF,
      /* Reserved */ 0x00, 0x00, 0x00, 0x00,
      /* header size */ BMP_HEADERSIZE, 0x00, 0x00, 0x00,
      /* extra header size */ 0x28, 0x00, 0x00, 0x00,
      /* width */ LCD_W & 0xFF, LCD_W >> 8, 0x00, 0x00,
      /* height */ LCD_H & 0xFF, LCD_H >> 8, 0x00, 0x00,
      /* planes */ 0x01, 0x00,
      /* depth */ BMP_BITS_PER_PIXEL, 0x00,
      0x00, 0x00,
      0x00, 0x00, 0x02, 0x04, 0x00, 0x00, 0xbc, 0x38, 0x00, 0x00, 0xbc, 0x38, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0x00, 0xee, 0xee, 0xee, 0x00, 0xdd, 0xdd,
      0xdd, 0x00, 0xcc, 0xcc, 0xcc, 0x00, 0xbb, 0xbb, 0xbb, 0x00, 0xaa, 0xaa, 0xaa, 0x00, 0x99, 0x99,
      0x99, 0x00, 0x88, 0x88, 0x88, 0x00, 0x77, 0x77, 0x77, 0x00, 0x66, 0x66, 0x66, 0x00, 0x55, 0x55,
      0x55, 0x00, 0x44, 0x44, 0x44, 0x00, 0x33, 0x33, 0x33, 0x00, 0x22, 0x22, 0x22, 0x00, 0x11, 0x11,
      0x11, 0x00, 0x00, 0x00, 0x00, 0x00};

  lv_obj_t *obj = lv_scr_act();
  lv_img_cf_t cf = LV_IMG_CF_TRUE_COLOR;

  LV_ASSERT_NULL(obj);
  uint32_t buff_size = lv_snapshot_buf_size_needed(obj, cf);

  void *buf = heap_caps_malloc(buff_size, MALLOC_CAP_32BIT | MALLOC_CAP_SPIRAM);

  lv_img_dsc_t *snapshot = (lv_img_dsc_t *)heap_caps_malloc(sizeof(lv_img_dsc_t), MALLOC_CAP_32BIT | MALLOC_CAP_SPIRAM);

  lv_coord_t w = lv_obj_get_width(obj);
  lv_coord_t h = lv_obj_get_height(obj);

  lv_coord_t ext_size = _lv_obj_get_ext_draw_size(obj);
  w += ext_size * 2;
  h += ext_size * 2;

  lv_area_t snapshot_area;
  lv_obj_get_coords(obj, &snapshot_area);
  lv_area_increase(&snapshot_area, ext_size, ext_size);

  lv_memset(buf, 0x00, buff_size);
  lv_memset_00(snapshot, sizeof(lv_img_dsc_t));

  lv_disp_t *obj_disp = lv_obj_get_disp(obj);
  lv_disp_drv_t driver;

  lv_disp_drv_init(&driver);

  /*In lack of a better idea use the resolution of the object's display*/
  driver.hor_res = lv_disp_get_hor_res(obj_disp);
  driver.ver_res = lv_disp_get_hor_res(obj_disp);
  lv_disp_drv_use_generic_set_px_cb(&driver, cf);

  lv_disp_t fake_disp;
  lv_memset_00(&fake_disp, sizeof(lv_disp_t));
  fake_disp.driver = &driver;

  lv_draw_ctx_t *draw_ctx = (lv_draw_ctx_t *)heap_caps_malloc(obj_disp->driver->draw_ctx_size, MALLOC_CAP_32BIT | MALLOC_CAP_SPIRAM);

  obj_disp->driver->draw_ctx_init(fake_disp.driver, draw_ctx);
  fake_disp.driver->draw_ctx = draw_ctx;
  draw_ctx->clip_area = &snapshot_area;
  draw_ctx->buf_area = &snapshot_area;
  draw_ctx->buf = (void *)buf;
  driver.draw_ctx = draw_ctx;

  lv_disp_t *refr_ori = _lv_refr_get_disp_refreshing();
  _lv_refr_set_disp_refreshing(&fake_disp);

  lv_obj_redraw(draw_ctx, obj);

  _lv_refr_set_disp_refreshing(refr_ori);
  obj_disp->driver->draw_ctx_deinit(fake_disp.driver, draw_ctx);
  lv_mem_free(draw_ctx);

  snapshot->data = (uint8_t *)buf;
  snapshot->header.w = w;
  snapshot->header.h = h;
  snapshot->header.cf = cf;

  // SD(SPI)
  pinMode(SD_CS, OUTPUT);
  digitalWrite(SD_CS, HIGH);
  SPI.begin(SD_SCK, SD_MISO, SD_MOSI);
  // SPI.setFrequency(100000);
  if (!SD.begin(SD_CS, SPI, 4000000))
  {
    Serial.println("Card Mount Failed");
    return;
  }
  else
  {
    Serial.println("SD OK");
  }

  char filename[42] = "/s2.bmp";

  File bmpFile = SD.open(filename, "w", true);

  bmpFile.write(BMP_HEADER, sizeof(BMP_HEADER));
  Serial.println("header written");

  for (int y = h - 1; y >= 0; y--)
  {
    for (int x = 0; x < w; x++)
    {

      lv_color_t pixel = lv_img_buf_get_px_color(snapshot, x, y, {});

      uint32_t dst = (0xFF << 24) | (pixel.ch.red << 19) | (pixel.ch.green << 10) | (pixel.ch.blue << 3);

      bmpFile.write((uint8_t *)&dst, sizeof(uint32_t));
      Serial.print(".");
    }
  }

  // lv_snapshot_free(snapshot);
  bmpFile.flush();
  bmpFile.close();
  SD.end();
}

/**
 * @brief     Initialize the graphical user interface (GUI)
 *
 * @details   Initializes the necessary components for the GUI, including the display,
 *            display driver, input device driver (e.g., touchpad), and other UI-related configurations.
 *            Assumes the use of LVGL library for graphical rendering and a specific input device (e.g., touchpad).
 */
void gui_start()
{

  // ----------- GFX -------------
  gfx.begin();
  //  gfx.setColorDepth(16);
  gfx.setBrightness(127);

  lv_init();
  lv_disp_draw_buf_init(&draw_buf, buf[0], buf[1], screenWidth * 10);

  /*Initialize the display*/
  static lv_disp_drv_t disp_drv;
  lv_disp_drv_init(&disp_drv);
  disp_drv.hor_res = screenWidth;
  disp_drv.ver_res = screenHeight;
  disp_drv.flush_cb = disp_flush;
  disp_drv.draw_buf = &draw_buf;
  lv_disp_drv_register(&disp_drv);

  /*Initialize the input device driver*/
  static lv_indev_drv_t indev_drv;
  lv_indev_drv_init(&indev_drv);
  indev_drv.type = LV_INDEV_TYPE_POINTER;
  indev_drv.read_cb = touchpad_read;
  lv_indev_drv_register(&indev_drv);

  ui_init();
}

// -------- Weatherstation Screen --------

void disp_date_time(char *date_time)
{
  lv_label_set_text(ui_DateTime, date_time);
}

void disp_sensor_data(uint8_t sensor_nr, double temperature, double humidity, double pressure, uint32_t voltage, char *date_time)
{

  Preferences preferences;
  preferences.begin("Weatherstation", false);

  uint16_t height = preferences.getString("height", "").toInt();

  preferences.end();

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
    lv_label_set_text(ui_Temp0, temperature_value);
    lv_label_set_text(ui_Hunidity0, humidity_value);
    lv_label_set_text(ui_Pressure0, pressure_value);
    lv_label_set_text(ui_Volt0, voltage_value);
    lv_label_set_text(ui_Update0, date_time);

    if (voltage_rounded >= 4.0)
    {
      lv_obj_set_style_bg_color(ui_LineStatus0, lv_color_hex(COLOR_GREEN), LV_PART_MAIN | LV_STATE_DEFAULT);
    }
    else if (voltage_rounded >= 3.8)
    {
      lv_obj_set_style_bg_color(ui_LineStatus0, lv_color_hex(COLOR_LIGHTGREEN), LV_PART_MAIN | LV_STATE_DEFAULT);
    }
    else if (voltage_rounded >= 3.6)
    {
      lv_obj_set_style_bg_color(ui_LineStatus0, lv_color_hex(COLOR_ORANGE), LV_PART_MAIN | LV_STATE_DEFAULT);
    }
    else
    {
      lv_obj_set_style_bg_color(ui_LineStatus0, lv_color_hex(COLOR_RED), LV_PART_MAIN | LV_STATE_DEFAULT);
    }
    break;
  case 1:
    lv_label_set_text(ui_Temp1, temperature_value);
    lv_label_set_text(ui_Hunidity1, humidity_value);
    lv_label_set_text(ui_Pressure1, pressure_value);
    lv_label_set_text(ui_Volt1, voltage_value);
    lv_label_set_text(ui_Update1, date_time);
    if (voltage_rounded >= 4.0)
    {
      lv_obj_set_style_bg_color(ui_LineStatus1, lv_color_hex(COLOR_GREEN), LV_PART_MAIN | LV_STATE_DEFAULT);
    }
    else if (voltage_rounded >= 3.8)
    {
      lv_obj_set_style_bg_color(ui_LineStatus1, lv_color_hex(COLOR_LIGHTGREEN), LV_PART_MAIN | LV_STATE_DEFAULT);
    }
    else if (voltage_rounded >= 3.6)
    {
      lv_obj_set_style_bg_color(ui_LineStatus1, lv_color_hex(COLOR_ORANGE), LV_PART_MAIN | LV_STATE_DEFAULT);
    }
    else
    {
      lv_obj_set_style_bg_color(ui_LineStatus1, lv_color_hex(COLOR_RED), LV_PART_MAIN | LV_STATE_DEFAULT);
    }
    break;
  case 2:
    lv_label_set_text(ui_Temp2, temperature_value);
    lv_label_set_text(ui_Hunidity2, humidity_value);
    lv_label_set_text(ui_Pressure2, pressure_value);
    lv_label_set_text(ui_Volt2, voltage_value);
    lv_label_set_text(ui_Update2, date_time);
    if (voltage_rounded >= 4.0)
    {
      lv_obj_set_style_bg_color(ui_LineStatus2, lv_color_hex(COLOR_GREEN), LV_PART_MAIN | LV_STATE_DEFAULT);
    }
    else if (voltage_rounded >= 3.8)
    {
      lv_obj_set_style_bg_color(ui_LineStatus2, lv_color_hex(COLOR_LIGHTGREEN), LV_PART_MAIN | LV_STATE_DEFAULT);
    }
    else if (voltage_rounded >= 3.6)
    {
      lv_obj_set_style_bg_color(ui_LineStatus2, lv_color_hex(COLOR_ORANGE), LV_PART_MAIN | LV_STATE_DEFAULT);
    }
    else
    {
      lv_obj_set_style_bg_color(ui_LineStatus2, lv_color_hex(COLOR_RED), LV_PART_MAIN | LV_STATE_DEFAULT);
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
    lv_obj_set_style_bg_color(ui_CO2, lv_color_hex(COLOR_GREEN), LV_PART_MAIN | LV_STATE_DEFAULT);
  }
  else if (co2 <= 1000)
  {
    lv_obj_set_style_bg_color(ui_CO2, lv_color_hex(COLOR_LIGHTGREEN), LV_PART_MAIN | LV_STATE_DEFAULT);
  }
  else if (co2 <= 1500)
  {
    lv_obj_set_style_bg_color(ui_CO2, lv_color_hex(COLOR_YELLOW), LV_PART_MAIN | LV_STATE_DEFAULT);
  }
  else if (co2 <= 1900)
  {
    lv_obj_set_style_bg_color(ui_CO2, lv_color_hex(COLOR_ORANGE), LV_PART_MAIN | LV_STATE_DEFAULT);
  }
  else
  {
    lv_obj_set_style_bg_color(ui_CO2, lv_color_hex(COLOR_RED), LV_PART_MAIN | LV_STATE_DEFAULT);
  }
}

void disp_sen5x(float ambientTemperature, float ambientHumidity, float massConcentrationPm1p0, float massConcentrationPm2p5, float massConcentrationPm4p0, float massConcentrationPm10p0, float vocIndex, float noxIndex)
{

  if (!isnan(ambientTemperature))
  {
    char temp[8];
    sprintf(temp, "%.1f", ambientTemperature);
    lv_label_set_text(ui_TempBase, temp);
  }

  if (!isnan(ambientHumidity))
  {
    char humidity[8];
    sprintf(humidity, "%.1f", ambientHumidity);
    lv_label_set_text(ui_HumidityBase, humidity);
  }

  if (massConcentrationPm1p0 <= 11.6)
  {
    lv_obj_set_style_bg_color(ui_PM1, lv_color_hex(COLOR_GREEN), LV_PART_MAIN | LV_STATE_DEFAULT);
  }
  else if (massConcentrationPm1p0 <= 32)
  {
    lv_obj_set_style_bg_color(ui_PM1, lv_color_hex(COLOR_LIGHTGREEN), LV_PART_MAIN | LV_STATE_DEFAULT);
  }
  else if (massConcentrationPm1p0 <= 50)
  {
    lv_obj_set_style_bg_color(ui_PM1, lv_color_hex(COLOR_YELLOW), LV_PART_MAIN | LV_STATE_DEFAULT);
  }
  else if (massConcentrationPm1p0 <= 68)
  {
    lv_obj_set_style_bg_color(ui_PM1, lv_color_hex(COLOR_ORANGE), LV_PART_MAIN | LV_STATE_DEFAULT);
  }
  else
  {
    lv_obj_set_style_bg_color(ui_PM1, lv_color_hex(COLOR_RED), LV_PART_MAIN | LV_STATE_DEFAULT);
  }

  if (massConcentrationPm2p5 <= 13)
  {
    lv_obj_set_style_bg_color(ui_PM2p5, lv_color_hex(COLOR_GREEN), LV_PART_MAIN | LV_STATE_DEFAULT);
  }
  else if (massConcentrationPm2p5 <= 35)
  {
    lv_obj_set_style_bg_color(ui_PM2p5, lv_color_hex(COLOR_LIGHTGREEN), LV_PART_MAIN | LV_STATE_DEFAULT);
  }
  else if (massConcentrationPm2p5 <= 55)
  {
    lv_obj_set_style_bg_color(ui_PM2p5, lv_color_hex(COLOR_YELLOW), LV_PART_MAIN | LV_STATE_DEFAULT);
  }
  else if (massConcentrationPm2p5 <= 75)
  {
    lv_obj_set_style_bg_color(ui_PM2p5, lv_color_hex(COLOR_ORANGE), LV_PART_MAIN | LV_STATE_DEFAULT);
  }
  else
  {
    lv_obj_set_style_bg_color(ui_PM2p5, lv_color_hex(COLOR_RED), LV_PART_MAIN | LV_STATE_DEFAULT);
  }

  if (massConcentrationPm4p0 <= 14.4)
  {
    lv_obj_set_style_bg_color(ui_PM4, lv_color_hex(COLOR_GREEN), LV_PART_MAIN | LV_STATE_DEFAULT);
  }
  else if (massConcentrationPm4p0 <= 38)
  {
    lv_obj_set_style_bg_color(ui_PM4, lv_color_hex(COLOR_LIGHTGREEN), LV_PART_MAIN | LV_STATE_DEFAULT);
  }
  else if (massConcentrationPm4p0 <= 60)
  {
    lv_obj_set_style_bg_color(ui_PM4, lv_color_hex(COLOR_YELLOW), LV_PART_MAIN | LV_STATE_DEFAULT);
  }
  else if (massConcentrationPm4p0 <= 82)
  {
    lv_obj_set_style_bg_color(ui_PM4, lv_color_hex(COLOR_ORANGE), LV_PART_MAIN | LV_STATE_DEFAULT);
  }
  else
  {
    lv_obj_set_style_bg_color(ui_PM4, lv_color_hex(COLOR_RED), LV_PART_MAIN | LV_STATE_DEFAULT);
  }

  if (massConcentrationPm10p0 <= 20)
  {
    lv_obj_set_style_bg_color(ui_PM10, lv_color_hex(COLOR_GREEN), LV_PART_MAIN | LV_STATE_DEFAULT);
  }
  else if (massConcentrationPm10p0 <= 50)
  {
    lv_obj_set_style_bg_color(ui_PM10, lv_color_hex(COLOR_LIGHTGREEN), LV_PART_MAIN | LV_STATE_DEFAULT);
  }
  else if (massConcentrationPm10p0 <= 80)
  {
    lv_obj_set_style_bg_color(ui_PM10, lv_color_hex(COLOR_YELLOW), LV_PART_MAIN | LV_STATE_DEFAULT);
  }
  else if (massConcentrationPm10p0 <= 110)
  {
    lv_obj_set_style_bg_color(ui_PM10, lv_color_hex(COLOR_ORANGE), LV_PART_MAIN | LV_STATE_DEFAULT);
  }
  else
  {
    lv_obj_set_style_bg_color(ui_PM10, lv_color_hex(COLOR_RED), LV_PART_MAIN | LV_STATE_DEFAULT);
  }

  if (vocIndex <= 50)
  {
    lv_obj_set_style_bg_color(ui_VOC, lv_color_hex(COLOR_GREEN), LV_PART_MAIN | LV_STATE_DEFAULT);
  }
  else if (vocIndex <= 150)
  {
    lv_obj_set_style_bg_color(ui_VOC, lv_color_hex(COLOR_LIGHTGREEN), LV_PART_MAIN | LV_STATE_DEFAULT);
  }
  else if (vocIndex <= 250)
  {
    lv_obj_set_style_bg_color(ui_VOC, lv_color_hex(COLOR_YELLOW), LV_PART_MAIN | LV_STATE_DEFAULT);
  }
  else if (vocIndex <= 400)
  {
    lv_obj_set_style_bg_color(ui_VOC, lv_color_hex(COLOR_ORANGE), LV_PART_MAIN | LV_STATE_DEFAULT);
  }
  else
  {
    lv_obj_set_style_bg_color(ui_VOC, lv_color_hex(COLOR_RED), LV_PART_MAIN | LV_STATE_DEFAULT);
  }

  if (noxIndex <= 1)
  {
    lv_obj_set_style_bg_color(ui_NOX, lv_color_hex(COLOR_GREEN), LV_PART_MAIN | LV_STATE_DEFAULT);
  }
  else if (noxIndex <= 20)
  {
    lv_obj_set_style_bg_color(ui_NOX, lv_color_hex(COLOR_LIGHTGREEN), LV_PART_MAIN | LV_STATE_DEFAULT);
  }
  else if (noxIndex <= 150)
  {
    lv_obj_set_style_bg_color(ui_NOX, lv_color_hex(COLOR_YELLOW), LV_PART_MAIN | LV_STATE_DEFAULT);
  }
  else if (noxIndex <= 300)
  {
    lv_obj_set_style_bg_color(ui_NOX, lv_color_hex(COLOR_ORANGE), LV_PART_MAIN | LV_STATE_DEFAULT);
  }
  else
  {
    lv_obj_set_style_bg_color(ui_NOX, lv_color_hex(COLOR_RED), LV_PART_MAIN | LV_STATE_DEFAULT);
  }
}

void disp_weather(OpenWeatherData data)
{

  // ----- Current data
  char temp[8];
  char clouds[8];
  char uv_index[8];
  char wind_speed[8];
  char wind_gust[8];
  char sunrise[8];
  char sunset[8];

  // Icon
  std::vector<Weather> weather_vec = data.get_current().get_weather();
  Weather weather = weather_vec[0];

  const lv_img_dsc_t *selected_descriptor = NULL;
  for (uint8_t i = 0; i < NUM_ICONS; i++)
  {
    if (weather.get_icon() == icon_mapping[i].icon)
    {
      LV_LOG_WARN("get_icon %s", weather.get_icon());
      lv_img_set_src(ui_WeatherIcon, icon_mapping[i].icon_image);
      break;
    }
  }

  sprintf(temp, "%.1f", data.get_current().get_temp());
  lv_label_set_text(ui_TempCurrent, temp);

  sprintf(clouds, "%d", data.get_current().get_clouds());
  lv_label_set_text(ui_CloudsCurrent, clouds);

  sprintf(uv_index, "%.0f", data.get_current().get_uvi());
  lv_label_set_text(ui_UvCurrent, uv_index);

  sprintf(wind_speed, "%.1f", data.get_current().get_wind_speed() * 3.6); // km/h
  lv_label_set_text(ui_WindSpeedCurrent, wind_speed);

  if (data.get_current().get_wind_gust())
  {
    sprintf(wind_gust, "%.1f", data.get_current().get_wind_gust().value() * 3.6); // km/h
    lv_label_set_text(ui_WindGustCurrent, wind_gust);
  }
  else
  {
    lv_label_set_text(ui_WindGustCurrent, wind_speed);
  }

  lv_img_set_angle(ui_WindDirectionCurrentIcon, data.get_current().get_wind_deg() * 10);

  char str_sunrise[8];
  char str_sunset[8];
  struct tm time_sunrise = data.get_current().get_sunrise();
  struct tm time_sunrset = data.get_current().get_sunset();
  strftime(str_sunrise, sizeof(str_sunrise), "%H:%M", &time_sunrise);
  lv_label_set_text(ui_SunriseCurrent, str_sunrise);
  strftime(str_sunset, sizeof(str_sunset), "%H:%M", &time_sunrset);
  lv_label_set_text(ui_SunsetCurrent, str_sunset);

  // ----- Hourly data
  std::vector<Hourly> hourly = data.get_hourly();

  lv_hourly_data hourly_data[NUM_HOURS];
  for (size_t i = 0; i < NUM_HOURS; i++)
  {

    const Hourly &hourlyData = hourly[i];
    lv_hourly_data &lvData = hourly_data[i];

    lvData.dt = hourlyData.get_dt();

    lvData.temp = hourlyData.get_temp();

    const std::optional<Rain> &rain = hourlyData.get_rain();
    const std::optional<Snow> &snow = hourlyData.get_snow();
    lvData.rain = rain.has_value() ? rain.value().get_one_hour() : 0.0;
    lvData.snow = snow.has_value() ? snow.value().get_one_hour() : 0.0;

    lvData.pop = hourlyData.get_pop();

    lvData.clouds = static_cast<int32_t>(hourlyData.get_clouds());
  }

  lv_hourly_chart_set_data(ui_HourlyChart, hourly_data);
  lv_hourly_chart_refresh(ui_HourlyChart);

  // ----- Daily data
  std::vector<Daily> daily = data.get_daily();

  lv_daily_data daily_data[NUM_DAYS];
  for (size_t i = 0; i < NUM_DAYS; i++)
  {
    const Daily &dailyData = daily[i];
    lv_daily_data &lvData = daily_data[i];

    lvData.dt = dailyData.get_dt();

    lvData.low_temp = dailyData.get_temp().get_min();
    lvData.high_temp = dailyData.get_temp().get_max();

    const std::optional<double> &rain = dailyData.get_rain();
    const std::optional<double> &snow = dailyData.get_snow();
    lvData.rain = rain.has_value() ? rain.value() : 0.0;
    lvData.snow = snow.has_value() ? snow.value() : 0.0;

    lvData.pop = dailyData.get_pop();

    lvData.clouds = static_cast<int32_t>(dailyData.get_clouds());
  }

  lv_daily_chart_set_data(ui_DailyChart, daily_data);
  lv_daily_chart_refresh(ui_DailyChart);
}

void set_brightness(uint8_t brightness)
{
  gfx.setBrightness(brightness);
}

// -------- Setup Screen --------

void disp_wifi_networks(String allNetworks)
{
  lv_dropdown_clear_options(ui_DropdownNetworks);
  lv_dropdown_set_options(ui_DropdownNetworks, allNetworks.c_str());
}

void disp_connect_status(bool is_connected)
{
  if (is_connected)
  {
    lv_obj_set_style_bg_color(ui_TextAreaPassword, lv_color_hex(COLOR_LIGHTGREEN), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_TextAreaPassword, 128, LV_PART_MAIN | LV_STATE_DEFAULT);
  }
  else
  {
    lv_obj_set_style_bg_color(ui_TextAreaPassword, lv_color_hex(COLOR_ORANGE), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_TextAreaPassword, 128, LV_PART_MAIN | LV_STATE_DEFAULT);
  }
}

void set_cities(const char *region)
{
  lv_dropdown_clear_options(ui_DropdownCity);
  for (size_t i = 0; i < sizeof(cityData) / sizeof(cityData[0]); i++)
  {
    if (strcmp(cityData[i][0], region) == 0)
    {
      // Found a matching region, split city names and add them to the dropdown
      const char *cities = cityData[i][1];
      lv_dropdown_add_option(ui_DropdownCity, cities, LV_DROPDOWN_POS_LAST);
    }
  }
}

void set_labels()
{

  Preferences preferences;
  preferences.begin("Weatherstation", false);

  String name_base = preferences.getString("name_base", "");
  String name_sensor_1 = preferences.getString("name_sensor_1", "");
  String name_sensor_2 = preferences.getString("name_sensor_2", "");
  String name_sensor_3 = preferences.getString("name_sensor_3", "");

  preferences.end();

  lv_label_set_text(ui_NameBase, name_base.c_str());
  lv_label_set_text(ui_Name0, name_sensor_1.c_str());
  lv_label_set_text(ui_Name1, name_sensor_2.c_str());
  lv_label_set_text(ui_Name2, name_sensor_3.c_str());
}

void start_tasks()
{

  xTaskCreatePinnedToCore(
      clock_task,   /* Task function. */
      "Clock Task", /* String with name of task. */
      4096,         /* Stack size in bytes. */
      NULL,         /* Parameter passed as input of the task */
      1,            /* Priority of the task. */
      NULL,         /* Task handle. */
      1);           /* Clock task on core 0*/

  xTaskCreatePinnedToCore(
      sensor_task,   /* Task function. */
      "Sensor Task", /* String with name of task. */
      4096,          /* Stack size in bytes. */
      NULL,          /* Parameter passed as input of the task */
      1,             /* Priority of the task. */
      NULL,          /* Task handle. */
      1);

  xTaskCreatePinnedToCore(
      brightness_task,
      "Brightness Task",
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
}

// -------- LVGL Events --------

void event_screen_loaded(lv_event_t *e)
{
  Serial.println("event_screen_loaded");

  Preferences preferences;

  preferences.begin("Weatherstation", false);

  String ssid = preferences.getString("ssid", "");
  String password = preferences.getString("password", "");
  String appid = preferences.getString("appid", "");
  String latitude = preferences.getString("latitude", "");
  String longitude = preferences.getString("longitude", "");
  String height = preferences.getString("height", "");
  uint8_t region_id = preferences.getUInt("region", 0);
  uint8_t city_id = preferences.getUInt("city", 0);
  String name_base = preferences.getString("name_base", "");
  String name_sensor_1 = preferences.getString("name_sensor_1", "");
  String name_sensor_2 = preferences.getString("name_sensor_2", "");
  String name_sensor_3 = preferences.getString("name_sensor_3", "");

  if (ssid != "")
  {
    lv_dropdown_clear_options(ui_DropdownNetworks);
    lv_dropdown_add_option(ui_DropdownNetworks, ssid.c_str(), LV_DROPDOWN_POS_LAST);
  }
  lv_textarea_set_text(ui_TextAreaPassword, password.c_str());
  lv_textarea_set_text(ui_TextAreaAppId, appid.c_str());

  lv_textarea_set_text(ui_TextAreaLatitude, latitude.c_str());
  lv_textarea_set_text(ui_TextAreaLongitude, longitude.c_str());
  lv_textarea_set_text(ui_TextAreaHoehe, height.c_str());

  // fill the region names
  for (size_t i = 0; i < sizeof(regionNames) / sizeof(regionNames[0]); i++)
  {
    lv_dropdown_add_option(ui_DropdownRegion, regionNames[i], LV_DROPDOWN_POS_LAST);
  }
  lv_dropdown_set_selected(ui_DropdownRegion, region_id); // set the selected region id
  // get the region name
  char region[64];
  lv_dropdown_get_selected_str(ui_DropdownRegion, region, sizeof(region));
  Serial.print("region ");
  Serial.println(region);
  // fill the city list
  set_cities(region);
  // set the selected city id
  lv_dropdown_set_selected(ui_DropdownCity, city_id);

  lv_textarea_set_text(ui_TextAreaBasis, name_base.c_str());
  lv_textarea_set_text(ui_TextAreaSensorName1, name_sensor_1.c_str());
  lv_textarea_set_text(ui_TextAreaSensorName2, name_sensor_2.c_str());
  lv_textarea_set_text(ui_TextAreaSensorName3, name_sensor_3.c_str());

  preferences.end();
}

void event_wifi_scan(lv_event_t *e)
{

  Serial.println("event_wifi_scan");

  xTaskCreatePinnedToCore(
      wifiscan_task,   // Task function
      "WiFiScan Task", // Task name
      4096,            // Stack size (bytes)
      NULL,            // Task input parameter
      16,              // Task priority
      NULL,            // Task handle
      1                // Core to run the task on (0 or 1)
  );
}

void event_wifi_connect(lv_event_t *e)
{
  Serial.println("event_wifi_connect");

  char network[64];
  lv_dropdown_get_selected_str(ui_DropdownNetworks, network, sizeof(network));
  const char *password = lv_textarea_get_text(ui_TextAreaPassword);

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
      1                   // Core to run the task on (0 or 1)
  );
}

void event_value_changed(lv_event_t *e)
{
  Serial.println("event_value_changed");

  int selectedRegion = lv_dropdown_get_selected(ui_DropdownRegion);
  Serial.print("selectedRegion ");
  Serial.println(selectedRegion);

  char region[64];
  lv_dropdown_get_selected_str(ui_DropdownRegion, region, sizeof(region));
  Serial.print("region ");
  Serial.println(region);

  set_cities(region);

  lv_dropdown_set_selected(ui_DropdownCity, 0);
}

void event_weatherstation_start(lv_event_t *e)
{
  Serial.println("event_weatherstation_start");

  // Store preferences
  Preferences preferences;
  preferences.begin("Weatherstation", false);

  char ssid[64];
  lv_dropdown_get_selected_str(ui_DropdownNetworks, ssid, sizeof(ssid));
  preferences.putString("ssid", ssid);

  String password = lv_textarea_get_text(ui_TextAreaPassword);
  preferences.putString("password", password);

  String appid = lv_textarea_get_text(ui_TextAreaAppId);
  preferences.putString("appid", appid);

  String latitude = lv_textarea_get_text(ui_TextAreaLatitude);
  preferences.putString("latitude", latitude);

  String longitude = lv_textarea_get_text(ui_TextAreaLongitude);
  preferences.putString("longitude", longitude);

  String height = lv_textarea_get_text(ui_TextAreaHoehe);
  preferences.putString("height", height);

  uint8_t region_id = lv_dropdown_get_selected(ui_DropdownRegion);
  preferences.putUInt("region", region_id);

  uint8_t city_id = lv_dropdown_get_selected(ui_DropdownCity);
  preferences.putUInt("city", city_id);

  String tz;
  const char *region = regionNames[region_id];
  for (size_t i = 0; i < sizeof(cityData) / sizeof(cityData[0]); i++)
  {
    if (strcmp(cityData[i][0], region) == 0)
    {
      tz = String(cityData[i + city_id][2]);
      break;
    }
  }
  preferences.putString("tz", tz);

  String name_base = lv_textarea_get_text(ui_TextAreaBasis);
  preferences.putString("name_base", name_base);

  String name_sensor_1 = lv_textarea_get_text(ui_TextAreaSensorName1);
  preferences.putString("name_sensor_1", name_sensor_1);

  String name_sensor_2 = lv_textarea_get_text(ui_TextAreaSensorName2);
  preferences.putString("name_sensor_2", name_sensor_2);

  String name_sensor_3 = lv_textarea_get_text(ui_TextAreaSensorName3);
  preferences.putString("name_sensor_3", name_sensor_3);

  preferences.end();

  // Start application
  wifi_start();
  esp_now_start();
  set_labels();
  start_tasks();
}
