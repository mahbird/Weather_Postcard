/*
The Weather postcard and all icons are created and drawn by mahbird on Github, written in Arduino IDE 2.3.2, based on information available from the Hong Kong Observatory. 
The code is distributed for free on github. For personal use only. Please retain all original messages when posting on other sites.
You are free to modify this code as you like, but do not redistribute modified code and do not use them elsewhere. 
Commerical use is not allowed.
Please note that the weather informations are taken from the publicly available Hong Kong Observatory
Open Data API (https://www.hko.gov.hk/en/weatherAPI/doc/files/HKO_Open_Data_API_Documentation.pdf) (Version 1.11, Date : Nov, 2023)
It is possible that this software may not be compatiable if the API changed in the future.

本天氣明信片以及圖示均為mahbird基於香港天文台的免費資訊制作及繪畫，於Arduino IDE 2.3.2編寫，並於github免費發佈，僅供個人使用，
轉載需保留原訊息，不得用於其他地方，可按需要自行修改但不得作二次發佈，不得用於商業用途。
本程序基於香港天文台的免費API (https://www.hko.gov.hk/tc/weatherAPI/doc/files/HKO_Open_Data_API_Documentation_tc.pdf) (Version 1.11, Date : Nov, 2023)
如未來API有變更，可能令本程序不相容

專頁 (Github page): https://github.com/mahbird/Weather_Postcard



Special thanks to the developer for the libraries (and the associated libararies in the libraries) used in this project:
(And install them if you haven't)
ArduinoJson by bblanchon: https://github.com/bblanchon/ArduinoJson
GxEPD2 by ZinggJM: https://github.com/ZinggJM/GxEPD2
(and GxEPD2_4G if you are using b/w display: https://github.com/ZinggJM/GxEPD2_4G)
U8g2_for_Adafruit_GFX by olikraus: https://github.com/olikraus/U8g2_for_Adafruit_GFX
Adafruit_GFX by Adafruit: https://github.com/adafruit/Adafruit-GFX-Library
*/

enum alignment {LEFT, RIGHT, CENTER};
enum alignY {TOP, MID, BOTTOM};

#include "JSONParse.h"
#include "warnicon.h"
#include "displayconfig.h"
#include "images.h"
#include "icon_48x48.h"

RTC_DATA_ATTR bool drawerror = 0;


void setup()
{  //Serial.begin(115200);
//delay(100);
//Serial.println("Power On");

//Serial.printf ("Error code:%d - count:%d\n", errorcode, errorcount); 
/*
display.init(0);
display.setFullWindow();
display.firstPage();
do{
  display.fillScreen(white);
}
while (display.nextPage());
display.powerOff();
esp_deep_sleep_start();*/

wifiNTP(); //connect to wifi and sync with NTP
//get information
if (errorcode != 1){
getForecast();
getCurrentWeather();
getVisibility();
getSRS();
getwarning();}
WiFi.disconnect();

if (weathericon < 0){errorcode = 3; weathericon = 0;}


if (errorcode == 0 || drawerror ==0){
drawdisplay();
drawerror = 1;
errorcount = 0;
}
if (errorcode !=0){errorcount++;}
else(drawerror=0);



//deep sleep
//Serial.printf ("Next update in %d minutes\n", sleepminute()); 
delay(500);
esp_deep_sleep(sleepminute() * 6e7); 
}



void loop()
{
}



