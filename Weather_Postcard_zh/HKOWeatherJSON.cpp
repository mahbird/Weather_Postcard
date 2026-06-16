#include "configurations.h"
#include "HKOWeatherJSON.h"

// #define useAQI

#define forecastdays 3
#define zh_trim_char 10 // length of trimmed weather description today zh/en
#define en_trim_char 18
#define forecast_zh_trim_char 7 // length of trimmed weather description zh/en
#define forecast_en_trim_char 15


Preferences pref;
int nowday, nowmonth, nowyear, nowhour, nowminute;
RTC_DATA_ATTR int errorcode = 0;
RTC_DATA_ATTR int errorcount = 0;

RTC_DATA_ATTR int todayrainchance;
RTC_DATA_ATTR int todaymintmpC = 0;
RTC_DATA_ATTR int todaymaxtmpC = 0;
RTC_DATA_ATTR int todayminRH = 0;
RTC_DATA_ATTR int todaymaxRH = 0;
String todaydescription = "";


// back up data if the current station fails
static String tmpClocation0 = "Hong Kong Observatory";

bool updateSRS = false;

RTC_DATA_ATTR int nowdaycheck = 0;

const char baseURL[] = "https://data.weather.gov.hk/weatherAPI/opendata/weather.php?dataType=";   // HKO weather
const String cwrq = "rhrread&lang=en";                                                            // CurrentWeather request
const String fcrq_en = "fnd&lang=en";                                                             // Forcast request english
const String fcrq_tc = "fnd&lang=tc";                                                             // Forcast request traditional chinese
const String baseURL2 = "https://data.weather.gov.hk/weatherAPI/opendata/opendata.php?dataType="; // HKO OpenWeather
const String vbrq = "LTMV&lang=en&rformat=json";                                                  // visibility request
const String warnrq_tc = "warnsum&lang=tc";                                                       // current weather warning
const String warnrq_en = "warnsum&lang=en";                                                       // current weather warning
const String flwrq_tc = "flw&lang=tc";
const String flwrq_en = "flw&lang=en";
// char srsrq [60]; // moved to getSRS();

// String forecastdate[3]; //forecast variables
String forecastDay[3];
RTC_DATA_ATTR int forecastmaxTmpC[3] = {-1, -1, -1};
RTC_DATA_ATTR int forecastminTmpC[3] = {-1, -1, -1};
RTC_DATA_ATTR int forecastmaxRH[3] = {-1, -1, -1};
RTC_DATA_ATTR int forecastminRH[3] = {-1, -1, -1};
RTC_DATA_ATTR int forecastPSR[3];
RTC_DATA_ATTR int forecasticon[3];
String forecastdescription[3]{"", "", ""};

// Current weather variables
RTC_DATA_ATTR int rainmax = 0;
RTC_DATA_ATTR int rainmin = 0;

RTC_DATA_ATTR int nowweathericon = 0;
RTC_DATA_ATTR int uv = 0;

RTC_DATA_ATTR int CurrentTmpC = -1;
RTC_DATA_ATTR int CurrentRH = -1;

// other variables
String Vis;
String sunrise;
String sunset;
RTC_DATA_ATTR warncode warnicon[5];

