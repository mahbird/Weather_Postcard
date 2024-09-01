#include <WiFi.h>
#include <HTTPClient.h>
#include "configurations.h"
#include <ArduinoJson.h>
#include <Preferences.h>


Preferences pref;
HTTPClient http;

RTC_DATA_ATTR int errorcode = 0; // 
RTC_DATA_ATTR int errorcount = 0;


const String baseURL = "https://data.weather.gov.hk/weatherAPI/opendata/weather.php?dataType="; // HKO weather
const String cwrq = "rhrread&lang=en"; // CurrentWeather request
const String fcrq= "fnd&lang=en"; // Forcast request
const String baseURL2 = "https://data.weather.gov.hk/weatherAPI/opendata/opendata.php?dataType="; // HKO OpenWeather
const String vbrq = "LTMV&lang=en&rformat=json"; // visibility requestH
const String warnrq ="warnsum&lang=en"; // current weather warning
char srsrq [60];


int nowdayint;
int nowdaycheck = 0;
int todaymintmpC;
int todaymaxtmpC;


String nowday, nowmonth, nowyear;
int nowhour, nowminute;



#define forecastdays 3

//String forecastdate[3]; //forecast variables
String forecastDay[3];
int forecastmaxTmpC[3];
int forecastminTmpC[3];
int forecastmaxRH[3];
int forecastminRH[3];
String forecastPSR[3];
int forecasticon[3];


 // Current weather variables
int weathericon;
int uv, rainmax, rainmin, CurrentTmpC, CurrentRH;

//other variables
String Vis;
String sunrise;
String sunset;
String warnicon[5];


void getForecast(){
HTTPClient http;
http.begin (baseURL+fcrq);
int httpCode = http.GET();

if (httpCode > 0) {
  String result = http.getString();

JsonDocument filter;

JsonObject filter_weatherForecast_0 = filter["weatherForecast"].add<JsonObject>();
filter_weatherForecast_0["forecastDate"] = true;
filter_weatherForecast_0["week"] = true;
filter_weatherForecast_0["forecastMaxtemp"]["value"] = true;
filter_weatherForecast_0["forecastMintemp"]["value"] = true;
filter_weatherForecast_0["forecastMaxrh"]["value"] = true;
filter_weatherForecast_0["forecastMinrh"]["value"] = true;
filter_weatherForecast_0["ForecastIcon"] = true;
filter_weatherForecast_0["PSR"] = true;

JsonDocument doc;

DeserializationError error = deserializeJson(doc, result, DeserializationOption::Filter(filter));

if (error) {
//  Serial.print("deserializeJson(forecast) failed: ");
//  Serial.println(error.c_str());
  errorcode = 3;
  
   return;
}

int i = 0;
for (JsonObject weatherForecast_item : doc["weatherForecast"].as<JsonArray>()) {

  forecastDay[i] = weatherForecast_item["week"].as<String>(); // "Monday", "Tuesday", ...
  forecastmaxTmpC[i] = weatherForecast_item["forecastMaxtemp"]["value"];
  forecastminTmpC[i] = weatherForecast_item["forecastMintemp"]["value"];
  forecastmaxRH[i] = weatherForecast_item["forecastMaxrh"]["value"]; // 90, ...
  forecastminRH[i] = weatherForecast_item["forecastMinrh"]["value"]; // 60, ...
forecastPSR[i] = weatherForecast_item["PSR"].as<String>(); // "Low", "Medium Low", "Low", ...

  int tmpicon = weatherForecast_item["ForecastIcon"];
   if (tmpicon > 89){tmpicon -= 65;} // Hot 90/ 	Warm 91/ 	Cool 92/	Cold 93 / -> 25-28
   else if (tmpicon > 79){tmpicon -= 61;} // Windy 80/	Dry	81/ Humid 82/	Fog	83/ Mist	84/Haze 85/ -> 19-24
else if (tmpicon > 69){tmpicon -= 59;} // night icon 70 - 77 -> 11-18
    else if (tmpicon > 59){tmpicon -= 55;} // Cloudy	Overcast 60/	Light Rain 61/	Rain 62/	Heavy Rain	63/ Thunderstorms 64/ -> 5-9
   else {tmpicon -= 50;} // Sunny 50/	Sunny Periods 51/	Sunny Intervals	52/Sunny Periods with A Few Showers 53/	Sunny Intervals with Showers	54 -> 0-4
  
   forecasticon[i] = tmpicon;

 

//read today min max from memory, overwrite if date has changed
pref.begin("weatherpostcard", false);
nowdaycheck = pref.getInt("nowdaycheck", 0);


if (nowdayint != nowdaycheck) {
pref.putInt("nowdaycheck", nowdayint);
pref.putInt("todaymintmpC", forecastminTmpC[i]);
pref.putInt("todaymaxtmpC", forecastmaxTmpC[i]);}

  todaymintmpC = pref.getInt("todaymintmpC", 0);
todaymaxtmpC = pref.getInt("todaymaxtmpC", 0);
pref.end();


if (weatherForecast_item["forecastDate"].as<String>() ==  nowyear + nowmonth + nowday){
   i-=1; 
} 

i++;
if (i>forecastdays-1){break;}

}


http.end();}
else {//Serial.println("Failed to get forecast information");
errorcode = 3;}
}



