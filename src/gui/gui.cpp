#include <Arduino.h>

#include <lv_conf.h>
#include <lvgl.h>
#include "../ui/ui.h"
#include "../gfx/LGFX_ESP32S3_RGB_MakerfabsParallelTFTwithTouch70.h"
#include "../lvgl/lv_hourly_chart.h"
#include "../lvgl/lv_daily_chart.h"
#include "../config/config.h"

#include "gui.h"

#include "SPI.h"
#include "SD.h"
#include "FS.h"

extern SemaphoreHandle_t mutex;

#define NUM_ICONS 18

typedef struct {
    const char* icon;
    const lv_img_dsc_t* icon_image;
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
    {"50n", &ui_img_50n_png}
};

static const char* TAG = "gui";



static const uint16_t screenWidth  = 800;
static const uint16_t screenHeight = 480;

static lv_disp_draw_buf_t draw_buf;
static lv_color_t buf[2][ screenWidth * 10 ];

LGFX gfx;

float calc_sea_level_pressure(float pressure, float temperature, uint16_t altitude)
{
  // https://de.wikipedia.org/wiki/Barometrische_H%C3%B6henformel

  // Konstanten
  float g = 9.80665;  // Schwerebeschleunigung in m / s^2
  float R = 287.05;  // Gaskonstante trockener Luft (= R/M)  in m^2/(s²K)
  float a = 0.0065;  // vertikaler Temperaturgradient
  float C_h = 0.12;  // Beiwert zur Berücksichtigung der mittleren Dampfdruckänderung K/hPa
  float T_0 = 273.15;  // Celsius to Kelvin

  float E; // Dampfdruck des Wasserdampfanteils (in hPa)

  if (temperature < 9.1) {
      E = 5.6402 * (-0.0916 + exp(0.06 * temperature));
  }    
  else {
      E = 18.2194 * (1.0463 + exp(-0.0666 * temperature));
  }

  // Luftdruck auf Meereshöhe berechnen
  float p = pressure * exp(altitude * g / (R * (temperature + T_0 + C_h * E + a * (altitude / 2))));

  return p;
}

/* Display flushing */
void disp_flush( lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p )
{
    if (gfx.getStartCount() == 0)
    {   // Processing if not yet started
        gfx.startWrite();
    }
    gfx.pushImageDMA( area->x1
                    , area->y1
                    , area->x2 - area->x1 + 1
                    , area->y2 - area->y1 + 1
                    , ( lgfx::rgb565_t* )&color_p->full);
    lv_disp_flush_ready( disp );
}


/*Read the touchpad*/
void touchpad_read( lv_indev_drv_t * indev_driver, lv_indev_data_t * data )
{
    uint16_t touchX, touchY;

    data->state = LV_INDEV_STATE_REL;

    if( gfx.getTouch( &touchX, &touchY ) )
    {
        data->state = LV_INDEV_STATE_PR;

        /*Set the coordinates*/
        data->point.x = touchX;
        data->point.y = touchY;
    }
}

void gui_screenshot() {

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
    'B',  'M',
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
    0x11, 0x00, 0x00, 0x00, 0x00, 0x00
  };






  lv_obj_t * obj = lv_scr_act();
  lv_img_cf_t cf = LV_IMG_CF_TRUE_COLOR;

  LV_ASSERT_NULL(obj);
  uint32_t buff_size = lv_snapshot_buf_size_needed(obj, cf);


  void *buf = heap_caps_malloc(buff_size, MALLOC_CAP_32BIT | MALLOC_CAP_SPIRAM);


  lv_img_dsc_t * snapshot = (lv_img_dsc_t *)heap_caps_malloc(sizeof(lv_img_dsc_t), MALLOC_CAP_32BIT | MALLOC_CAP_SPIRAM);



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

    lv_disp_t * obj_disp = lv_obj_get_disp(obj);
    lv_disp_drv_t driver;

    lv_disp_drv_init(&driver);

    /*In lack of a better idea use the resolution of the object's display*/
    driver.hor_res = lv_disp_get_hor_res(obj_disp);
    driver.ver_res = lv_disp_get_hor_res(obj_disp);
    lv_disp_drv_use_generic_set_px_cb(&driver, cf);

    lv_disp_t fake_disp;
    lv_memset_00(&fake_disp, sizeof(lv_disp_t));
    fake_disp.driver = &driver;


    lv_draw_ctx_t * draw_ctx = (lv_draw_ctx_t *) heap_caps_malloc(obj_disp->driver->draw_ctx_size, MALLOC_CAP_32BIT | MALLOC_CAP_SPIRAM);

    obj_disp->driver->draw_ctx_init(fake_disp.driver, draw_ctx);
    fake_disp.driver->draw_ctx = draw_ctx;
    draw_ctx->clip_area = &snapshot_area;
    draw_ctx->buf_area = &snapshot_area;
    draw_ctx->buf = (void *)buf;
    driver.draw_ctx = draw_ctx;

    lv_disp_t * refr_ori = _lv_refr_get_disp_refreshing();
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
    //SPI.setFrequency(100000);
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

  for (int y = h - 1; y >= 0; y--) {
    for (int x = 0; x < w; x++) {

      lv_color_t pixel = lv_img_buf_get_px_color(snapshot, x, y, {});

      uint32_t dst = (0xFF << 24)
          | (pixel.ch.red << 19)
          | (pixel.ch.green << 10)
          | (pixel.ch.blue << 3);

      bmpFile.write((uint8_t*)&dst, sizeof(uint32_t));
      Serial.print(".");
    }
  }
  Serial.println("write ok");

  //lv_snapshot_free(snapshot);
  bmpFile.flush();
  bmpFile.close();
  SD.end();