void drawdisplay()
{   display.init(0); // change to 115200 if you would like to have the display status in serial
  u8g2Fonts.begin(display);
  u8g2Fonts.setFontMode(1);                 
  u8g2Fonts.setFontDirection(0);            
  u8g2Fonts.setForegroundColor(black);         
  u8g2Fonts.setBackgroundColor(white);       



  display.firstPage();
       
    do{
    display.drawBitmap(0,0,gImage_bg_b,400,300, black);
        display.drawBitmap(0,0,gImage_bg_r,400,300, red); // draw background

//drawstampdate
   u8g2Fonts.setFont(datefont); 
char printnowyear[3];
      snprintf(printnowyear, sizeof(printnowyear), "%02d", (nowyear%100));
char printnowmonth[3];
snprintf(printnowmonth, sizeof(printnowmonth), "%02d", nowmonth);
char printnowday[3];
snprintf(printnowday, sizeof(printnowday), "%02d", nowday);
drawString(223,78, printnowday, LEFT, MID, red);
drawString(247,78, printnowmonth, CENTER, MID, red);
drawString(274,78, printnowyear, RIGHT, MID, red);




if (weathericon >10 && weathericon<19){ // Draw night weather icon
display.fillRect(317, 30, 54, 64, black);
display.drawBitmap (320, 38, icon_y[weathericon], 48, 48, yellow);
display.drawBitmap (320, 38, icon_b[weathericon], 48, 48, white);
    }

else{
display.drawRect(317, 30, 54, 64, black); // draw stamp box
display.drawBitmap (320, 38, icon_y[weathericon], 48, 48, yellow);
display.drawBitmap (320, 38, icon_b[weathericon], 48, 48, black);} // Current weather icon




//Draw Current Weather
u8g2Fonts.setFont(smallfont); 

char todayminmaxtmpC[10];
snprintf (todayminmaxtmpC, 10, "%d-%d°C", todaymintmpC, todaymaxtmpC);
drawString(375, 98, todayminmaxtmpC, RIGHT, TOP, black);

if (rainmax > 0){ //rain
char cwrain[14];
snprintf (cwrain, sizeof(cwrain), "%d-%dmm", rainmin, rainmax);
drawString(375, 108, cwrain, RIGHT, TOP, black);
};


//draw warning icon
for (int i=0; i<5; i++)
{   
if (warnicon[0] == "") {  // draw no warning signal
display.drawBitmap(216, 248, gImage_nws_y96x32, 96, 32, yellow);
display.drawBitmap(216, 248, gImage_nws_r96x32, 96, 32, red);
display.drawBitmap(216, 248, gImage_nws_b96x32, 96, 32, black);
  break;}

else if (warnicon[i] == "" && i>0 && i<3) // draw bird in case of the no warnicons
{     display.drawBitmap(216+i*32, 248, gImage_bird_r32x32[i-1], 32, 32, red);
  display.drawBitmap(216+i*32, 248, gImage_bird_y32x32[i-1], 32, 32, yellow);
  display.drawBitmap(216+i*32, 248, gImage_bird_b32x32[i-1], 32, 32, black);
  }

else if (warnicon[i] == "WHOT") {
  display.drawBitmap(216+i*32, 248, gImage_WHOT_r, 32, 32, red); // HOT WEATHER WARNING
  display.drawBitmap(216+i*32, 248, gImage_WHOT_y, 32, 32, yellow);
  }
    else if (warnicon[i] == "WCOLD") {
  display.drawBitmap(216+i*32, 248, gImage_WCOLD_b, 32, 32, black); // COLD WEATHER WARNING
  }
  else if (warnicon[i] == "WRAINA") {
  display.drawBitmap(216+i*32, 248, gImage_WRAINA_y, 32, 32, yellow);
  display.drawBitmap(216+i*32, 248, gImage_WRAINA_b, 32, 32, black); // AMBER RAINSTORM
  }
  else if (warnicon[i] == "WRAINR") {
  display.drawBitmap(216+i*32, 248, gImage_WRAINR_r, 32, 32, red); // RED RAINSTORM
  }
  else if (warnicon[i] == "WRAINB") {
  display.drawBitmap(216+i*32, 248, gImage_WRAINB_b, 32, 32, black); // BLACK RAINSTORM
  }
  else if (warnicon[i] == "WTS") {
  display.drawBitmap(216+i*32, 248, gImage_WTS_b, 32, 32, black); // THUNDERSTORM
  display.drawBitmap(216+i*32, 248, gImage_WTS_y, 32, 32, yellow);}
  else if (warnicon[i] == "TC1") {
  display.drawBitmap(216+i*32, 248, gImage_TC1_b, 32, 32, black); }// TYPHOON NO 1
    else if (warnicon[i] == "TC3") {
  display.drawBitmap(216+i*32, 248, gImage_TC3_b, 32, 32, black); }// TYPHOON NO 3
    else if (warnicon[i] == "TC8NE") {
  display.drawBitmap(216+i*32, 248, gImage_TC8NE_b, 32, 32, black); }// TYPHOON NO 8NE
    else if (warnicon[i] == "TC8SE") {
  display.drawBitmap(216+i*32, 248, gImage_TC8SE_b, 32, 32, black); }// TYPHOON NO 8SE
      else if (warnicon[i] == "TC8NW") {
  display.drawBitmap(216+i*32, 248, gImage_TC8NW_b, 32, 32, black); }// TYPHOON NO 8NW
      else if (warnicon[i] == "TC8SW") {
  display.drawBitmap(216+i*32, 248, gImage_TC8SW_b, 32, 32, black); }// TYPHOON NO 8SW
    else if (warnicon[i] == "TC9") {
  display.drawBitmap(216+i*32, 248, gImage_TC9_b, 32, 32, black); }// TYPHOON NO 9
      else if (warnicon[i] == "TC10") {
  display.drawBitmap(216+i*32, 248, gImage_TC10_b, 32, 32, black); }// TYPHOON NO 10
  else if (warnicon[i] == "WL"){
display.drawBitmap(216+i*32, 248, gImage_WL_b, 32, 32, black); }// Landslide
  else if (warnicon[i] == "WFIREY") {
  display.drawBitmap(216+i*32, 248, gImage_WFIREY_y, 32, 32, yellow);
  display.drawBitmap(216+i*32, 248, gImage_WFIREY_b, 32, 32, black); // Yellow fire
  }
  else if (warnicon[i] == "WFIRER"){
display.drawBitmap(216+i*32, 248, gImage_WFIRER_r, 32, 32, red); }// Red fire

} 


if (warnicon[3]=="" && warnicon[4]==""){ display.drawBitmap (316, 248, gImage_label_64x32, 64, 32, black);} // draw wifi mail label if warnicon less than 4


//line text: (218, 138 + i*22)

u8g2Fonts.setFont(forecastfont);

drawString (218, 138, "Current Weather:", LEFT, BOTTOM, black);

char cwline1[20]; // tmpC +RH
snprintf (cwline1, sizeof(cwline1), "Temp:%d°C  RH:%d%%", CurrentTmpC, CurrentRH);
drawString (218, 138+22, cwline1, LEFT, BOTTOM, black);


char cwline2[20]; //line 2 VIS + UV
snprintf (cwline2, sizeof(cwline2), "UV:%d  Vis:%s", uv, Vis);
drawString (218, 138+44, cwline2, LEFT, BOTTOM, black);

drawString (218, 138+66, "SR:" + sunrise + "  SS:" + sunset, LEFT, BOTTOM, black); // line 3 SRS


// Draw error messages
if (errorcode == 1) {customtext = "No wifi";}
if (errorcode == 2) {customtext = "Fail to get time";}
if (errorcode == 3) {customtext = "Fail to get weather";}

drawString (218, 138+88, customtext, LEFT, BOTTOM, black); // draw custom text

for (int i=0; i<5; i++){ // draw lines after drawing all current information to avoid white text backgrounds
  display.drawRect(216, i*22+140, 160, 1, black);
}

// draw current weather end




//draw forecast

u8g2Fonts.setFont(forecastfont); 

drawString(15, 15, "3 Day Forecast,", LEFT, TOP ,black); // top left text

for (int i = 0; i<3; i++){

drawString (20, i*70+40, forecastDay[i], LEFT, TOP, black);
char fcline2[20];
snprintf(fcline2, sizeof(fcline2), "%d-%d°C,%d-%d%%", forecastminTmpC[i], forecastmaxTmpC[i], forecastminRH[i], forecastmaxRH[i]);
drawString (20, i*70+57, fcline2, LEFT, TOP, black) ; 
drawString (20, i*70+74, "Rain chance:", LEFT, TOP, black) ; 
drawString (20, i*70+91, forecastPSR[i], LEFT, TOP, black) ; 
display.drawBitmap (142, i*70+48, icon_b[forecasticon[i]], 48, 48, black); 
display.drawBitmap (142, i*70+48, icon_y[forecasticon[i]], 48, 48, yellow); 
display.drawFastHLine(20, i*70+107, 170, red);
}



// drawbattery
String batlvl;

#ifndef nobattery
analogReadResolution(12);
int BATmv = (analogReadMilliVolts(batterypin)*2);
if (BATmv >4000)
{batlvl = "Battery Good,";}
else if (BATmv >3800)
{batlvl = "Battery Fair,";}
else {batlvl = "Battery Poor,";}
#else
batlvl = "No Battery,";
#endif

drawString (15, 268, batlvl, LEFT, BOTTOM, black);
String gn;
if (sleepminute()>5+updateinterval*60){gn = ", good night";}
else{gn ="";}
char str_updatetime[20];  // draw updatetime
if (time24h==1){
snprintf (str_updatetime, sizeof(str_updatetime), "%02d:%02d%s", nowhour, nowminute,gn);} // 24 hour
else {
if (nowhour >12){int nowhour12 = nowhour-12;
snprintf (str_updatetime, sizeof(str_updatetime), "%d:%02dpm%s", nowhour12, nowminute,gn);
}
else if (nowhour ==12){
snprintf (str_updatetime, sizeof(str_updatetime), "%d:%02dpm%s", nowhour, nowminute,gn);
}
else {snprintf (str_updatetime, sizeof(str_updatetime), "%d:%02dam%s", nowhour, nowminute,gn);}
}

drawString(15, 285, str_updatetime, LEFT, BOTTOM ,black); 


  }
  while (display.nextPage());
    display.powerOff();
}





