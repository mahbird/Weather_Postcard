#include <Arduino.h>
#include "configurations.h"

// Please don't change anything above


// These below needs changing
// Please also change the settings in "displayconfig.h"

String ssid = "";       // replace with your SSID, works with 2.4GHz only
String pass = "";       // replace with your password
int updateinterval = 1; // default update interval = 1 hour
int sleephour = 14;     // the hour of the last update, if previous update time + interval > sleep hour, it will still update one more time at interval before sleeping.
int wakehour = 6;       //  the hour which the update starts, set both to same value to disable sleep
// please keep wakehour before 10am to allow correct display for the min and max temperture of the current day
String rainlocation = "Kowloon City";          // Rainfall location, see "locations.h" for list
String tmpClocation = "Hong Kong Observatory"; // Temperature location, see "locations.h" for list
String customtext = "Have a nice day";         // custom text at bottom, can't be too long or it will overflow
bool usectext = false; // set to true to display customtext above, false to display weather description
language displaylanguage = ZH;  // Display language: ZH for Traditional Chinese, EN for English
bool time24h = 0;                              // 1 for display in 24hour time and 0 for am/pm update time