void getCurrentWeather()
{ // Get current weather data
  HTTPClient http;
  http.begin(baseURL + cwrq);
  int httpCode = http.GET();

  if (httpCode > 0)
  {
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
    // filter["tcmessage"] = true;

    JsonObject filter_humidity = filter["humidity"].to<JsonObject>();
    // filter_humidity["recordTime"] = true;
    filter_humidity["data"][0]["value"] = true;

    JsonDocument doc;

    DeserializationError error = deserializeJson(doc, result, DeserializationOption::Filter(filter));

    if (error)
    {
      //Serial.print("deSerialzeJson(currentweather) failed: ");
      //Serial.println(error.c_str());
      errorcode = 3;

      return;
    }

    for (JsonObject rainfall_data_item : doc["rainfall"]["data"].as<JsonArray>())
    {

      String rainfall_data_item_place = rainfall_data_item["place"].as<String>(); // "Central & Western District", ...
      int rainfall_data_item_max = rainfall_data_item["max"];                     // 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, ...
      int rainfall_data_item_min = rainfall_data_item["min"];
      if (rainfall_data_item["place"] == rainlocation)
      {
        rainmax = rainfall_data_item["max"];
        rainmin = rainfall_data_item["min"];
      }
    }

    nowweathericon = doc["icon"][0]; // 50
    if (nowweathericon > 89)
    {
      nowweathericon -= 65;
    } // Hot 90/ 	Warm 91/ 	Cool 92/	Cold 93 / -> 25-28
    else if (nowweathericon > 79)
    {
      nowweathericon -= 61;
    } // Windy 80/	Dry	81/ Humid 82/	Fog	83/ Mist	84/Haze 85/ -> 19-24
    else if (nowweathericon > 69)
    {
      nowweathericon -= 59;
    } // night icon 70 - 77 -> 11-18
    else if (nowweathericon > 59)
    {
      nowweathericon -= 55;
    } // Cloudy	Overcast 60/	Light Rain 61/	Rain 62/	Heavy Rain	63/ Thunderstorms 64/ -> 5-10
    else
    {
      nowweathericon -= 50;
    } // Sunny 50/	Sunny Periods 51/	Sunny Intervals	52/Sunny Periods with A Few Showers 53/	Sunny Intervals with Showers	54 -> 0-4

    uv = doc["uvindex"]["data"][0]["value"]; // 8

    for (JsonObject temperature_data_item : doc["temperature"]["data"].as<JsonArray>())
    {

      String CurrenttmpClocation = temperature_data_item["place"].as<String>(); // "King's Park", "Hong Kong ...
                                                                                // CurrentTmpC = temperature_data_item["value"]; // 33, 33, 34, 36, 35, 36, 35, 34, 35, ...
      // settmpClocation
      if (CurrenttmpClocation == tmpClocation0 && CurrentTmpC == -1)
      {
        CurrentTmpC = temperature_data_item["value"];
        //Serial.println(tmpClocation0);
      }
      if (CurrenttmpClocation == tmpClocation)
      {
        CurrentTmpC = temperature_data_item["value"];
        //Serial.println(tmpClocation);
      }
    }

    // const char* tcmessage = doc["tcmessage"]; // nullptr

    // const char* humidity_recordTime = doc["humidity"]["recordTime"]; // "2024-08-04T15:00:00+08:00"

    CurrentRH = doc["humidity"]["data"][0]["value"]; // 61

    // errorcode = 0;
  }
  else
  {
    errorcode = 3;
    //Serial.println("Failed to get Current weather information");
  }

  http.end();
}

// #endif

void getVisibility()
{ // Get current weather data visibility
  HTTPClient http;
  http.begin(baseURL2 + vbrq);
  int httpCode = http.GET();

  if (httpCode > 0)
  {
    String result = http.getString();

    JsonDocument filter;
    filter["data"][0] = true;

    JsonDocument doc;

    DeserializationError error = deserializeJson(doc, result, DeserializationOption::Filter(filter));

    if (error)
    {
      //Serial.print("deSerialzeJson(visibility) failed: ");
      //Serial.println(error.c_str());
      errorcode = 3;

      return;
    }

    // Visibility taken from Central if you wish to use other locations, change the data_0 to data_n (n=1 for Chek Lap Kok /  2 for Sai Wan Ho/ 3 for Waglan Island)
    JsonArray data = doc["data"];

    JsonArray data_0 = data[0];
    // const char* data_0_0 = data_0[0]; // "202408041730"
    // const char* data_0_1 = data_0[1]; // "Central"
    Vis = data_0[2].as<String>();
    ;                     // "29 km"
    Vis.replace(" ", ""); // removes the space

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

    // errorcode = 0;
  }
  else
  {
    errorcode = 3;
    //Serial.println("Failed to get visibility information");
  }
  http.end();
}