Serial.println("end");
}

/**
 * @brief  
 *
 */
void gui_start(){

  // ----------- GFX -------------
  gfx.begin();
//  gfx.setColorDepth(16);
  gfx.setBrightness(127);


  lv_init();
  lv_disp_draw_buf_init( &draw_buf, buf[0], buf[1], screenWidth * 10 );

  /*Initialize the display*/
  static lv_disp_drv_t disp_drv;
  lv_disp_drv_init( &disp_drv );
  disp_drv.hor_res = screenWidth;
  disp_drv.ver_res = screenHeight;
  disp_drv.flush_cb = disp_flush;
  disp_drv.draw_buf = &draw_buf;
  lv_disp_drv_register( &disp_drv );

  /*Initialize the input device driver*/
  static lv_indev_drv_t indev_drv;
  lv_indev_drv_init( &indev_drv );
  indev_drv.type = LV_INDEV_TYPE_POINTER;
  indev_drv.read_cb = touchpad_read;
  lv_indev_drv_register( &indev_drv );

  ui_init();

}



void disp_date_time(char* date_time) {
  lv_label_set_text(ui_DateTime, date_time);
}

void disp_sensor_data(uint8_t sensor_nr, double temperature, double humidity, double pressure, uint32_t voltage, char* date_time) {

  pressure = calc_sea_level_pressure(pressure, temperature, HEIGHT_ABOVE_SEALEVEL);
  
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
    
    if (voltage_rounded >= 4.0) {
      lv_obj_set_style_bg_color(ui_LineStatus0, lv_color_hex(COLOR_GREEN), LV_PART_MAIN | LV_STATE_DEFAULT);
    }
    else if (voltage_rounded >=3.8) {
      lv_obj_set_style_bg_color(ui_LineStatus0, lv_color_hex(COLOR_LIGHTGREEN), LV_PART_MAIN | LV_STATE_DEFAULT);
    }
    else if (voltage_rounded >=3.6) {
      lv_obj_set_style_bg_color(ui_LineStatus0, lv_color_hex(COLOR_ORANGE), LV_PART_MAIN | LV_STATE_DEFAULT);
    }
    else {
      lv_obj_set_style_bg_color(ui_LineStatus0, lv_color_hex(COLOR_RED), LV_PART_MAIN | LV_STATE_DEFAULT);
    }
    break;
  case 1:
    lv_label_set_text(ui_Temp1, temperature_value);
    lv_label_set_text(ui_Hunidity1, humidity_value);
    lv_label_set_text(ui_Pressure1, pressure_value);
    lv_label_set_text(ui_Volt1, voltage_value);
    lv_label_set_text(ui_Update1, date_time);
    if (voltage_rounded >= 4.0) {
      lv_obj_set_style_bg_color(ui_LineStatus1, lv_color_hex(COLOR_GREEN), LV_PART_MAIN | LV_STATE_DEFAULT);
    }
    else if (voltage_rounded >=3.8) {
      lv_obj_set_style_bg_color(ui_LineStatus1, lv_color_hex(COLOR_LIGHTGREEN), LV_PART_MAIN | LV_STATE_DEFAULT);
    }
    else if (voltage_rounded >=3.6) {
      lv_obj_set_style_bg_color(ui_LineStatus1, lv_color_hex(COLOR_ORANGE), LV_PART_MAIN | LV_STATE_DEFAULT);
    }
    else {
      lv_obj_set_style_bg_color(ui_LineStatus1, lv_color_hex(COLOR_RED), LV_PART_MAIN | LV_STATE_DEFAULT);
    }  
    break;
  case 2:
    lv_label_set_text(ui_Temp2, temperature_value);
    lv_label_set_text(ui_Hunidity2, humidity_value);
    lv_label_set_text(ui_Pressure2, pressure_value);
    lv_label_set_text(ui_Volt2, voltage_value);
    lv_label_set_text(ui_Update2, date_time);
    if (voltage_rounded >= 4.0) {
      lv_obj_set_style_bg_color(ui_LineStatus2, lv_color_hex(COLOR_GREEN), LV_PART_MAIN | LV_STATE_DEFAULT);
    }
    else if (voltage_rounded >=3.8) {
      lv_obj_set_style_bg_color(ui_LineStatus2, lv_color_hex(COLOR_LIGHTGREEN), LV_PART_MAIN | LV_STATE_DEFAULT);
    }
    else if (voltage_rounded >=3.6) {
      lv_obj_set_style_bg_color(ui_LineStatus2, lv_color_hex(COLOR_ORANGE), LV_PART_MAIN | LV_STATE_DEFAULT);
    }
    else {
      lv_obj_set_style_bg_color(ui_LineStatus2, lv_color_hex(COLOR_RED), LV_PART_MAIN | LV_STATE_DEFAULT);
    }
    break;
  default:
    break;
  }

}



