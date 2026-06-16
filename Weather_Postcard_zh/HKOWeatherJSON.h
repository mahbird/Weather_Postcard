#ifndef HKOWeatherJSON_h
#define HKOWeatherJSON_h

#include "configurations.h"

#include <Arduino.h>
#include <Preferences.h>
//#include <iostream>
//#include <map>


extern Preferences pref;
extern int nowday, nowmonth, nowyear, nowhour, nowminute;
extern int errorcode;
extern int errorcount;


enum warncode
{
    NOWARNING,
    WFIREY,
    WFIRER,
    WFROST,
    WHOT,
    WCOLD,
    WMSGNL,
    WRAINA,
    WRAINR,
    WRAINB,
    WFNTSA,
    WL,
    TC1,
    TC3,
    TC8NE,
    TC8SE,
    TC8NW,
    TC8SW,
    TC9,
    TC10,
    WTMW,
    WTS
};

// Current weather variables

extern int rainmax;
extern int rainmin;
extern int uv;
extern int CurrentTmpC;
extern int CurrentRH;
extern int todayrainchance;
extern int todaymintmpC, todaymaxtmpC;
extern int todayminRH, todaymaxRH;
extern int AQI;
extern String Vis;
extern String sunrise;
extern String sunset;
extern warncode warnicon[5];
extern int nowweathericon;
extern String todaydescription;
extern String todayAQI;
extern int todayAQicon;

// forecast variables
extern String forecastDay[3];
extern int forecastmaxTmpC[3];
extern int forecastminTmpC[3];
extern int forecastmaxRH[3];
extern int forecastminRH[3];
extern int forecastPSR[3];
extern int forecasticon[3];
extern String forecastdescription[3];



extern int nowdaycheck;

extern bool updateSRS;

enum language
{
    ZH,
    EN
};

// trim weathertext
void trimweatherZH(String &inputtext, int numofchar = 8); //chinese character limit
void trimweatherEN(String &inputtext, int numofchar = 15);


void getForecast(language lang);
void getCurrentWeather();
void getVisibility();
void getSRS();

void gettodaydescription(language lang);
int PSRtoInt(String PSRinput, language lang);
String PSRtoString(int PSRInt);
void getwarning();
warncode warningtoenum(const char* warninginput);



#endif