void getSRS()
{ // Get Sunset and Sunrise

  if (updateSRS)
  {
    char srsrq[60];
    snprintf(srsrq, sizeof(srsrq), "SRS&lang=en&rformat=json&year=%d&month=%d&day=%d", nowyear, nowmonth, nowday);
    HTTPClient http;
    http.begin(baseURL2 + srsrq);
    int httpCode = http.GET();

    if (httpCode > 0)
    {
      String result = http.getString();
      JsonDocument filter;
      filter["data"] = true;

      JsonDocument doc;

      DeserializationError error = deserializeJson(doc, result, DeserializationOption::Filter(filter));

      if (error)
      {
        //Serial.print("deSerialzeJson(SRS) failed: ");
        //Serial.println(error.c_str());
        errorcode = 3;

        return;
      }

      JsonArray data_0 = doc["data"][0];
      // const char* data_0_0 = data_0[0]; // "2024-08-04"
      sunrise = data_0[1].as<String>(); // "05:57"
      // const char* data_0_2 = data_0[2]; // "12:29"
      sunset = data_0[3].as<String>(); // "19:02"

      pref.begin("weather", false); // **test(probably not needed)
      pref.putString("sunrise", sunrise);
      pref.putString("sunset", sunset);
      pref.end();
      //Serial.printf("Sunrise for the day saved: %s, sunset: %s\n", sunrise, sunset);

      http.end();
    }
    else
    {
      errorcode = 3;
      //Serial.println("Fail to get SRS");
    }
  }
  else
  {
    pref.begin("weather", true);
    sunrise = pref.getString("sunrise", "");
    sunset = pref.getString("sunset", "");
    //Serial.println("SRS update not needed, load SRS from pref");
  }
}

void gettodaydescription(language lang)
{ // Get weather description today, traditional chinese
  HTTPClient http;
  if (lang == ZH)
  {
    http.begin(baseURL + flwrq_tc);
  }
  else if (lang == EN)
  {
    http.begin(baseURL + flwrq_en);
  }
  int httpCode = http.GET();

  if (httpCode > 0)
  {
    String result = http.getString();

    JsonDocument filter;

    // filter["generalSituation"] = "一股強烈冬季季候風正為廣東帶來普遍晴朗的天氣。正午時分，本港各區的相對濕度下降至百分之四十左右。";
    // filter["tcInfo"] = "";
    // filter["fireDangerWarning"] = "";
    // filter["forecastPeriod"] = "本港地區下午及今晚天氣預測";
    filter["forecastDesc"] = true;
    // filter["outlook"] = "週末期間早上持續寒冷，市區氣溫在11度左右，新界部分地區及高地嚴寒，日夜溫差頗大。日間非常乾燥。星期一早上仍然相當清涼，隨後一兩日雲量增多，氣溫稍為回升。";
    // filter["updateTime"] = "2025-01-10T12:45:00+08:00";

    JsonDocument doc;

    DeserializationError error = deserializeJson(doc, result, DeserializationOption::Filter(filter));

    if (error)
    {
      //Serial.print("deSerialzeJson(today weather description) failed: ");
      //Serial.println(error.c_str());
      errorcode = 3;

      return;
    }
    // errorcode = 0;
    http.end();

    todaydescription = doc["forecastDesc"].as<String>(); // "天晴。下午非常乾燥。吹和緩至清勁北至東北風。"
    if (lang == ZH)
    {
      trimweatherZH(todaydescription, zh_trim_char);
    }
    else
    {
      trimweatherEN(todaydescription, en_trim_char);
    }

  }
  else
  {
    errorcode = 3;
    //Serial.println("Failed to get visibility information");
  }
}