void getCurrentWeather(){ // Get current weather data
HTTPClient http;
http.begin (baseURL+cwrq);
int httpCode = http.GET();

if (httpCode > 0) {
  String result = http.getString();


JsonDocument filter;

JsonObject filter_rainfall_data_0 = filter["rainfall"]["data"].add<JsonObject>();
filter_rainfall_data_0["place"] = true;
filter_rainfall_data_0["max"] = true;
filter_rainfall_data_0["min"] = true;
filter["icon"] = true;
filter["uvindex"]["data"][0]["value"] = true;

JsonObject filter_temperature_data_0 = filter["temperature"]["data"].add<JsonObject>();
filter_temperature_data_0["place"] = true;
filter_temperature_data_0["value"] = true;
//filter["tcmessage"] = true;

JsonObject filter_humidity = filter["humidity"].to<JsonObject>();
//filter_humidity["recordTime"] = true;
filter_humidity["data"][0]["value"] = true;

JsonDocument doc;

DeserializationError error = deserializeJson(doc, result, DeserializationOption::Filter(filter));

if (error) {
 // Serial.print("deserializeJson(currentweather) failed: ");
 // Serial.println(error.c_str());
    errorcode = 3;
    
 
  return;
}


for (JsonObject rainfall_data_item : doc["rainfall"]["data"].as<JsonArray>()) {


  String rainfall_data_item_place = rainfall_data_item["place"].as<String>(); // "Central & Western District", ...
  int rainfall_data_item_max = rainfall_data_item["max"]; // 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, ...
  int rainfall_data_item_min = rainfall_data_item["min"];
if (rainfall_data_item["place"] == rainlocation)
{rainmax= rainfall_data_item["max"];
 rainmin= rainfall_data_item["min"];
 }


}

weathericon = doc["icon"][0]; // 50
   if (weathericon > 89){weathericon -= 65;} // Hot 90/ 	Warm 91/ 	Cool 92/	Cold 93 / -> 25-28
   else if (weathericon > 79){weathericon -= 61;} // Windy 80/	Dry	81/ Humid 82/	Fog	83/ Mist	84/Haze 85/ -> 19-24
else if (weathericon > 69){weathericon -= 59;} // night icon 70 - 77 -> 11-18
    else if (weathericon > 59){weathericon -= 55;} // Cloudy	Overcast 60/	Light Rain 61/	Rain 62/	Heavy Rain	63/ Thunderstorms 64/ -> 5-10
   else {weathericon -= 50;} // Sunny 50/	Sunny Periods 51/	Sunny Intervals	52/Sunny Periods with A Few Showers 53/	Sunny Intervals with Showers	54 -> 0-4


uv = doc["uvindex"]["data"][0]["value"]; // 8

for (JsonObject temperature_data_item : doc["temperature"]["data"].as<JsonArray>()) {

String CurrenttmpClocation = temperature_data_item["place"].as<String>(); // "King's Park", "Hong Kong ...
 // CurrentTmpC = temperature_data_item["value"]; // 33, 33, 34, 36, 35, 36, 35, 34, 35, ...
//settmpClocation
if (CurrenttmpClocation==tmpClocation)
{CurrentTmpC = temperature_data_item["value"];}

}

// const char* tcmessage = doc["tcmessage"]; // nullptr

//const char* humidity_recordTime = doc["humidity"]["recordTime"]; // "2024-08-04T15:00:00+08:00"

CurrentRH = doc["humidity"]["data"][0]["value"]; // 61



http.end();}
else {errorcode = 3;
  //Serial.println("Failed to get Current weather information");
}}


void getVisibility(){ // Get current weather data visibility
HTTPClient http;
http.begin (baseURL2 + vbrq);
int httpCode = http.GET();

if (httpCode > 0) {
  String result = http.getString();


JsonDocument filter;
filter["data"][0] = true;

JsonDocument doc;

DeserializationError error = deserializeJson(doc, result, DeserializationOption::Filter(filter));

if (error) {
  //Serial.print("deserializeJson(visibility) failed: ");
  //Serial.println(error.c_str());
    errorcode = 3;
    
  return;
}


//Visibility taken from Central if you wish to use other locations, change the data_0 to data_n (n=1 for Chek Lap Kok /  2 for Sai Wan Ho/ 3 for Waglan Island)
JsonArray data = doc["data"];

JsonArray data_0 = data[0];
//const char* data_0_0 = data_0[0]; // "202408041730"
//const char* data_0_1 = data_0[1]; // "Central"
Vis = data_0[2].as<String>();; // "29 km"
Vis.replace(" ", ""); //removes the space

/*

JsonArray data_1 = data[1];
const char* data_1_0 = data_1[0]; // "202408041730"
const char* data_1_1 = data_1[1]; // "Chek Lap Kok"
const char* data_1_2 = data_1[2]; // "50 km"

JsonArray data_2 = data[2];
const char* data_2_0 = data_2[0]; // "202408041730"
const char* data_2_1 = data_2[1]; // "Sai Wan Ho"
const char* data_2_2 = data_2[2]; // "50 km"

JsonArray data_3 = data[3];
const char* data_3_0 = data_3[0]; // "202408041730"
const char* data_3_1 = data_3[1]; // "Waglan Island"
const char* data_3_2 = data_3[2]; // "45 km"
*/


http.end();}
else {errorcode = 3;
  //Serial.println("Failed to get visibility information");
}}


