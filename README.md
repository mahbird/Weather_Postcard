<h1>Weather Postcard</h1>

This is Weather Postcard, a e-ink weather station that displays the current weather condition and 3 day forcasts. 
It is available in 4 color or in black/white eink screen. (With limited support for 3 color B/W/R panel (yellow is not shown))

Weather Postcard uses Hong Kong Observatory Open Data API, so can be used for weather in Hong Kong only.



<img src="https://github.com/user-attachments/assets/34ad3884-371f-4965-a3cb-2b93d82f40f7" height="300"><img src="https://github.com/user-attachments/assets/905c8267-9e64-404b-abd4-6f26ee25fbc8" height="300">

4 Color vs BW Version (not real data)


This project should be appropiate for Arduino beginners as there are no sensor modules involved. If you opt for the no battery choices with the Good Display / waveshare development board, you can even skip the basic soldering. If you have not used Arduino before, you just need to learn some basic things: how to upload codes, how to install ESP32 boards for Arduino, and how to install libraries.


**Why e-ink?**  E-ink displays can kept its last displayed content without power supply, so the device is in deep sleep mode between readings intervals. It only wakes up for around 30-60s on each update (to connect to wifi to fetch NTP and weather data and then 20s for refreshing the screen). It can be viewed under bright light. Depending on the battery and update interval you choose, it will last months without needing to be charged.


*****
**If you are new to e-ink, please take extra care when handling the e-ink screen. They are SUPER FRAGILE, if you drop it or put pressure on it, it is going to be a goner :'(**

Flickering on screen refreshes is, unfortunately, normal for e-ink as far as the current technology goes. For panels that are black and white only, the flickering time is much shorter. See videos below for references.
*****


https://github.com/user-attachments/assets/486b02ce-66e1-41c6-a889-607f01d05504

https://github.com/user-attachments/assets/113a706e-1558-40b3-8cf9-02a3e4cd63c4



<h2>Weather Postcard Explained</h2>
<p>
<img src="https://github.com/user-attachments/assets/32aa5b87-fce2-4041-a7f5-c9beafc634cf"></p>

The Weather Postcard first connects to the internet, syncing time with NTP server, then it requests data from HKO API. 

If any errors happen during any steps above, the relevant error message will be displayed. It will return again in 5 mins. If error still persists after 5 attempts, all subsequent retries will take place every 120 mins. The screen will not update until error is resolved. Error count is saved on the ESP32's NSV using Preferences library, so the count will remain until you have a successful load or if you clear the NVS manually.

After sucessfully getting data, the display will refresh. Once the update is finished, the ESP32 will go into deep sleep until the next wake up interval. Because the internal timer of the ESP32 has about ~5% (according to google), there will be some fluctuations for the update time. 

You can always manually press RST on your ESP32 to get data instantly.


<h2>Materials needed: </h2>
(Prices are the displayed price as of time of this post (1 Sep, 2024),  for reference only , excludes shipping / discounts). I do not recevie any form of rebate from the manufacturer(s), you are free to try out different ones.
<p>
<img src="https://github.com/user-attachments/assets/61679cf7-91b8-4f46-968a-c884e56e1c70" height=400px></p>


