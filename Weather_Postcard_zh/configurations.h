// Please change your configurations in configurations.cpp 

#ifndef configurations_h
#define configurations_h

#include <Arduino.h>
#include <Preferences.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <Arduino.h>
#include <ArduinoJson.h>
#include "HKOWeatherJSON.h"
#include "displayconfig.h"


extern String ssid;
extern String pass;
extern int updateinterval;
extern int sleephour;
extern int wakehour;
extern String rainlocation;
extern String tmpClocation;
extern String customtext;
extern bool time24h;
extern bool usectext; 
extern language displaylanguage;

#define timezone 8
#define daysavetime 0




#endif