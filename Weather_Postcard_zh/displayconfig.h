#ifndef displayconfig_h
#define displayconfig_h
#include "configurations.h"
#include <U8g2_for_Adafruit_GFX.h>
#define ENABLE_GxEPD2_GFX 1
extern U8G2_FOR_ADAFRUIT_GFX u8g2Fonts;



// Choose any ONE of the display type only
// #define display_BW
// #define display_4G
// #define display_BWR
// #define display_BWY
#define display_4C

// Change the pin numbers

#define CS 7
#define DC 10
#define RST 0
#define BUSY 5

/* <-************************Change below as needed*******************************->*/

#define batterypin 1 // battery ADC pin (voltage dividers using same resistor values), -1 if you are not using battery ADC


/* <-************************Change above as needed*******************************->*/

#ifdef display_4G
// #include <GxEPD2_4G_BW.h>
#include <GxEPD2_4G_4G.h>
#define GxEPD2_DISPLAY_CLASS GxEPD2_4G_4G
#define GxEPD2_DRIVER_CLASS GxEPD2_420_GDEY042T81 // or other panels as needed, not tested

#elif defined display_BW
#include <GxEPD2_BW.h>
#define GxEPD2_DISPLAY_CLASS GxEPD2_BW
#define GxEPD2_DRIVER_CLASS GxEPD2_420_GDEY042T81 // or other panels as needed, not tested

#elif defined display_BWR
#include <GxEPD2_3C.h>
#define GxEPD2_DISPLAY_CLASS GxEPD2_3C
#define GxEPD2_DRIVER_CLASS GxEPD2_420c // or other panels as needed, not tested
#elif defined display_BWY
#include <GxEPD2_3C.h>
#define GxEPD2_DISPLAY_CLASS GxEPD2_3C
#define GxEPD2_DRIVER_CLASS BWYPLACEHOLDER // I don't have a panel to test,
#else
#include <GxEPD2_4C.h>
#define GxEPD2_DISPLAY_CLASS GxEPD2_4C
#define GxEPD2_DRIVER_CLASS GxEPD2_420c_GDEY0420F51
#endif

#define MAX_DISPLAY_BUFFER_SIZE 65536ul
#define MAX_HEIGHT(EPD) (EPD::HEIGHT <= MAX_DISPLAY_BUFFER_SIZE / (EPD::WIDTH / 8) ? EPD::HEIGHT : MAX_DISPLAY_BUFFER_SIZE / (EPD::WIDTH / 8))
extern GxEPD2_DISPLAY_CLASS<GxEPD2_DRIVER_CLASS, MAX_HEIGHT(GxEPD2_DRIVER_CLASS)> display;

#ifdef display_BW
#define black GxEPD_BLACK
#define white GxEPD_WHITE
#define yellow GxEPD_WHITE
#define red GxEPD_BLACK

#elif defined display_4G
#define black GxEPD_BLACK
#define white GxEPD_WHITE
#define yellow GxEPD_LIGHTGREY
#define red GxEPD_DARKGREY

#elif defined display_BWR
#define black GxEPD_BLACK
#define white GxEPD_WHITE
#define yellow GxEPD_YELLOW
#define red GxEPD_BLACK

#elif defined display_BWY
#define black GxEPD_BLACK
#define white GxEPD_WHITE
#define yellow GxEPD_WHITE
#define red GxEPD_RED

#else
#define black GxEPD_BLACK
#define white GxEPD_WHITE
#define yellow GxEPD_YELLOW
#define red GxEPD_RED
#endif

// select u8g2 font from here: https://github.com/olikraus/u8g2/wiki/fntlistall
// it is highly recommended you keep the fonts as they are
#define forecastfont u8g2_font_t0_15b_tf
#define datefont u8g2_font_t0_14b_tn
//#define datefont u8g2_font_t0_15b_tf // test use same font
#define smallfont u8g2_font_t0_11b_tf

 #include "assets\u8g2_font_WPfont.c"
 #define chinesefont WPfont

#endif