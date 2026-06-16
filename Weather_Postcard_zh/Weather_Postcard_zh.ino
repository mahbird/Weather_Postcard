/*
Release Note (June 2026): (Chinese version and misc updates)

!!!!!!!!IMPORTANT!!!!!!!!!!!
Before you update your existing Weather Postcard, back up all your existing configurations and especially your displayconfig.h
configurations are now in configurations.cpp instead of configurations.h
!!!!!!!!!!!!!!!!!!!!!!!!!!!!


New features and changes:

- Now supports Chinese or English display - just choose your desired display language in configurations.cpp
Chinese font used is Google Sans Serif, converted to .bdf with FontForge and fine tuned by me. 
Note that due to limited memory, the font is incompleted and only has common words and is not intended for custom text in Chinese. However, you can convert your own fonts, I have a guide at:
https://github.com/mahbird/u8g2_chinese_convert_tool_py
- Forecast description - simple forecast description is shown below PSR (Possibility of significant rain)
- Custom text section now supports current weather description - ability to show current weather description in place of custom text
- If rain chance for today is higher than 30%, it would be shown under the min-max temperature today (under the stamp)
- Added extra warning icons: The only omitted warning icons now are: Frost,  Flooding in the Northern New Territories and Tsunami (these are too rare and don't concern most people) 
- Tested working on Arduino IDE 2.3.10

** Please update your settings in configurations.cpp (instead of configurations.h) and displaycong.h **
** Depending on your board, please choose a partition scheme that has 1.5MB or more for the app **


*****

Author's Comment:
Weather Postcard was one of my starter projects, since then I have been learning and working on other projects on and off. 
Since I started this while I was relatively new to coding, structure-wise these could use better organizing and many functions could probably be done in a better way if I rewrite this now.
If you think there are things that look odd (but should still be working), they probably are odd but I just leave them as is.

*****

Original release note:
The Weather postcard and all icons are created and drawn by mahbird on Github, written in Arduino IDE 2.3.6, based on information available from the Hong Kong Observatory.
The code is distributed for free on github. For personal use only. Please retain all original messages when posting on other sites.
You are free to modify this code as you like, but do not redistribute modified code and do not use them elsewhere.
Commerical use is not allowed.
Please note that the weather informations are taken from the publicly available Hong Kong Observatory
Open Data API (https://www.hko.gov.hk/en/weatherAPI/doc/files/HKO_Open_Data_API_Documentation.pdf) (Version 1.12, Date : Nov, 2024)
It it possible that this software may not be compatiable if the API changed in the future.

本天氣明信片以及圖示均為mahbird基於香港天文台的免費資訊制作及繪畫，於Arduino IDE 2.3.6編寫，並於github免費發佈，僅供個人使用，
轉載需保留原訊息，不得用於其他地方，可按需要自行修改但不得作二次發佈，不得用於商業用途。
本程序基於香港天文台的免費API (https://www.hko.gov.hk/tc/weatherAPI/doc/files/HKO_Open_Data_API_Documentation_tc.pdf) (Version 1.12, Date : Nov, 2024)
如未來API有變更，可能令本程序不相容


專頁 (Github page): https://github.com/mahbird/Weather_Postcard


Special thanks to the developer for the libraries (and the associated libararies in the libraries) used in this project:
(And install them if you haven't, if you encounter problems while compiling, try the versions on or before April, 2025)
ArduinoJson by bblanchon: https://github.com/bblanchon/ArduinoJson
GxEPD2 by ZinggJM: https://github.com/ZinggJM/GxEPD2
(and GxEPD2_4G if you are using b/w display: https://github.com/ZinggJM/GxEPD2_4G)
u8g2 fonts by olikraus: https://github.com/olikraus/u8g2
Adafruit_GFX by Adafruit: https://github.com/adafruit/Adafruit-GFX-Library
*/

#include <Arduino.h>
#include "configurations.h"
#include "HKOWeatherJSON.h"
#include "displayconfig.h"
#include "assets/warnicon.h"
#include "assets/images.h"
#include "assets/icon_48x48.h"


RTC_DATA_ATTR bool drawerror = 0;
RTC_DATA_ATTR static bool lowbatscreendrawn = false;
int BATmv;
static int nextupdateminute = 5;