String forecastextract(String weatherinput)
{
  if (weatherinput.indexOf("Sunny periods") >= 0)
  {
    return "Sunny periods";
  }

  else if (weatherinput.indexOf("Sunny interval") >= 0)
  {
    return "Sunny interval";
  }

  else if (weatherinput.indexOf("Mostly sunny") >= 0)
  {
    return "Mostly sunny";
  }

  else if (weatherinput.indexOf("Sunny") >= 0)
  {
    return "Sunny";
  }

  else if (weatherinput.indexOf("Mainly cloudy") >= 0)
  {
    return "Mainly cloudy";
  }

  else if (weatherinput.indexOf("Mostly cloudy") >= 0)
  {
    return "Mostly cloudy";
  }

  else if (weatherinput.indexOf("Cloudy") >= 0)
  {
    return "Cloudy";
  }

  else
  {
    return weatherinput.substring(0, 15);
  }
}

void getForecast(language lang)
{
  HTTPClient http;
  if (lang == ZH)
  {
    http.begin(baseURL + fcrq_tc);
  }
  else
  {
    http.begin(baseURL + fcrq_en);
  }
  int httpCode = http.GET();

  //Serial.printf("Getting forecast, http code: %d\n", httpCode);

  if (httpCode > 0)
  {
    String result = http.getString();

    JsonDocument filter;

    JsonObject filter_weatherForecast_0 = filter["weatherForecast"].add<JsonObject>();
    filter_weatherForecast_0["forecastDate"] = true;
    filter_weatherForecast_0["week"] = true;
    filter_weatherForecast_0["forecastWeather"] = true;
    filter_weatherForecast_0["forecastMaxtemp"]["value"] = true;
    filter_weatherForecast_0["forecastMintemp"]["value"] = true;
    filter_weatherForecast_0["forecastMaxrh"]["value"] = true;
    filter_weatherForecast_0["forecastMinrh"]["value"] = true;
    filter_weatherForecast_0["ForecastIcon"] = true;
    filter_weatherForecast_0["PSR"] = true;
    filter_weatherForecast_0["forecastWeather"] = true;

    JsonDocument doc;

    DeserializationError error = deserializeJson(doc, result, DeserializationOption::Filter(filter));

    if (error)
    {
      ////Serial.print("deSerialzeJson(forecast) failed: ");
      ////Serial.println(error.c_str());
      errorcode = 3;

      return;
    }
    int datecheckint = nowyear * 10000 + nowmonth * 100 + nowday;
    bool readpreftoday = false;
    // bool gettodayweather = true;
    // bool readtodayweatheronce = false;
    int i = 0;
    for (JsonObject weatherForecast_item : doc["weatherForecast"].as<JsonArray>())
    {

      forecastdescription[i] = weatherForecast_item["forecastWeather"].as<String>();
      forecastDay[i] = weatherForecast_item["week"].as<String>(); // "Monday", "Tuesday", ...
      forecastmaxTmpC[i] = weatherForecast_item["forecastMaxtemp"]["value"];
      forecastminTmpC[i] = weatherForecast_item["forecastMintemp"]["value"];
      forecastmaxRH[i] = weatherForecast_item["forecastMaxrh"]["value"]; // 90, ...
      forecastminRH[i] = weatherForecast_item["forecastMinrh"]["value"]; // 60, ...

      String PSRString = weatherForecast_item["PSR"].as<String>(); // "Low", "Medium Low", "Low", ...
      if (lang == ZH)
      {
        forecastPSR[i] = PSRtoInt(PSRString, ZH);
        // forecastdescription[i] = trimweatherZH(forecastdescription[i], forecast_zh_trim_char); // changed to void
        trimweatherZH(forecastdescription[i], forecast_zh_trim_char);
        //Serial.print("PSR int:");
        //Serial.println(forecastPSR[i]);
      }

      else
      {
        forecastPSR[i] = PSRtoInt(PSRString, EN);
        // forecastdescription[i] = trimweatherEN(forecastdescription[i], forecast_en_trim_char); // changed to void
        trimweatherEN(forecastdescription[i], forecast_en_trim_char);
        //Serial.print("PSR int:"  );
        //Serial.println(forecastPSR[i]);
      }

      int tmpicon = weatherForecast_item["ForecastIcon"];
      if (tmpicon > 89)
      {
        tmpicon -= 65;
      } // Hot 90/ 	Warm 91/ 	Cool 92/	Cold 93 / -> 25-28
      else if (tmpicon > 79)
      {
        tmpicon -= 61;
      } // Windy 80/	Dry	81/ Humid 82/	Fog	83/ Mist	84/Haze 85/ -> 19-24
      else if (tmpicon > 69)
      {
        tmpicon -= 59;
      } // night icon 70 - 77 -> 11-18
      else if (tmpicon > 59)
      {
        tmpicon -= 55;
      } // Cloudy	Overcast 60/	Light Rain 61/	Rain 62/	Heavy Rain	63/ Thunderstorms 64/ -> 5-9
      else
      {
        tmpicon -= 50;
      } // Sunny 50/	Sunny Periods 51/	Sunny Intervals	52/Sunny Periods with A Few Showers 53/	Sunny Intervals with Showers	54 -> 0-4

      forecasticon[i] = tmpicon;

      // pref.end();  // end the current pref ** test probably not needed

      if (nowdaycheck == 0)
      {
        pref.begin("weather", true); // not sure if needed, just in case
        nowdaycheck = pref.getInt("nowdaycheck", 0);
        sunrise = pref.getString("sunrise", "");
        sunset = pref.getString("sunset", "");
        todayminRH = pref.getInt("todayminRH", 0);
        todaymaxRH = pref.getInt("todaymaxRH", 0);
        todaymintmpC = pref.getInt("todaymintmpC", 0);
        todaymaxtmpC = pref.getInt("todaymaxtmpC", 0);
        todayrainchance = pref.getInt("todayrainchance", 0);
        //Serial.println("No RTC data, loaded data from preferences");
        if (sunrise == "" || sunset == "")
        {
          updateSRS = true;
        }
        pref.end();
      }

      // pref.begin("weather", false);  //**test probably not needed */

      if (datecheckint != nowdaycheck || todaymintmpC == 0)
      {
        todaymintmpC = forecastminTmpC[i];
        todaymaxtmpC = forecastmaxTmpC[i];
        todayminRH = forecastminRH[i];
        todaymaxRH = forecastmaxRH[i];
        todayrainchance = forecastPSR[i];
        nowdaycheck = datecheckint;

        pref.begin("weather", false); // not sure if needed, just in case
        pref.putInt("nowdaycheck", nowdaycheck);
        pref.putInt("todaymintmpC", forecastminTmpC[i]);
        pref.putInt("todaymaxtmpC", forecastmaxTmpC[i]);
        pref.putInt("todayminRH", forecastminRH[i]);
        pref.putInt("todaymaxRH", forecastmaxRH[i]);
        pref.putInt("todayrainchance", forecastPSR[i]);
        updateSRS = true; // get changed SRS
        pref.end();       // not sure if needed, just in case
        //Serial.printf("datecheckint: %d, nowdaycheck: %d\n", datecheckint, nowdaycheck);
        //Serial.println("Forecast for today saved");
        // pref.begin("weather", false);
      }



      //Serial.printf("Forecast date: %s, datecheckint: %s\n", weatherForecast_item["forecastDate"].as<String>(), String(datecheckint));

      if (weatherForecast_item["forecastDate"].as<String>() == String(datecheckint))
      {
        i -= 1;

        //Serial.println("Skipped forecast for today");
      }

      else
      {
        //Serial.printf("Saved forecast values for %s: %d-%dC, %s\n", weatherForecast_item["forecastDate"].as<String>(),  forecastminTmpC[0], forecastmaxTmpC[0], forecastdescription[0]);
        // pref.putInt("tmrweathericon", forecasticon[i]);
        // pref.putInt("tmrmintmpC", forecastminTmpC[i]);
        // pref.putInt("tmrmaxtmpC", forecastmaxTmpC[i]);
        // pref.putInt("tmrminRH", forecastminRH[i]);
        // pref.putInt("tmrmaxRH", forecastmaxRH[i]);
        // pref.putInt("tmrrainchance", forecastPSR[i]);
        pref.putString("tmrdesc", forecastdescription[i]);
      }

      i++;
      if (i > forecastdays - 1)
      {
        break;
      }
    }
    // errorcode = 0;
    http.end();
  }
  else
  {
    //Serial.println("Failed to get forecast information");
    errorcode = 3;
  }
}

