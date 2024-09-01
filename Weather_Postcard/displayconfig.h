//Choose one of the option below only
#define display_4C
//#define display_BW // 
//#define display_3C // For B/W/R 3 color display / Note: yellow won't show


//Choose one of the board below
//#define GoodDisplayESP32 // (select WEMOS LOLIN32/ ESP32 Dev Module when compling)
#define Firebeetle2ESP32C6 // (select DFRobot Beetle ESP32-C6 when compling)
//#define OtherESP32boards // (For other boards, also define the pins below. I have not and will not be testing on other boards, use at your own risk)

#ifdef OtherESP32boards
#define CS -1
#define DC -1
#define RST -1
#define BUSY -1
#endif



#ifdef Firebeetle2ESP32C6
#define CS 1
#define DC 18
#define RST 9
#define BUSY 14
#endif

#ifdef GoodDisplayESP32
#define CS 27
#define DC 14
#define RST 12
#define BUSY 13
#endif




#define MAX_DISPLAY_BUFFER_SIZE 65536ul 


#ifdef display_BW
#include <GxEPD2_4G_4G.h>
#include <GxEPD2_4G_BW.h>
#define GxEPD2_DRIVER_CLASS GxEPD2_420_GDEY042T81


#define MAX_HEIGHT_BW(EPD) (EPD::HEIGHT <= (MAX_DISPLAY_BUFFER_SIZE / 2) / (EPD::WIDTH / 8) ? EPD::HEIGHT : (MAX_DISPLAY_BUFFER_SIZE / 2) / (EPD::WIDTH / 8))
#define MAX_HEIGHT_4G(EPD) (EPD::HEIGHT <= (MAX_DISPLAY_BUFFER_SIZE / 2) / (EPD::WIDTH / 4) ? EPD::HEIGHT : (MAX_DISPLAY_BUFFER_SIZE / 2) / (EPD::WIDTH / 4))
GxEPD2_4G_4G<GxEPD2_DRIVER_CLASS, MAX_HEIGHT_4G(GxEPD2_DRIVER_CLASS)> display(GxEPD2_DRIVER_CLASS(CS, DC, RST, BUSY));  


GxEPD2_4G_BW_R<GxEPD2_DRIVER_CLASS, MAX_HEIGHT_BW(GxEPD2_DRIVER_CLASS)> display_bw(display.epd2);
#undef MAX_HEIGHT_BW
#undef MAX_HEIGHT_4G



uint16_t white = GxEPD_WHITE;
uint16_t black = GxEPD_BLACK;
uint16_t red = GxEPD_DARKGREY;
uint16_t yellow = GxEPD_LIGHTGREY;


#endif


#ifdef display_4C

#include <GxEPD2_4C.h>


#define GxEPD2_DISPLAY_CLASS GxEPD2_4C
#define GxEPD2_DRIVER_CLASS GxEPD2_420c_GDEY0420F51

#define MAX_HEIGHT(EPD) (EPD::HEIGHT <= MAX_DISPLAY_BUFFER_SIZE / (EPD::WIDTH / 8) ? EPD::HEIGHT : MAX_DISPLAY_BUFFER_SIZE / (EPD::WIDTH / 8))

GxEPD2_DISPLAY_CLASS<GxEPD2_DRIVER_CLASS, MAX_HEIGHT(GxEPD2_DRIVER_CLASS)> display(GxEPD2_DRIVER_CLASS(CS, DC, RST, BUSY));


#undef MAX_HEIGHT

uint16_t white = GxEPD_WHITE;
uint16_t black = GxEPD_BLACK;
uint16_t red = GxEPD_RED;
uint16_t yellow = GxEPD_YELLOW;

#endif


#ifdef display_3C

#include <GxEPD2_3C.h>


#define GxEPD2_DISPLAY_CLASS GxEPD2_3C
#define GxEPD2_DRIVER_CLASS GxEPD2_420c

#define MAX_DISPLAY_BUFFER_SIZE 65536ul 
#define MAX_HEIGHT(EPD) (EPD::HEIGHT <= MAX_DISPLAY_BUFFER_SIZE / (EPD::WIDTH / 8) ? EPD::HEIGHT : MAX_DISPLAY_BUFFER_SIZE / (EPD::WIDTH / 8))

GxEPD2_DISPLAY_CLASS<GxEPD2_DRIVER_CLASS, MAX_HEIGHT(GxEPD2_DRIVER_CLASS)> display(GxEPD2_DRIVER_CLASS(CS, DC, RST, BUSY));


#undef MAX_HEIGHT

uint16_t white = GxEPD_WHITE;
uint16_t black = GxEPD_BLACK;
uint16_t red = GxEPD_RED;
uint16_t yellow = GxEPD_WHITE;

#endif

#undef MAX_DISPLAY_BUFFER_SIZE
#define ENABLE_GxEPD2_GFX 1
#include <U8g2_for_Adafruit_GFX.h>
U8G2_FOR_ADAFRUIT_GFX u8g2Fonts;



 // select u8g2 font from here: https://github.com/olikraus/u8g2/wiki/fntlistall
 // it is highly recommended you keep the fonts as they are
#define forecastfont u8g2_font_t0_15b_tf
#define datefont u8g2_font_t0_14b_tn
#define smallfont u8g2_font_t0_11b_tf