// edit: realized I made a mistake here, but editing this will make the current layout a few pixels off, so I am just leaving it this way
void drawString(int x, int y, String text, alignment align, alignY aligny, uint16_t color) { 
    int16_t  x1, y1; 
  uint16_t w, h;
  u8g2Fonts.setForegroundColor(color);
  display.setTextWrap(false);
  display.getTextBounds(text, x, y, &x1, &y1, &w, &h);
  w = u8g2Fonts.getUTF8Width(text.c_str()); //Use U8G2Fonts Width
  if (align == RIGHT)  {x = x - w;}
  if (align == CENTER) {x = x - w / 2;}
 if (aligny == TOP) {y = y + h+h/2;}
  if (aligny == MID) {y = y + h/2;}
  u8g2Fonts.setCursor(x, y);
  u8g2Fonts.print(text);}




// Deep Sleep calculation
int sleepminute() {
int sleeptime;
if (errorcode != 0) {
  if (errorcount < 6){return 5;} //
else {return 120;}
} // retry in 5 mins if errorcount <6, 120 mins otherwise
if (sleephour > wakehour)
  {
  if (nowhour >= sleephour && nowhour < 24) 
     { sleeptime = (24-nowhour+wakehour)*60 - nowminute;
      }
  else if (nowhour < wakehour) {sleeptime = (wakehour - nowhour)*60 - nowminute;
  }
  else{sleeptime = updateinterval*60 - nowminute;
  } 
  }
else if (sleephour < wakehour)
{if (nowhour >= sleephour && nowhour < wakehour)
{sleeptime = (wakehour - nowhour)*60 - nowminute;
}
else{sleeptime = updateinterval*60 - nowminute;
} 
}
else{sleeptime = updateinterval*60 - nowminute;
} 

if (sleeptime < 5){sleeptime =+ updateinterval*60;}// if next update time is scheduled in less than 5 mins, skip til next interval to prevent too frequent update

return sleeptime;
}