**E-ink screen:**
- 4.2-inch Black, White, Yellow, and Red E Ink Display, 400x300, GDEM042F52, from Good-Display  (https://www.good-display.com/product/564.html) - USD $20.9 on their official website/ CNY 89.9 on taobao
- OR 4.2-inch Black White, E Ink Display, 400x300, GDEY042T81, if you prefer less flickering over color (https://www.good-display.com/product/386.html) - USD $17.60 on their official website/ CNY 78 on taobao

**Microcontroller board:**
- ** recommended ** DFRobot FireBeetle 2 ESP32C6, SKU:DFR1075, from DFRobot: - USD $5.9 on their official website/ CNY 40 on taobao
(https://www.dfrobot.com/product-2771.html), this can also be purchased from their store in taobao.
This board has a low power consumption and comes with on-board charging circuit for lithium battery. It is also relatively inexpensive.

Or: Alternative that require no soldering, but doesn't not come with battery charging, you will have to add external power source:
- Good Display ESP32 development kit (ESP32-L(C02)): - USD $14.58 on their official website / CNY 110 on taobao, price includes the DESPI-C02 adaptor board
  (https://www.good-display.com/product/338.html)

- You need the version that comes with the C02 (ESP32-L(C02)). The advantage of this board is that it requires no soldering to use, and it comes with female header pins so you can test other sensors (if you wish to add those) easily. ESP32 also has better processor than the ESP32-C6, though it doesn't really matter for this project. The downside is it is relatively expensive and the power consumption is higher than the Firebeetle ESP32C6.

*You could also try other ESP32 boards, but you would have to adjust the pin numbers yourself, and if they don't work, I can't help you. Though if you decide to use different boards, I am sure you know what you are doing.*


**E-ink adaptor board:** (should have been included if you are buying the Good Display ESP32 development kit)
- e.g. DESPI-C02: - USD $6.99 on their official website/ CNY 30 on taobao
You need an adaptor board to connect the eink screen to your ESP32. There are other brands out there, some options are cheaper and they come in different forms, above is just a suggestion and is the one used in this tutorial. 


**Lithium Battery: (optional if you decide to power the unit directly from USB-C all the time)**
The Firebeetle 2 has a PH2.0 female connector for 3.7V lithium battery. There isn't a specific capacity you need to use, but I suggest around the range of 2000-5000mah. Lower ones will just result in more frequent charges and higher ones are just more bulky. Pay attention to the connector orientation. The +(red) side is on the left and the -(black) side is on the right when you view the board from the top. Buy the battery with the right connector from shops, best to avoid having to add them yourself unless you have appropiate tools. Should look like this:

<img src ="https://github.com/user-attachments/assets/b7d5438f-538d-4dae-a98f-dc6b1686b535" height = 200>



**Others:**
You will also need your soldering tools/ materials unless you are using the Good display development board.
I also suggest getting FFC cables and connectors (24pin / 0.5mm) so you have more flexibility with your screen and your board since the FPC cable on the display is fairly short.


Case: I just put mine in a 4"*4" photo frame and applied small pieces of VBH tapes to keep thing in places. I bought one that has 1.5cm depth but it's fairly tight so I ended up using a small battery to be able to fit everything inside, I left mine as is since I don't want to waste the frame, but you would probably have better results getting one that is deeper / bigger. Or you can get creative and make your own case! I currently have no plans for a 3D case since I have other priorities. 

<img src ="https://github.com/user-attachments/assets/905f7b32-d680-4e3a-93b4-daad408d5d91" height = 400>


*******

<h2>Software Installation: </h2>
(You may skip this if you know what you are doing)

Software:
- Install Arduino IDE:
https://www.arduino.cc/en/software

Arduino IDE 2.3.2 is the last version at the time of this project which is the version being used. Choose the appropiate option depending on your OS.


- Install the Firebeetle 2 ESP32-C6 board in arduino: (Or relevent boards if you are using others)
https://wiki.dfrobot.com/SKU_DFR1075_FireBeetle_2_Board_ESP32_C6#target_5

- Install the following libraries:

ArduinoJson by bblanchon: https://github.com/bblanchon/ArduinoJson

GxEPD2 by ZinggJM: https://github.com/ZinggJM/GxEPD2

(and GxEPD2_4G if you are using b/w display: https://github.com/ZinggJM/GxEPD2_4G)

u8g2 fonts by olikraus: https://github.com/olikraus/u8g2

Adafruit_GFX by Adafruit: https://github.com/adafruit/Adafruit-GFX-Library




<h2>Connecting the wires:</h2>

- DESPI-C02 ->	ESP32-C6:

- BUSY	->	14

- RES	->	9

- D/C	->	1

- CS	->	18

- SCK	->	23

- SDI	->	22

- GND	->	GND 

- 3V3	->	3V3

(There are two GND and 3V3 on the Firebeetle ESP32-C6, either ones will work.
**Do not feed 5V to the display, most of them operates at 3V3**

Then connect the screen to the connector on the board. Pay attention to the orientation of the screen, usually both the display and the adaptor board should be facing up , but this might be the opposite for some adaptor boards. If screen doesn't work, try the other way round.


- Download the code on this project's main page, (click on <> code -> Download ZIP). 
![download](https://github.com/user-attachments/assets/1ea66c3d-8c26-47a8-86db-87f056e0b4fb)

Extract the zip files and open Weather_Postcard.ino in Arduino IDE, select your board and upload. It will take a while for the code to compile for the first time so please be patient.


<h2>Configuable settings:</h2>

**configurations.h**

Settings with info are found in the header file "configurations.h"

- ssid - your wifi SSID

- password - your wifi password

- wakehour - The hour which the first update of the day starts, in 24 hour time. *Please keep it below 10, otherwise today min max temperature cannot show correctly.

*The min and max temperature of the current day is actually taken from the forecast and not from current weather, and the forecast of the current day disapper sometime after 10-11am, so if the weather postcard has not gotten any data from the HKO since the day changed, it will not have any actual data for the min max for the day (instead, the min max shown will be the min max for the next day.) I have checked HKO's API numberous time, but if you find the min max temperature for the current day elsewhere, let me know and I can adjust the code.

- sleephour - The hour of the last update**, in 24 hour time (e.g. 10pm = 22).

- updateinterval - The frequency which the display is updated, in hours**.

**Update interval precedes sleep hour, so say you have set 3 hour update interval with 8am(8) wakehour and 10pm(22) sleephour, the last update will take place at 11pm.

- tmpClocation: Location of where the current temperature is displayed. Please refer to "locations.h" for the list of avaiable locations.

- rainlocation: Location of where the rainfall of last hour is displayed. Please refer to "locations.h" for the list of avaiable locations. 

- customtext: Any text you would like to display on the last line, though it can't show too many!

- time24h: Display the last update time in 24h or a.m./p.m.

- batterypin / nobattery: whether you are using ADCbatteryread, no need to change if you are using Firebeetle ESP32-C6


**Display Configurations:**

Configuration for the display is in "displayconfig.h"

The default setting is for Firebeetle ESP32-C6 with the 4 color E-Ink display, no need to change anything if you are are using the said combination.

If you are using b/w display with the Firebeetle ESP32-C6, add "//" before #define display_4C and delete the "//" before #define display_BW (There is also display_3C for B/W/R display if you have old panels that you want to use, but it will be missing the yellow colors)

If you are using other microcontroller boards, you need to define your own pins. 


********

This is my first project on github, if you notice anything wrong with the code, let me know, thank you.

********


本天氣明信片以及圖示均為mahbird基於香港天文台的免費資訊制作及繪畫，於Arduino IDE 2.3.2編寫，並於github免費發佈，僅供個人使用，
轉載需保留原訊息，不得用於其他地方，可按需要自行修改但不得作二次發佈，不得用於商業用途。
本程序基於香港天文台的免費API (https://www.hko.gov.hk/tc/weatherAPI/doc/files/HKO_Open_Data_API_Documentation_tc.pdf) (Version 1.11, Date : Nov, 2023)
如未來API有變更，可能令本程序不相容

The Weather postcard and all icons are created and drawn by mahbird on Github, written in Arduino IDE 2.3.2, based on information available from the Hong Kong Observatory. 

The code is distributed for free on github. For personal use only. Please retain all original messages when posting on other sites. You are free to modify this code as you like, but do not redistribute modified code and do not use them elsewhere. Commerical use is not allowed.

Please note that the weather informations are taken from the publicly available Hong Kong Observatory Open Data API (https://www.hko.gov.hk/en/weatherAPI/doc/files/HKO_Open_Data_API_Documentation.pdf) (Version 1.11, Date : Nov, 2023). It it possible that this software may not be compatiable if the API changed in the future.




Special thanks to the developer for the libraries (and the associated libararies in the libraries) used in this project:

ArduinoJson by bblanchon: https://github.com/bblanchon/ArduinoJson

GxEPD2 by ZinggJM: https://github.com/ZinggJM/GxEPD2

U8g2_for_Adafruit_GFX by olikraus: https://github.com/olikraus/U8g2_for_Adafruit_GFX

Adafruit_GFX by Adafruit: https://github.com/adafruit/Adafruit-GFX-Library