void getSRS() { // Get Sunset and Sunrise
HTTPClient http;
http.begin (baseURL2 + srsrq);
int httpCode = http.GET();

if (httpCode > 0) {
  String result = http.getString();
JsonDocument filter;
filter["data"] = true;

JsonDocument doc;

DeserializationError error = deserializeJson(doc, result, DeserializationOption::Filter(filter));

if (error) {
  //Serial.print("deserializeJson(SRS) failed: ");
  //Serial.println(error.c_str());
    errorcode = 3;
    
  return;
}

JsonArray data_0 = doc["data"][0];
//const char* data_0_0 = data_0[0]; // "2024-08-04"
sunrise = data_0[1].as<String>(); // "05:57"
//const char* data_0_2 = data_0[2]; // "12:29"
sunset = data_0[3].as<String>(); // "19:02"

http.end();
}
else {errorcode = 3;
//Serial.println("Fail to get SRS");
}

} 



void getwarning(){

HTTPClient http;
http.begin (baseURL + warnrq);
int httpCode = http.GET();

if (httpCode > 0) {
  String result = http.getString();


JsonDocument doc;

DeserializationError error = deserializeJson(doc, result);

if (error) {
//  Serial.print("deserializeJson(warning) failed: ");
 // Serial.println(error.c_str());
    errorcode = 3;
    
  return;
}


int i=0;
for (JsonPair item : doc.as<JsonObject>()) {
  //const char* item_key = item.key().c_str(); // "WHOT", "WCOLD", "WFNTSA", "WMSGNL"
  //const char* value_name = item.value()["name"]; // "Very Hot Weather Warning", "Cold Weather Warning", ...
  //const char* value_code = item.value()["code"]; // "WHOT", "WCOLD", "WCOLD", "WCOLD"
 // const char* value_actionCode = item.value()["actionCode"]; // "ISSUE", "ISSUE", "ISSUE", "ISSUE"
 // const char* value_issueTime = item.value()["issueTime"]; // "2020-09-24T07:00:00+08:00", ...
 // const char* value_updateTime = item.value()["updateTime"]; // "2020-09-24T07:00:00+08:00", ...


String warncode = item.value()["code"].as<String>(); // "WHOT", "WCOLD", "WCOLD", "WCOLD"
String actioncode = item.value()["actionCode"].as<String>(); // "ISSUE", "ISSUE", "ISSUE", "ISSUE"

if (warncode !="" && actioncode !="CANCEL")
{if (warncode=="WTMW"||warncode=="WFNTSA" ||warncode=="WMSGNL" || warncode=="WTCPRE8" || warncode=="WFROST")
{warnicon[i]="";}
else {warnicon[i]=warncode;
i++;}
}
}
http.end();}
else {//Serial.println("Failed to get warning information");
errorcode = 3;}
}


//connect to wifi and sync with NTP, wifi remains connected 
void wifiNTP(){
WiFi.begin(ssid, pass);
//Serial.print ("Connecting to Wifi");
while (WiFi.status() != WL_CONNECTED)
{//Serial.print (".");
delay (1000);
if (millis()>15000) {//Serial.print("\nError 1 - Fail to connect to Wifi\n");
errorcode = 1;

return;}
}
errorcode = 0;
//Serial.println("\nWifi Connected");

//Serial.println("Contacting NTP");
configTime(3600 * timezone, daysavetime * 3600, "time.nist.gov", "pool.ntp.org", "stdtime.gov.hk"); //configtime
//Serial.println("Configuring time");
  struct tm tmstruct;
  tmstruct.tm_year = 0;
  getLocalTime(&tmstruct, 5000);
   if (tmstruct.tm_year < 100){
   // Serial.println ("Error 2 -Configtime fails");
 // delay(100); 
  errorcode = 2;
return;
  }
  errorcode = 0;
 // Serial.println ("Configtime done");
  nowyear = String((tmstruct.tm_year) +1900);
  nowmonth = String((tmstruct.tm_mon)+1);
  nowday = String(tmstruct.tm_mday);
  nowdayint = tmstruct.tm_mday;
  nowhour = tmstruct.tm_hour;
  nowminute = tmstruct.tm_min;
 //Serial.printf("Now is %02d:%02d - %s/%s/%s\n", nowhour, nowminute, nowday, nowmonth, nowyear);
snprintf (srsrq, sizeof(srsrq), "SRS&lang=en&rformat=json&year=%s&month=%s&day=%s", nowyear, nowmonth, nowday);
if (nowday.length()==1){nowday = "0"+nowday;}
if (nowmonth.length()==1){nowmonth = "0"+nowmonth;}
errorcode = 0;
}