void disp_scd4x(uint16_t co2) {
  if (co2 <= 600) {
    lv_obj_set_style_bg_color(ui_CO2, lv_color_hex(COLOR_GREEN), LV_PART_MAIN | LV_STATE_DEFAULT);
  }
  else if (co2 <=1000 ) {
    lv_obj_set_style_bg_color(ui_CO2, lv_color_hex(COLOR_LIGHTGREEN), LV_PART_MAIN | LV_STATE_DEFAULT);
  }
  else if (co2 <=1500 ) {
    lv_obj_set_style_bg_color(ui_CO2, lv_color_hex(COLOR_YELLOW), LV_PART_MAIN | LV_STATE_DEFAULT);
  }
  else if (co2 <=1900 ) {
    lv_obj_set_style_bg_color(ui_CO2, lv_color_hex(COLOR_ORANGE), LV_PART_MAIN | LV_STATE_DEFAULT);
  }
  else {
    lv_obj_set_style_bg_color(ui_CO2, lv_color_hex(COLOR_RED), LV_PART_MAIN | LV_STATE_DEFAULT);
  }
}

void disp_sen5x(float ambientTemperature, float ambientHumidity, float massConcentrationPm1p0, float massConcentrationPm2p5, float massConcentrationPm4p0, float massConcentrationPm10p0, float vocIndex, float noxIndex) {
  
  if (!isnan(ambientTemperature)) {
      char temp[8];
      sprintf(temp, "%.1f", ambientTemperature);
      lv_label_set_text(ui_TempBase, temp);
  }

    if (!isnan(ambientHumidity)) {
      char humidity[8];
      sprintf(humidity, "%.1f", ambientHumidity);
      lv_label_set_text(ui_HumidityBase, humidity);
  }

  if (massConcentrationPm1p0 <= 11.6) {
    lv_obj_set_style_bg_color(ui_PM1, lv_color_hex(COLOR_GREEN), LV_PART_MAIN | LV_STATE_DEFAULT);
  }
  else if (massConcentrationPm1p0 <= 32 ) {
    lv_obj_set_style_bg_color(ui_PM1, lv_color_hex(COLOR_LIGHTGREEN), LV_PART_MAIN | LV_STATE_DEFAULT);
  }
  else if (massConcentrationPm1p0 <= 50 ) {
    lv_obj_set_style_bg_color(ui_PM1, lv_color_hex(COLOR_YELLOW), LV_PART_MAIN | LV_STATE_DEFAULT);
  }
  else if (massConcentrationPm1p0 <= 68 ) {
    lv_obj_set_style_bg_color(ui_PM1, lv_color_hex(COLOR_ORANGE), LV_PART_MAIN | LV_STATE_DEFAULT);
  }
  else {
    lv_obj_set_style_bg_color(ui_PM1, lv_color_hex(COLOR_RED), LV_PART_MAIN | LV_STATE_DEFAULT);
  }

  if (massConcentrationPm2p5 <= 13) {
    lv_obj_set_style_bg_color(ui_PM2p5, lv_color_hex(COLOR_GREEN), LV_PART_MAIN | LV_STATE_DEFAULT);
  }
  else if (massConcentrationPm2p5 <= 35 ) {
    lv_obj_set_style_bg_color(ui_PM2p5, lv_color_hex(COLOR_LIGHTGREEN), LV_PART_MAIN | LV_STATE_DEFAULT);
  }
  else if (massConcentrationPm2p5 <= 55 ) {
    lv_obj_set_style_bg_color(ui_PM2p5, lv_color_hex(COLOR_YELLOW), LV_PART_MAIN | LV_STATE_DEFAULT);
  }
  else if (massConcentrationPm2p5 <= 75 ) {
    lv_obj_set_style_bg_color(ui_PM2p5, lv_color_hex(COLOR_ORANGE), LV_PART_MAIN | LV_STATE_DEFAULT);
  }
  else {
    lv_obj_set_style_bg_color(ui_PM2p5, lv_color_hex(COLOR_RED), LV_PART_MAIN | LV_STATE_DEFAULT);
  }

  if (massConcentrationPm4p0 <= 14.4) {
    lv_obj_set_style_bg_color(ui_PM4, lv_color_hex(COLOR_GREEN), LV_PART_MAIN | LV_STATE_DEFAULT);
  }
  else if (massConcentrationPm4p0 <= 38 ) {
    lv_obj_set_style_bg_color(ui_PM4, lv_color_hex(COLOR_LIGHTGREEN), LV_PART_MAIN | LV_STATE_DEFAULT);
  }
  else if (massConcentrationPm4p0 <= 60 ) {
    lv_obj_set_style_bg_color(ui_PM4, lv_color_hex(COLOR_YELLOW), LV_PART_MAIN | LV_STATE_DEFAULT);
  }
  else if (massConcentrationPm4p0 <= 82 ) {
    lv_obj_set_style_bg_color(ui_PM4, lv_color_hex(COLOR_ORANGE), LV_PART_MAIN | LV_STATE_DEFAULT);
  }
  else {
    lv_obj_set_style_bg_color(ui_PM4, lv_color_hex(COLOR_RED), LV_PART_MAIN | LV_STATE_DEFAULT);
  }

  if (massConcentrationPm10p0 <= 20) {
    lv_obj_set_style_bg_color(ui_PM10, lv_color_hex(COLOR_GREEN), LV_PART_MAIN | LV_STATE_DEFAULT);
  }
  else if (massConcentrationPm10p0 <= 50 ) {
    lv_obj_set_style_bg_color(ui_PM10, lv_color_hex(COLOR_LIGHTGREEN), LV_PART_MAIN | LV_STATE_DEFAULT);
  }
  else if (massConcentrationPm10p0 <= 80 ) {
    lv_obj_set_style_bg_color(ui_PM10, lv_color_hex(COLOR_YELLOW), LV_PART_MAIN | LV_STATE_DEFAULT);
  }
  else if (massConcentrationPm10p0 <= 110 ) {
    lv_obj_set_style_bg_color(ui_PM10, lv_color_hex(COLOR_ORANGE), LV_PART_MAIN | LV_STATE_DEFAULT);
  }
  else {
    lv_obj_set_style_bg_color(ui_PM10, lv_color_hex(COLOR_RED), LV_PART_MAIN | LV_STATE_DEFAULT);
  }

  if (vocIndex <= 50) {
    lv_obj_set_style_bg_color(ui_VOC, lv_color_hex(COLOR_GREEN), LV_PART_MAIN | LV_STATE_DEFAULT);
  }
  else if (vocIndex <= 150 ) {
    lv_obj_set_style_bg_color(ui_VOC, lv_color_hex(COLOR_LIGHTGREEN), LV_PART_MAIN | LV_STATE_DEFAULT);
  }
  else if (vocIndex <= 250 ) {
    lv_obj_set_style_bg_color(ui_VOC, lv_color_hex(COLOR_YELLOW), LV_PART_MAIN | LV_STATE_DEFAULT);
  }
  else if (vocIndex <= 400 ) {
    lv_obj_set_style_bg_color(ui_VOC, lv_color_hex(COLOR_ORANGE), LV_PART_MAIN | LV_STATE_DEFAULT);
  }
  else {
    lv_obj_set_style_bg_color(ui_VOC, lv_color_hex(COLOR_RED), LV_PART_MAIN | LV_STATE_DEFAULT);
  }  

  if (noxIndex <= 1) {
    lv_obj_set_style_bg_color(ui_NOX, lv_color_hex(COLOR_GREEN), LV_PART_MAIN | LV_STATE_DEFAULT);
  }
  else if (noxIndex <= 20 ) {
    lv_obj_set_style_bg_color(ui_NOX, lv_color_hex(COLOR_LIGHTGREEN), LV_PART_MAIN | LV_STATE_DEFAULT);
  }
  else if (noxIndex <= 150 ) {
    lv_obj_set_style_bg_color(ui_NOX, lv_color_hex(COLOR_YELLOW), LV_PART_MAIN | LV_STATE_DEFAULT);
  }
  else if (noxIndex <= 300 ) {
    lv_obj_set_style_bg_color(ui_NOX, lv_color_hex(COLOR_ORANGE), LV_PART_MAIN | LV_STATE_DEFAULT);
  }
  else {
    lv_obj_set_style_bg_color(ui_NOX, lv_color_hex(COLOR_RED), LV_PART_MAIN | LV_STATE_DEFAULT);
  }  
}

