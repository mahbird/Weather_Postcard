//Default is for 4-color panel and for the firebeetle ESP32C6, if you are using B&W panel or other boards, please also change the settings in "displayconfig.h"

const char* ssid = "yourSSID"; // replace with your SSID, works with 2.4GHz only
const char* pass = "yourwifipassword; // replace with your password

int updateinterval = 1; // default update interval = 1 hour
int sleephour = 16;// the hour of the last update, if previous update time + interval > sleep hour, it will still update one more time at interval before sleeping.
int wakehour = 7;//  the hour which the update starts, set both to same value to disable sleep
//please keep it before 10am to allow correct display for the min and max temperture of the current day
String rainlocation = "Kowloon City"; // Rainfall location, see "locations.h" for list
String tmpClocation = "Hong Kong Observatory"; // Temperature location, see "locations.h" for list
String customtext = "Have a nice day"; // custom text at bottom, can't be too long or it will overflow
bool time24h = 0;// 1 for display in 24hour time and 0 for am/pm update time

// #define nobattery // uncomment this if you are not using battery
#define batterypin 0 // change if you are using another board or not using battery


//The following should not need changing since this calendar is specific to Hong Kong and no point using it elsewhere
#define timezone 8
#define daysavetime 0