enum alignX
{
  LEFT,
  RIGHT,
  CENTER
};
enum alignY
{
  TOP,
  MID,
  BOTTOM
};

U8G2_FOR_ADAFRUIT_GFX u8g2Fonts;
GxEPD2_DISPLAY_CLASS<GxEPD2_DRIVER_CLASS, MAX_HEIGHT(GxEPD2_DRIVER_CLASS)> display(GxEPD2_DRIVER_CLASS(CS, DC, RST, BUSY));

void drawdisplay();
void drawString(int x, int y, String text, alignX alignx, alignY aligny, uint16_t color);
int sleepminute();



void wifiConnect()
{
  WiFi.begin(ssid, pass);
  unsigned long startcount = millis();
  // Serial.print ("Connecting to Wifi");
  bool buttonstate = 0;
  while (WiFi.status() != WL_CONNECTED)
  { // Serial.print (".");
    delay(500);
    if (millis() - startcount > 15000)
    {
      // Serial.print("\nError 1 - Fail to connect to Wifi\n");
      errorcode = 1;
      // Serial.println("\nFail to connect to Wifi");
      delay(100);
      return;
    }
  }
  //  Serial.println("\nWifi Connected");
  errorcode = 0;
}

void wifiNTP()
{
  errorcode = 0; // indicates it successfully get to this part
  // Serial.println("Contacting NTP");
  const char *ntpServers[] = {"time.google.com", "stdtime.gov.hk", "time.nist.gov", "pool.ntp.org"};
  for (const char *server : ntpServers)
  {
    configTime(3600 * timezone, daysavetime * 3600, server);
    struct tm tmstruct;
    if (getLocalTime(&tmstruct, 5000))
    {
      nowyear = (tmstruct.tm_year) + 1900;
      nowmonth = (tmstruct.tm_mon) + 1;
      nowday = tmstruct.tm_mday;
      nowhour = tmstruct.tm_hour;
      nowminute = tmstruct.tm_min;
      // nowsecond = tmstruct.tm_sec;
      // nowwday = tmstruct.tm_wday;
      // snprintf (srsrq, sizeof(srsrq), "SRS&lang=en&rformat=json&year=%d&month=%d&day=%d", nowyear, nowmonth, nowday); //moved to getSRS()

      //
      if (nowyear < 2025 || nowyear > 2100) // Check if year is valid, don't think anyone will be using this in 2100 lol
      // if invalid, try next server
      {                                         // Serial.printf("Fail - %s return incorrect time: %02d:%02d - %d/%d/%d\n", server, nowhour, nowminute, nowday, nowmonth, nowyear );
        memset(&tmstruct, 0, sizeof(tmstruct)); // clears tmstruct
        delay(500);
        errorcode = 2;
        continue;
      }

      // Serial.printf("NTP at %s success. Now is %02d:%02d - %d/%d/%d\n", server, nowhour, nowminute, nowday, nowmonth, nowyear);
      errorcode = 0;
      return;
    }
    // Serial.println("Fail to get time");
    errorcode = 2;
  }
}

int voltagetolevel(int voltage)
{
  // thresholds in descending order, if these doesn't work well for your battery change accordingly
  int thresholds[] = {3850, 3700, 3450};
  int levels[] = {3, 2, 1};

  for (int i = 0; i < 3; i++)
  {
    if (voltage > thresholds[i])
    {
      return levels[i];
    }
  }
  return 0; // fallback
}

// End of misc functions