int PSRtoInt(String PSRinput, language lang)
{
  PSRinput.trim();
  if (lang == ZH)
  {
    if (PSRinput == "低")
    {
      return 0;
    }
    else if (PSRinput == "中低")
    {
      return 1;
    }
    else if (PSRinput == "中")
    {
      return 2;
    }
    else if (PSRinput == "中高")
    {
      return 3;
    }
    else if (PSRinput == "高")
    {
      return 4;
    }
    else
    {
      return -1;
    } // invalid input
  }

  else if (lang == EN) // EN中
  {
    if (PSRinput == "Low")
    {
      return 0;
    }
    else if (PSRinput == "Medium Low")
    {
      return 1;
    }
    else if (PSRinput == "Medium")
    {
      return 2;
    }
    else if (PSRinput == "Medium High")
    {
      return 3;
    }
    else if (PSRinput == "High")
    {
      return 4;
    }
    else
    {
      return -1;
    } // invalid input
  }

  else
  {
    return -1;
  } // just in case
}

String PSRtoString(int PSRInt)
{
  switch (PSRInt)
  {
  case 0:
    return "<30%";
    break;

  case 1:
    return "30-44%";
    break;

  case 2:
    return "45-54%";
    break;

  case 3:
    return "55-69%";
    break;

  case 4:
    return ">70%";
    break;
  }
  return "N/A";
}

