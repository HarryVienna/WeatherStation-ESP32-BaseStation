// This file was generated by SquareLine Studio
// SquareLine Studio version: SquareLine Studio 1.3.0
// LVGL version: 8.3.6
// Project name: SquareLine_Project

#include "ui.h"
#include "ui_helpers.h"

///////////////////// VARIABLES ////////////////////

// SCREEN: ui_Screen
void ui_Screen_screen_init(void);
lv_obj_t * ui_Screen;
lv_obj_t * ui_Current;
lv_obj_t * ui_DateTime;
lv_obj_t * ui_Panel15;
lv_obj_t * ui_WeatherIcon;
lv_obj_t * ui_Panel12;
lv_obj_t * ui_LineTempCurrent;
lv_obj_t * ui_TempCurrent;
lv_obj_t * ui_TempCurrentUnit;
lv_obj_t * ui_LineCloudsUvCurrent;
lv_obj_t * ui_PanelCloudsCurrent;
lv_obj_t * ui_CloudsCurrentIcon;
lv_obj_t * ui_CloudsCurrent;
lv_obj_t * ui_CloudsCurrentUnit;
lv_obj_t * ui_PanelUvCurrent;
lv_obj_t * ui_UvCurrentIcon;
lv_obj_t * ui_UvCurrent;
lv_obj_t * ui_Panel13;
lv_obj_t * ui_WindDirectionCurrentIcon;
lv_obj_t * ui_PanelWindCurrent;
lv_obj_t * ui_LineWindSpeedCurrent;
lv_obj_t * ui_WindSpeedCurrent;
lv_obj_t * ui_WindSpeedCurrentUnit;
lv_obj_t * ui_LineWindGustCurrent;
lv_obj_t * ui_WindGustCurrent;
lv_obj_t * ui_WindGustCurrentUnit;
lv_obj_t * ui_PanelSunCurrent;
lv_obj_t * ui_LineSunriseCurrent;
lv_obj_t * ui_SunriseCurrentIcon;
lv_obj_t * ui_SunriseCurrent;
lv_obj_t * ui_LineSunsetCurrent;
lv_obj_t * ui_SunsetCurrentIcon;
lv_obj_t * ui_SunsetCurrent;
lv_obj_t * ui_Base;
lv_obj_t * ui_NameBase;
lv_obj_t * ui_LineTempBase;
lv_obj_t * ui_TempBase;
lv_obj_t * ui_TempBaseUnit;
lv_obj_t * ui_LineHumidityBase;
lv_obj_t * ui_IconHumidityBase;
lv_obj_t * ui_HumidityBase;
lv_obj_t * ui_HumidityBaseIcon;
lv_obj_t * ui_PanelSensorsBase;
lv_obj_t * ui_PanelPM;
lv_obj_t * ui_PM10;
lv_obj_t * ui_PM4;
lv_obj_t * ui_PM2p5;
lv_obj_t * ui_PM1;
lv_obj_t * ui_LabelPM;
lv_obj_t * ui_PanelVOC;
lv_obj_t * ui_VOC;
lv_obj_t * ui_LabelVOC;
lv_obj_t * ui_PanelNOX;
lv_obj_t * ui_NOX;
lv_obj_t * ui_Panel21;
lv_obj_t * ui_LabelNOX1;
lv_obj_t * ui_LabelNOX2;
lv_obj_t * ui_PanelCO2;
lv_obj_t * ui_CO2;
lv_obj_t * ui_Panel23;
lv_obj_t * ui_LabelCO21;
lv_obj_t * ui_LabelCO22;
lv_obj_t * ui_Sensor0;
lv_obj_t * ui_Name0;
lv_obj_t * ui_LineTemp0;
lv_obj_t * ui_Temp0;
lv_obj_t * ui_Unit0;
lv_obj_t * ui_LineHumidity0;
lv_obj_t * ui_ImageHumidity0;
lv_obj_t * ui_Hunidity0;
lv_obj_t * ui_Hunidity0Unit;
lv_obj_t * ui_LinePressure0;
lv_obj_t * ui_ImagePressure0;
lv_obj_t * ui_Pressure0;
lv_obj_t * ui_Pressure0Unit;
lv_obj_t * ui_LineStatus0;
lv_obj_t * ui_ImageVolt0;
lv_obj_t * ui_Volt0;
lv_obj_t * ui_ImageUpdate0;
lv_obj_t * ui_Update0;
lv_obj_t * ui_Sensor1;
lv_obj_t * ui_Name1;
lv_obj_t * ui_LineTemp1;
lv_obj_t * ui_Temp1;
lv_obj_t * ui_Unit1;
lv_obj_t * ui_LineHumidity1;
lv_obj_t * ui_ImageHumidity1;
lv_obj_t * ui_Hunidity1;
lv_obj_t * ui_Hunidity1Unit;
lv_obj_t * ui_LinePressure1;
lv_obj_t * ui_ImagePressure1;
lv_obj_t * ui_Pressure1;
lv_obj_t * ui_Pressure1Unit;
lv_obj_t * ui_LineStatus1;
lv_obj_t * ui_ImageVolt1;
lv_obj_t * ui_Volt1;
lv_obj_t * ui_ImageUpdate1;
lv_obj_t * ui_Update1;
lv_obj_t * ui_Sensor2;
lv_obj_t * ui_Name2;
lv_obj_t * ui_LineTemp2;
lv_obj_t * ui_Temp2;
lv_obj_t * ui_Unit2;
lv_obj_t * ui_LineHumidity2;
lv_obj_t * ui_ImageHumidity2;
lv_obj_t * ui_Hunidity2;
lv_obj_t * ui_Hunidity2Unit;
lv_obj_t * ui_LinePressure2;
lv_obj_t * ui_ImagePressure2;
lv_obj_t * ui_Pressure2;
lv_obj_t * ui_Pressure2Unit;
lv_obj_t * ui_LineStatus2;
lv_obj_t * ui_ImageVolt2;
lv_obj_t * ui_Volt2;
lv_obj_t * ui_ImageUpdate2;
lv_obj_t * ui_Update2;
lv_obj_t * ui_Hourly;
lv_obj_t * ui_HourlyChart;
lv_obj_t * ui_Daily;
lv_obj_t * ui_DailyChart;
lv_obj_t * ui____initial_actions0;

///////////////////// TEST LVGL SETTINGS ////////////////////
#if LV_COLOR_DEPTH != 16
    #error "LV_COLOR_DEPTH should be 16bit to match SquareLine Studio's settings"
#endif
#if LV_COLOR_16_SWAP !=0
    #error "LV_COLOR_16_SWAP should be 0 to match SquareLine Studio's settings"
#endif

///////////////////// ANIMATIONS ////////////////////

///////////////////// FUNCTIONS ////////////////////

///////////////////// SCREENS ////////////////////

void ui_init(void)
{
    lv_disp_t * dispp = lv_disp_get_default();
    lv_theme_t * theme = lv_theme_default_init(dispp, lv_palette_main(LV_PALETTE_BLUE), lv_palette_main(LV_PALETTE_RED),
                                               false, LV_FONT_DEFAULT);
    lv_disp_set_theme(dispp, theme);
    ui_Screen_screen_init();
    ui____initial_actions0 = lv_obj_create(NULL);
    lv_disp_load_scr(ui_Screen);
}