void setup()
{
  // Serial.begin(115200);
  //  Serial.println("Power On");
  setCpuFrequencyMhz(80); // set to a lower frequency can reduce power consumption depending on your board, but of course a slightly lower speeed

  //  Serial.printf("Error code:%d - count:%d\n", errorcode, errorcount);

  // Low battery check
  analogReadResolution(12);
  BATmv = (analogReadMilliVolts(batterypin) * 2);
  //  Serial.printf("Battery voltage: %d mV\n", BATmv);
  if (BATmv < 3400)
  {
   // Serial.printf("Battery voltage: %d mV - Critically low \n", BATmv); 
    errorcode = 4;
    if (!lowbatscreendrawn)
    {
      drawdisplay();
      lowbatscreendrawn = true;
    }
    esp_deep_sleep(30 * 6e7); // Does not update if battery is critically low, check back again every half an hour for battery voltage
  }

  lowbatscreendrawn = false;

  //delay(500); // ** slight delay for power to stabilize, since sometimes the wifi fail to connect if called too soon after wake up, might not be needed depending on board/ power source
  wifiConnect();
  if (errorcode != 1)
  {
    wifiNTP();
    // get information
    if (errorcode == 2)
    {
      wifiNTP();
    } // retry wifiNTP in case
  }

  if (errorcode == 0)
  {
    getForecast(displaylanguage);
    getCurrentWeather();
    if (!usectext)
    {
      gettodaydescription(displaylanguage);
    } // new for weather description to replace customtext
    getVisibility();
    getSRS();
    getwarning();
  }
  WiFi.disconnect();
  WiFi.mode(WIFI_OFF);

  setCpuFrequencyMhz(40);

  if (nowweathericon < 0)
  {
    errorcode = 3;
    nowweathericon = 0;
  }

  nextupdateminute = sleepminute();
  if (errorcode == 0)
  {
    
    drawdisplay();
    drawerror = false;
    errorcount = 0;
  }

  if (errorcode != 0)
  {
    errorcount++;

    if (errorcount == 3) // draw error message on 3rd retry
    {
      
      drawdisplay();
      drawerror = true;
    }
  }


  // Serial.printf("Error code:%d - count:%d\n", errorcode, errorcount);
  // Serial.printf ("Next update in %d minutes\n", nextupdateminute);

  esp_deep_sleep(nextupdateminute * 6e7);
}

void loop()
{
}