void trimweatherZH(String &weatherinput, int numofchar)
{
  weatherinput = weatherinput.substring(0, 3 + numofchar * 3);
  // Serial.println("Original:" + weatherinput);
  weatherinput.replace("。", "，");
  weatherinput.replace("，", ",");
  int commaindex = weatherinput.lastIndexOf(",");
  // Serial.println("Comma index: " + String(commaindex));
  int andindex = weatherinput.lastIndexOf("及");
  // Serial.println("And index: " + String(andindex));
  if (commaindex > 0 || andindex > 0) 
  {
  
    if (andindex > 0 && (andindex - commaindex) < 9) // **test trim to comma if and is less than 3 chinese char after comma otherwise it might not make sense e.g xx及
    {weatherinput = weatherinput.substring(0, commaindex);}

    else {
    int index = (commaindex > andindex) ? commaindex : andindex;
    weatherinput = weatherinput.substring(0, index); }
  }
  else
  {
    weatherinput = weatherinput.substring(0, numofchar * 3); // goes back one char if no index found
  }
  // Serial.println("Trimmed:" + weatherinput);
}

void getwarning()
{
  HTTPClient http;
  for (int i = 0; i < 5; i++)
  {
    warnicon[i] = NOWARNING;
  }

    http.begin(baseURL + warnrq_en);
  int httpCode = http.GET();

  if (httpCode > 0)
  {
    String result = http.getString();

    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, result);

    if (error)
    {
      //  //  //  Serial.print("deSerialzeJson(warning) failed: ");
      //  //  Serial.println(error.c_str());
      errorcode = 3;

      return;
    }

    int i = 0;
    for (JsonPair item : doc.as<JsonObject>())
    {
      // const char* item_key = item.key().c_str(); // "WHOT", "WCOLD", "WFNTSA", "WMSGNL"
      // const char* value_name = item.value()["name"]; // "Very Hot Weather Warning", "Cold Weather Warning", ...
      // const char* value_code = item.value()["code"]; // "WHOT", "WCOLD", "WCOLD", "WCOLD"
      // const char* value_actionCode = item.value()["actionCode"]; // "ISSUE", "ISSUE", "ISSUE", "ISSUE"
      // const char* value_issueTime = item.value()["issueTime"]; // "2020-09-24T07:00:00+08:00", ...
      // const char* value_updateTime = item.value()["updateTime"]; // "2020-09-24T07:00:00+08:00", ...

      //const std::string rawcode = item.value()["code"].as<std::string>(); // "WHOT", "WCOLD", "WCOLD", "WCOLD" // test works but avoiding std::string to save memory
      String actioncode = item.value()["actionCode"].as<String>();        // "ISSUE", "ISSUE", "ISSUE", "ISSUE"

      const char* rawcode = item.value()["code"].as<const char*>(); // test 

      warnicon[i] = warningtoenum(rawcode);
      if (warnicon[i] != NOWARNING)
      {
        i++;
      }

      // old
      /*
      if (rawcode="" || rawcode=="WTMW"||rawcode=="WFNTSA" ||rawcode=="WMSGNL" || rawcode=="WTCPRE8" || rawcode=="WFROST")
      {
      //  //  Serial.println("No warning or code is not on list");
      //  //  Serial.println (rawcode.c_str());
      }
      else if (actioncode !="CANCEL")
      {warnicon[i] = warningtoenum(rawcode);
      i++;
      }
      */
    }

    http.end();
  }
  else
  { //  //  Serial.println("Failed to get warning information");
    errorcode = 3;
  }
}