void disp_weather(OpenWeatherData data) {

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

  const lv_img_dsc_t* selected_descriptor = NULL;
  for (uint8_t i = 0; i < NUM_ICONS; i++) {
      if (weather.get_icon() == icon_mapping[i].icon) {
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

  sprintf(wind_speed, "%.1f", data.get_current().get_wind_speed() * 3.6);  // km/h
  lv_label_set_text(ui_WindSpeedCurrent, wind_speed);

  if (data.get_current().get_wind_gust()) {
    sprintf(wind_gust, "%.1f", data.get_current().get_wind_gust().value() * 3.6);  // km/h
    lv_label_set_text(ui_WindGustCurrent, wind_gust);
  }
  else {
      lv_label_set_text(ui_WindGustCurrent, wind_speed);
  }    

  lv_img_set_angle(ui_WindDirectionCurrentIcon, data.get_current().get_wind_deg() * 10);

  char str_sunrise[8];
  char str_sunset[8];
  struct tm time_sunrise = data.get_current().get_sunrise();
  struct tm time_sunrset = data.get_current().get_sunset();
  strftime(str_sunrise, sizeof(str_sunrise),"%H:%M", &time_sunrise);
  lv_label_set_text(ui_SunriseCurrent, str_sunrise);
  strftime(str_sunset, sizeof(str_sunset),"%H:%M", &time_sunrset);
  lv_label_set_text(ui_SunsetCurrent, str_sunset);

  // ----- Hourly data
  std::vector<Hourly> hourly = data.get_hourly();

  lv_hourly_data hourly_data[NUM_HOURS];
  for (size_t i = 0; i < NUM_HOURS; i++) {
   
    const Hourly& hourlyData = hourly[i];
    lv_hourly_data& lvData = hourly_data[i];

    lvData.dt = hourlyData.get_dt();

    lvData.temp = hourlyData.get_temp();

    const std::optional<Rain>& rain = hourlyData.get_rain();
    const std::optional<Snow>& snow = hourlyData.get_snow();
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
  for (size_t i = 0; i < NUM_DAYS; i++) {
    const Daily& dailyData = daily[i];
    lv_daily_data& lvData = daily_data[i];

    lvData.dt = dailyData.get_dt();

    lvData.low_temp = dailyData.get_temp().get_min();
    lvData.high_temp = dailyData.get_temp().get_max();

    const std::optional<double>& rain = dailyData.get_rain();
    const std::optional<double>& snow = dailyData.get_snow();
    lvData.rain = rain.has_value() ? rain.value() : 0.0;
    lvData.snow = snow.has_value() ? snow.value() : 0.0;

    lvData.pop = dailyData.get_pop();

    lvData.clouds = static_cast<int32_t>(dailyData.get_clouds());
  }

  lv_daily_chart_set_data(ui_DailyChart, daily_data);
  lv_daily_chart_refresh(ui_DailyChart);
}

void set_brightness(uint8_t brightness) {
  gfx.setBrightness(brightness);
}