void drawdisplay()
{ // display.init(115200); // change to 115200 if you would like to have the display status in serial

  if (nowhour == wakehour)
  {
    display.init(115200, true);
  }
  else
  {
    display.init(115200, false);
  }

  u8g2Fonts.begin(display);
  u8g2Fonts.setFontMode(1);
  u8g2Fonts.setFontDirection(0);
  u8g2Fonts.setForegroundColor(black);
  u8g2Fonts.setBackgroundColor(white);

  display.firstPage();

  do
  {
    display.drawBitmap(0, 0, gImage_bg_b, 400, 300, black);
    display.drawBitmap(0, 0, gImage_bg_r, 400, 300, red); // draw background

    // drawstampdate
    u8g2Fonts.setFont(datefont);
    char printnowyear[3];
    snprintf(printnowyear, sizeof(printnowyear), "%02d", (nowyear % 100));
    char printnowmonth[3];
    snprintf(printnowmonth, sizeof(printnowmonth), "%02d", nowmonth);
    char printnowday[3];
    snprintf(printnowday, sizeof(printnowday), "%02d", nowday);
    drawString(223, 78, printnowday, LEFT, MID, red);
    drawString(248, 78, printnowmonth, CENTER, MID, red);
    drawString(274, 78, printnowyear, RIGHT, MID, red);

    if (nowweathericon > 10 && nowweathericon < 19)
    { // Draw night weather icon
      display.fillRect(317, 30, 54, 64, black);
      display.drawBitmap(320, 38, icon_y[nowweathericon], 48, 48, yellow);
      display.drawBitmap(320, 38, icon_b[nowweathericon], 48, 48, white);
    }

    else
    {
      display.drawRect(317, 30, 54, 64, black); // draw stamp box
      display.drawBitmap(320, 38, icon_y[nowweathericon], 48, 48, yellow);
      display.drawBitmap(320, 38, icon_b[nowweathericon], 48, 48, black);
    } // Current weather icon

    // Draw Current Weather
    u8g2Fonts.setFont(smallfont);

    char todayminmaxtmpC[10];
    snprintf(todayminmaxtmpC, 10, "%d-%d°C", todaymintmpC, todaymaxtmpC);
    drawString(375, 98 + 5, todayminmaxtmpC, RIGHT, TOP, black);
    if (rainmax > 0)
    { // rain
      char cwrain[14];
      snprintf(cwrain, sizeof(cwrain), "%d-%dmm", rainmin, rainmax);
      drawString(375, 108 + 5, cwrain, RIGHT, TOP, black);
    }

    else if (todayrainchance > 0) // *Show today rain chance if not raining and if rain chance > 0
    {
      drawString(375, 108 + 5, PSRtoString(todayrainchance), RIGHT, TOP, black);
    }

    // draw warning icon

    for (int i = 0; i < 5; i++)
    {
      if (warnicon[0] == NOWARNING)
      { // draw no warning signal
        display.drawBitmap(216, 248, gImage_nws_y96x32, 96, 32, yellow);
        display.drawBitmap(216, 248, gImage_nws_r96x32, 96, 32, red);
        display.drawBitmap(216, 248, gImage_nws_b96x32, 96, 32, black);
        break;
      }

      else if (warnicon[i] == NOWARNING && i > 0 && i < 3) // draw bird in case of the no warnicons
      {
        display.drawBitmap(216 + i * 32, 248, gImage_bird_r32x32[i - 1], 32, 32, red);
        display.drawBitmap(216 + i * 32, 248, gImage_bird_y32x32[i - 1], 32, 32, yellow);
        display.drawBitmap(216 + i * 32, 248, gImage_bird_b32x32[i - 1], 32, 32, black);
      }

      else if (warnicon[i] == WHOT)
      {
        display.drawBitmap(216 + i * 32, 248, gImage_WHOT_r, 32, 32, red); // HOT WEATHER WARNING
        display.drawBitmap(216 + i * 32, 248, gImage_WHOT_y, 32, 32, yellow);
      }
      else if (warnicon[i] == WCOLD)
      {
        display.drawBitmap(216 + i * 32, 248, gImage_WCOLD_b, 32, 32, black); // COLD WEATHER WARNING
      }
      else if (warnicon[i] == WRAINA)
      {
        display.drawBitmap(216 + i * 32, 248, gImage_WRAINA_y, 32, 32, yellow);
        display.drawBitmap(216 + i * 32, 248, gImage_WRAINA_b, 32, 32, black); // AMBER RAINSTORM
      }
      else if (warnicon[i] == WRAINR)
      {
        display.drawBitmap(216 + i * 32, 248, gImage_WRAINR_r, 32, 32, red); // RED RAINSTORM
      }
      else if (warnicon[i] == WRAINB)
      {
        display.drawBitmap(216 + i * 32, 248, gImage_WRAINB_b, 32, 32, black); // BLACK RAINSTORM
      }
      else if (warnicon[i] == WTS)
      {
        display.drawBitmap(216 + i * 32, 248, gImage_WTS_b, 32, 32, black); // THUNDERSTORM
        display.drawBitmap(216 + i * 32, 248, gImage_WTS_y, 32, 32, yellow);
      }
      else if (warnicon[i] == TC1)
      {
        display.drawBitmap(216 + i * 32, 248, gImage_TC1_b, 32, 32, black);
      } // TYPHOON NO 1
      else if (warnicon[i] == TC3)
      {
        display.drawBitmap(216 + i * 32, 248, gImage_TC3_b, 32, 32, black);
      } // TYPHOON NO 3
      else if (warnicon[i] == TC8NE)
      {
        display.drawBitmap(216 + i * 32, 248, gImage_TC8NE_b, 32, 32, black);
      } // TYPHOON NO 8NE
      else if (warnicon[i] == TC8SE)
      {
        display.drawBitmap(216 + i * 32, 248, gImage_TC8SE_b, 32, 32, black);
      } // TYPHOON NO 8SE
      else if (warnicon[i] == TC8NW)
      {
        display.drawBitmap(216 + i * 32, 248, gImage_TC8NW_b, 32, 32, black);
      } // TYPHOON NO 8NW
      else if (warnicon[i] == TC8SW)
      {
        display.drawBitmap(216 + i * 32, 248, gImage_TC8SW_b, 32, 32, black);
      } // TYPHOON NO 8SW
      else if (warnicon[i] == TC9)
      {
        display.drawBitmap(216 + i * 32, 248, gImage_TC9_b, 32, 32, black);
      } // TYPHOON NO 9
      else if (warnicon[i] == TC10)
      {
        display.drawBitmap(216 + i * 32, 248, gImage_TC10_b, 32, 32, black);
      } // TYPHOON NO 10
      else if (warnicon[i] == WL)
      {
        display.drawBitmap(216 + i * 32, 248, gImage_WL_b, 32, 32, black);
      } // Landslide
      else if (warnicon[i] == WFIREY)
      {
        display.drawBitmap(216 + i * 32, 248, gImage_WFIREY_y, 32, 32, yellow);
        display.drawBitmap(216 + i * 32, 248, gImage_WFIREY_b, 32, 32, black); // Yellow fire
      }
      else if (warnicon[i] == WFIRER)
      {
        display.drawBitmap(216 + i * 32, 248, gImage_WFIRER_r, 32, 32, red);
      } // Red fire
	  else if (warnicon[i] == WMSGNL)
      {
        display.drawBitmap(216 + i * 32, 248, gImage_WMSGNL_b, 32, 32, black);
      } // Monsoon
    }

    if (warnicon[3] == NOWARNING && warnicon[4] == NOWARNING)
    {
      display.drawBitmap(316, 248, gImage_label_64x32, 64, 32, black);
    } // draw wifi mail label if warnicon less than 4

    // line text: (216, 138 + i*22)

    // < ----- Current weather starts

    int linespacing = 22;
    int cwx = 214; // current weather x pos
    u8g2Fonts.setFont(displaylanguage == ZH ? chinesefont : forecastfont);
    if (displaylanguage == ZH)
    {
      drawString(cwx, 138, "當前天氣:", LEFT, BOTTOM, black);
      char cwline1[40];
      snprintf(cwline1, sizeof(cwline1), "溫度:%d°C  濕度:%d%%", CurrentTmpC, CurrentRH);
      drawString(cwx, 138 + linespacing, cwline1, LEFT, BOTTOM, black);

      char cwline2[40]; // line 2 VIS + UV
      snprintf(cwline2, sizeof(cwline2), "UV:%d  能見度:%s", uv, Vis);
      drawString(cwx, 138 + linespacing * 2, cwline2, LEFT, BOTTOM, black);
      drawString(cwx, 138 + linespacing * 3, "日出:" + sunrise + "  日落:" + sunset, LEFT, BOTTOM, black); // line 3 SRS

      if (errorcode > 0)
      {
        String errorstr[4] = {"無法連線", "無法獲取時間", "無法獲取天氣", "電量不足，請充電"};
        customtext = errorstr[errorcode - 1];
        drawString(cwx, 138 + linespacing * 4, customtext, LEFT, BOTTOM, black);
      }

      else if (usectext)
      {
        u8g2Fonts.setFont(forecastfont); // No chinese customtext
        drawString(cwx, 138 + linespacing * 4, customtext, LEFT, BOTTOM, black);
        u8g2Fonts.setFont(chinesefont); // switch back to chinese font for other text
      } // draw custom text
      else // not using custom text, draw description
      {
        drawString(cwx, 138 + linespacing * 4, todaydescription, LEFT, BOTTOM, black);
      } // draw weather description if no custom text

      for (int i = 0; i < 5; i++)
      {                                                                    // draw lines after drawing all current information to avoid white text backgrounds
        display.drawFastHLine(cwx - 2, i * linespacing + 140, 162, black); // lines slightly different for zh (longer by 2 px to the left)
      }
    }

    // English version
    else
    {
      cwx = 216; // (test, original 218), slightly different from chinese version
      drawString(cwx, 138, "Current Weather:", LEFT, BOTTOM, black);
      char cwline1[40]; // tmpC +RH
      snprintf(cwline1, sizeof(cwline1), "Temp:%d°C  RH:%d%%", CurrentTmpC, CurrentRH);
      drawString(cwx, 138 + linespacing, cwline1, LEFT, BOTTOM, black);
      char cwline2[40]; // line 2 VIS + UV
      snprintf(cwline2, sizeof(cwline2), "UV:%d  Vis:%s", uv, Vis);
      drawString(cwx, 138 + linespacing * 2, cwline2, LEFT, BOTTOM, black);
      drawString(cwx, 138 + linespacing * 3, "SR:" + sunrise + "  SS:" + sunset, LEFT, BOTTOM, black); // line 3 SRS

      // Draw error messages
      if (errorcode > 0)
      {
        String errorstr[4] = {"No wifi", "Fail to get time", "Fail to get weather", "Please recharge"};
        customtext = errorstr[errorcode - 1];
      }

      else if (usectext)
      {

        drawString(cwx, 138 + linespacing * 4, customtext, LEFT, BOTTOM, black);
      } // draw custom text
      else
      {
        drawString(cwx, 138 + linespacing * 4, todaydescription, LEFT, BOTTOM, black);
      } // draw weather description if no custom text

      for (int i = 0; i < 5; i++)
      { // draw lines after drawing all current information to avoid white text backgrounds

        display.drawFastHLine(cwx - 2, i * linespacing + 140, 162, black);
      }
    }
    // draw current weather end ----->

    // <------- draw forecast

    u8g2Fonts.setFont(displaylanguage == ZH ? chinesefont : forecastfont);
    // u8g2Fonts.setFont(chinesefont);
    // u8g2Fonts.setFont(forecastfont);

    if (displaylanguage == ZH)
    {
      drawString(15, 21, "天氣預報,", LEFT, TOP, black); // top left text
    }
    else
    {
      drawString(15, 18, "3 Day Forecast,", LEFT, TOP, black); // top left text
    }

    for (int i = 0; i < 3; i++)
    {

      if (displaylanguage == ZH)
      {
        int fclinespacing = 18;
        int boxheight = 71;
        drawString(20, i * boxheight + 45, forecastDay[i], LEFT, TOP, black);
        char fcline2[20];
        u8g2Fonts.setFont(forecastfont);
        snprintf(fcline2, sizeof(fcline2), "%d-%d°C,%d-%d%%", forecastminTmpC[i], forecastmaxTmpC[i], forecastminRH[i], forecastmaxRH[i]);
        drawString(20, i * boxheight + 60, fcline2, LEFT, TOP, black);
        // drawString(20, i * 71 + 57 + 3, fcline2, LEFT, TOP, black);
        u8g2Fonts.setFont(chinesefont);
        drawString(20, i * boxheight + 60 + fclinespacing-2, "顯著降雨:", LEFT, TOP, black);
        // drawString(20, i * 71 + 74 + 2, "顯著降雨:", LEFT, TOP, black);
        u8g2Fonts.setFont(forecastfont);
        u8g2Fonts.print(PSRtoString(forecastPSR[i]));
        u8g2Fonts.setFont(chinesefont);
        drawString(20, i * boxheight + 60 + fclinespacing * 2 -1, forecastdescription[i], LEFT, TOP, black);
        // drawString(20, i * 71 + 91 + 3, forecastdescription[i], LEFT, TOP, black);
        display.drawBitmap(142, i * boxheight + 46, icon_b[forecasticon[i]], 48, 48, black);
        display.drawBitmap(142, i * boxheight + 46, icon_y[forecasticon[i]], 48, 48, yellow);
        display.drawFastHLine(20, i * boxheight + 107, 170, red);
      }
      else // en display
      {
        int fclinespacing = 17;
        int boxheight = 70;

        drawString(20, i * boxheight + 45, forecastDay[i], LEFT, TOP, black);
        char fcline2[20];
        snprintf(fcline2, sizeof(fcline2), "%d-%d°C,%d-%d%%", forecastminTmpC[i], forecastmaxTmpC[i], forecastminRH[i], forecastmaxRH[i]);
        drawString(20, i * boxheight + 62, fcline2, LEFT, TOP, black);
        drawString(20, i * boxheight + 62 + fclinespacing, "Rain:" + PSRtoString(forecastPSR[i]), LEFT, TOP, black);
        drawString(20, i * boxheight + 62 + fclinespacing * 2, forecastdescription[i], LEFT, TOP, black);
        display.drawBitmap(142, i * boxheight + 48, icon_b[forecasticon[i]], 48, 48, black);
        display.drawBitmap(142, i * boxheight + 48, icon_y[forecasticon[i]], 48, 48, yellow);
        display.drawFastHLine(20, i * boxheight + 107, 170, red);
      }
    }
    // drawbattery
    String batlvl;

#ifndef nobattery
    int batlevel = voltagetolevel(BATmv);
    if (displaylanguage == ZH)
    {
      String batmsg[4] = {"電量不足", "電量偏低", "電量一般,", "電量充足,"};
      batlvl = batmsg[batlevel];
    }
    else
    {
      String batmsg[4] = {"Please recharge,", "Battery Low,", "Battery Fair,", "Battery Good,"};
      batlvl = batmsg[batlevel];
    }

#else
    batlvl = "Last updated,";

#endif
    //  Serial.println(batlvl);
    drawString(15, 268, batlvl, LEFT, BOTTOM, black);
    String gn;
    if (nextupdateminute > 5 + updateinterval * 60)
    {
      if (displaylanguage == ZH)
      {
        gn = ", 晚安";
      }
      else
      {
        gn = ", good night";
      }
    }

    else
    {
      gn = "";
    }
    char str_updatetime[20]; // draw updatetime
    if (time24h == 1)
    {
      snprintf(str_updatetime, sizeof(str_updatetime), "%02d:%02d%s", nowhour, nowminute, gn);
    } // 24 hour
    else
    {
      if (nowhour > 12)
      {
        int nowhour12 = nowhour - 12;
        snprintf(str_updatetime, sizeof(str_updatetime), "%d:%02dpm%s", nowhour12, nowminute, gn);
      }
      else if (nowhour == 12)
      {
        snprintf(str_updatetime, sizeof(str_updatetime), "%d:%02dpm%s", nowhour, nowminute, gn);
      }
      else
      {
        snprintf(str_updatetime, sizeof(str_updatetime), "%d:%02dam%s", nowhour, nowminute, gn);
      }
    }

    drawString(15, 285, str_updatetime, LEFT, BOTTOM, black);

    // end forecast ------>
    setCpuFrequencyMhz(10); //**test reduce power */

  } while (display.nextPage());
  display.powerOff();
}