// This is probably making things more complicated but since there's so limited space for english to make sense
// Currently checks for punctuations/ specific keywords, then description and lastly last space, if anyone has better suggestion let me know
void trimweatherEN(String &weatherinput, int numofchar) 
{
  weatherinput = weatherinput.substring(0, numofchar);
  // Serial.println("Initial trim: " + weatherinput);

  struct kwoff {
    int pos;
    int offset;
  };
  kwoff best = {-1, 0};

  // punctuation/connectors
  kwoff kw1[] = {
    {weatherinput.indexOf("."), 0},
    {weatherinput.indexOf(","), 0},
    {weatherinput.indexOf("and"), 0},
    {weatherinput.indexOf("with"), 0},
    {weatherinput.indexOf("periods"), 8},
    {weatherinput.indexOf("intervals"), 9}
  };

  for (auto &kw : kw1) {
    if (kw.pos >= 0 && kw.pos <= numofchar && kw.pos > best.pos) {
      best = kw;
    }
  }

  if (best.pos >= 0) {
    weatherinput = weatherinput.substring(0, best.pos + best.offset);
    return;
  }

// checks for things like very/ mostly/ mainly etc if nothing above
const char *kw2[] = {"Mostly ", "Mainly ", "Very "}; // With the space
for (auto &kw : kw2) {
  int pos = weatherinput.indexOf(kw);
  if (pos >= 0 && pos < numofchar) {
    int secondspace = weatherinput.indexOf(" ", pos + strlen(kw)); 
    if (secondspace > 0) { // checks if there's a complete word, probably should
weatherinput = weatherinput.substring(0, secondspace);
return;
    } else {
      // Serial.println("Modifier '" + String(kw) + "' found but no complete word after it");
              }
  }
}

  // If no trim yet, check for other keywords 
  const char *kw3[] = {
  "Sunny", "Cloudy", "Fine", "Hot", "Rain", "Showers", "Thunderstorms", "Windy",
      "Dry", "Humid", "Fog", "Mist", "Haze"};

  String lowerInput = weatherinput;
  lowerInput.toLowerCase();

  for (auto &kw : kw3) {
    String lowerKw = kw;
    lowerKw.toLowerCase();
    if (lowerInput.indexOf(lowerKw) >= 0) {
      weatherinput = kw; // canonical output with proper casing
      return;
    }
  }

  // if still nothing, just trim last space
  int lastspacepos = weatherinput.lastIndexOf(" ");
  if (lastspacepos > 0) {
    weatherinput = weatherinput.substring(0, lastspacepos);
  }
}






void getweatherreport(std::string &report, language lang)
{
  report = "\n無法取得詳細天氣，請稍後重試";

  HTTPClient http;
  if (lang == ZH)
  {
    http.begin(baseURL + flwrq_tc);
  }
  else
  {
    http.begin(baseURL + flwrq_en);
  }
  int httpCode = http.GET();

  if (httpCode > 0)
  {
    String data = http.getString();

    JsonDocument filter;
    filter["generalSituation"] = true;
    filter["forecastPeriod"] = true;
    filter["forecastDesc"] = true;
    filter["outlook"] = true;

    JsonDocument doc;

    DeserializationError error = deserializeJson(doc, data, DeserializationOption::Filter(filter));

    if (error)
    {
      //Serial.print("deserializeJson() failed: getweatherreport");
      //Serial.println(error.c_str());
      errorcode = 3;
      return;
    }

    std::string generalSituation = doc["generalSituation"].as<std::string>(); // "一股清勁至強風程度的東北季候風正影響廣東沿岸。同時，一道廣闊雲帶覆蓋華南。"
    std::string forecastPeriod = doc["forecastPeriod"].as<std::string>();     // "本港地區今日天氣預測"
    std::string forecastDesc = doc["forecastDesc"].as<std::string>();         // "多雲清涼，有一兩陣雨。吹和緩至清勁北至東北風，離岸及高地間中吹強風。"
    std::string outlook = doc["outlook"].as<std::string>();                   // "未來兩三日天氣持續清涼，有一兩陣雨，市區最低氣溫在15度左右，新界再低兩三度。下週中後期天色好轉，氣溫逐步回升。"

    report = "天氣概況:\n" + generalSituation + +"\n" + forecastPeriod + ":\n" + forecastDesc + "\n展望:\n" + outlook;
    //Serial.println(report.c_str());
  }

  else
  {
    //Serial.println("Failed to get forecast information");
    errorcode = 3;
  }

  http.end();
}


/*
warncode warningtoenum(const std::string &warninginput)
{
  std::map<std::string, warncode> stringToEnum = {
      {"", NOWARNING},
      {"WFIREY", WFIREY},
      {"WFIRER", WFIRER},
      {"WHOT", WHOT},
      {"WCOLD", WCOLD},
      {"WRAINA", WRAINA},
      {"WRAINR", WRAINR},
      {"WRAINB", WRAINB},
      {"WL", WL},
      {"TC1", TC1},
      {"TC3", TC3},
      {"TC8NE", TC8NE},
      {"TC8SE", TC8SE},
      {"TC8NW", TC8NW},
      {"TC8SW", TC8SW},
      {"TC9", TC9},
      {"TC10", TC10},
      {"WTS", WTS}};

  auto it = stringToEnum.find(warninginput);
  if (it != stringToEnum.end())
  {
    warncode enumValue = it->second;
    return enumValue;
  }
  else
  {
    return NOWARNING;
  }
}
*/

//test trying to avoid std::string
warncode warningtoenum(const char* warninginput)
{
    struct warnmap { const char* text; warncode value; };
    static const warnmap table[] = {
        {"", NOWARNING},
        {"WFIREY", WFIREY},
        {"WFIRER", WFIRER},
        {"WHOT",   WHOT},
        {"WCOLD",  WCOLD},
        {"WRAINA", WRAINA},
        {"WRAINR", WRAINR},
        {"WRAINB", WRAINB},
        {"WL",     WL},
        {"TC1",    TC1},
        {"TC3",    TC3},
        {"TC8NE",  TC8NE},
        {"TC8SE",  TC8SE},
        {"TC8NW",  TC8NW},
        {"TC8SW",  TC8SW},
        {"TC9",    TC9},
        {"TC10",   TC10},
        {"WTS",    WTS},
		{"WMSGNL", WMSGNL}
    };

    for (auto &m : table) {
        if (strcmp(warninginput, m.text) == 0) {
            return m.value;
        }
    }
    return NOWARNING; // default if not found
}