void drawString(int x, int y, String text, alignX alignx, alignY aligny, uint16_t color)
{
  int16_t x1, y1;
  uint16_t w, h;
  u8g2Fonts.setForegroundColor(color);
  h = u8g2Fonts.getFontAscent() + u8g2Fonts.getFontDescent(); // this seem to work even though I don't know why it is + and not -
  w = u8g2Fonts.getUTF8Width(text.c_str());
  if (alignx == RIGHT)
  {
    x -= w;
  }
  if (alignx == CENTER)
  {
    x -= w / 2;
  }
  if (aligny == TOP)
  {
    y += h;
  }
  if (aligny == MID)
  {
    y += h / 2;
  }
  u8g2Fonts.setCursor(x, y);
  u8g2Fonts.print(text);
}

// Deep Sleep calculation (Actually this is not a very good way to write, but just leave it since it works)
int sleepminute()
{
  int sleeptime;
  if (errorcode != 0)
  {
    if (errorcount < 3)
    {
      return 1;
    } //
    //  else if (errorcount < 5)
    //{ return 5; }
    else
    {
      return (updateinterval * 120);
    }
  } // /*retry in 5 mins if errorcount <6, /* update to just retry in 120 mins otherwise

  if (sleephour == wakehour)
  {
    return (updateinterval * 60 - nowminute + 5);
  } // if sleephour == wakehour, no sleep
  if (sleephour > wakehour)
  {
    if (nowhour >= sleephour && nowhour < 24)
    {
      sleeptime = (24 - nowhour + wakehour) * 60 - nowminute + 5;
    }
    else if (nowhour < wakehour)
    {
      sleeptime = (wakehour - nowhour) * 60 - nowminute + 5;
    }
    else
    {
      sleeptime = updateinterval * 60 - nowminute + 5;
    }
  }
  else if (sleephour < wakehour)
  {
    if (nowhour >= sleephour && nowhour < wakehour)
    {
      sleeptime = (wakehour - nowhour) * 60 - nowminute + 5;
    }
    else
    {
      sleeptime = updateinterval * 60 - nowminute + 5;
    }
  }
  else
  {
    sleeptime = updateinterval * 60 - nowminute + 5;
  }

  if (sleeptime < 5)
  {
    sleeptime = +updateinterval * 60 + 5;
  } // if next update time is scheduled in less than 5 mins, skip til next interval to prevent too frequent update

  return sleeptime;
}
