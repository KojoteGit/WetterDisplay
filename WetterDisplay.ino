#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <LiquidCrystal_I2C.h>
#include <JC_Button.h>

#define SSID  "<YOUR WLAN SSID>"
#define WLAN_KEY "<YOUR WLAN KEY>"
#define PHPURL "<URL OF PHP SCRIPT>"

#define TIMER_INTERVAL_LIGHT 600000 // 10 min
#define TIMER_INTERVAL_UPDATE 600000 //10 min
#define TIMER_INTERVAL_PAGE 5000 // 5 sec

#define SEPARATOR "#"

#define HOST_NAME "WetterDisplay" //can be changed to another name

#define BRIGHTNESS 512 //max value is 1024; PWM to set brightness
#define BRIGHTNESS_PIN D6

//---------------------------
//defining needed variables
//---------------------------

//current raw data
String currentDataRaw;

// Timestamps
unsigned long time_now;
unsigned long lastTimeStampLight=0;
unsigned long lastTimeStampUpdate=0;
unsigned long lastTimeStampPage=0;

// Fields for values
String currentTemperatureBaseStation;
String currentTemperatureOutside;
String CoTwo;
String humidy_inside;
String humidy_outside;
String rain_last_hour;
String rain_last_24hours;

//Enum for display
enum pages {
  page1=1,
  page2=2,
  page3=3
};

boolean backlightTimerActive = false;

int currentPage=0; //current Page

bool backlight = true; //state background light

Button myBtn(D5);       // define the button

//Display
LiquidCrystal_I2C lcd(0x27,16,2);


// Remote debug over telnet - not recommended for production/release, only for development

// Disable all debug ?

// Important to compile for prodution/release
// Disable all debug ? Good to release builds (production)
// as nothing of RemoteDebug is compiled, zero overhead :-)
// For it just uncomment the DEBUG_DISABLED
// On change it, if in another IDE than Arduino IDE, like Eclipse or VSCODE,
// please clean the project, before compile

//#define DEBUG_DISABLED true

// Disable the auto function feature of RemoteDebug ?

//#define DEBUG_DISABLE_AUTO_FUNC true

#include "RemoteDebug.h"        //https://github.com/JoaoLopesF/RemoteDebug

#ifndef DEBUG_DISABLED // Only if debug is not disabled (for production/release)

// Instance of RemoteDebug

RemoteDebug Debug;

#endif

// Setup routine
void setup() {
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  WiFi.persistent(false);
  WiFi.begin(SSID, WLAN_KEY);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  };
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  // initialze LCD Display
  lcd.init();
  lcd.begin(16, 2);
  switchBacklightOn();
  analogWrite(BRIGHTNESS_PIN, BRIGHTNESS);

  myBtn.begin();
  lastTimeStampPage=millis();
  lastTimeStampLight=millis();
  lastTimeStampUpdate=millis();

  currentDataRaw = getData();
  analyseData();

#ifndef DEBUG_DISABLED // Only for development

    // Initialize the WiFi server of RemoteDebug

    Debug.begin(HOST_NAME); // Initiaze the WiFi server

    //Debug.setPassword("r3m0t0."); // Password of WiFi (as telnet) connection ?

    Debug.setResetCmdEnabled(true); // Enable the reset command

    Debug.showProfiler(true); // Profiler (Good to measure times, to optimize codes)

    Debug.showColors(true); // Colors

    Debug.setSerialEnabled(true); // if you wants serial echo - only recommended if ESP is plugged in USB

#endif

}

// Loop method
void loop() {

#ifndef DEBUG_DISABLED
    // RemoteDebug handle (for WiFi connections)
    Debug.handle();
#endif

time_now = millis();
myBtn.read(); // Check button

if (myBtn.wasReleased()) {
  switchBacklightOn();
}

if (checkTimerPage(time_now)) {
  nextPage();
  displayPage((pages) currentPage);
  lastTimeStampPage=time_now;
}

if (backlightTimerActive && checkTimerLight(time_now)) {
  switchBacklightOff();
  lastTimeStampLight=time_now;
}

if (checkTimerUpdate(time_now)) {
  currentDataRaw = getData();
  analyseData();
  lastTimeStampUpdate=time_now;
}
  yield();
}

/* This method requests the data from PHP script
*  @return current data in raw format
*/
String getData() {
  String returnValue;
  HTTPClient httpClient;

  httpClient.begin(PHPURL);
  if (httpClient.GET() > 0)
  {
   returnValue = httpClient.getString();
   rprintDln("GetData: " + returnValue);
  }

  return returnValue;
};

/*
* analysis of data - split raw data into Fields
*/
void analyseData()
{
  rprintDln("current Data: " + currentDataRaw);

  int positionOfSeparator[8] = {-1, -1, -1, -1, -1, -1, -1, -1};
  int currentPos=-1;

  for (int i = 0; i < sizeof(positionOfSeparator)/sizeof(int); i++) {
    currentPos = currentDataRaw.indexOf(SEPARATOR, currentPos+1);
    positionOfSeparator[i] = currentPos;
  }

  currentTemperatureBaseStation =
    currentDataRaw.substring(positionOfSeparator[0]+1,
       positionOfSeparator[1]);

  currentTemperatureOutside = currentDataRaw.substring(positionOfSeparator[1]+1, positionOfSeparator[2]);

  CoTwo = currentDataRaw.substring(positionOfSeparator[2]+1, positionOfSeparator[3]);

  humidy_inside = currentDataRaw.substring(positionOfSeparator[3]+1, positionOfSeparator[4]);

  humidy_outside = currentDataRaw.substring(positionOfSeparator[4]+1, positionOfSeparator[5]);

  rain_last_hour = currentDataRaw.substring(positionOfSeparator[5]+1, positionOfSeparator[6]);

  rain_last_24hours = currentDataRaw.substring(positionOfSeparator[6]+1, positionOfSeparator[7]);

}

/*
* print values for debugging purposes
*/
void printValues() {
  rprintDln("Temp innen: " +currentTemperatureBaseStation);
  rprintDln("Temp außen: " +currentTemperatureOutside);
  rprintDln("CO2: " +CoTwo);
  rprintDln("Luftfeuchtigkeit innen: " +humidy_inside);
  rprintDln("Luftfeuchtigkeit außen: " +humidy_outside);
  rprintDln("Regen 1h: " +rain_last_hour);
  rprintDln("Regen 24h: " +rain_last_24hours);

}

/*
* print value to display
* @pages current page for displaying
*/
void displayPage(pages p) {
  rdebugD("pages p %i \n", p);
  char lineOne[16];
  char lineTwo[16];
  lcd.clear();

  switch (p) {
    case page1:
      sprintf(lineOne, "Innen: %s C", currentTemperatureBaseStation.c_str());
      sprintf(lineTwo, "Aussen: %s C", currentTemperatureOutside.c_str());
      break;
    case page2:
      sprintf(lineOne, "Innen: %s %%", humidy_inside.c_str());
      sprintf(lineTwo, "Aussen: %s %%", humidy_outside.c_str());
      break;
    case page3:
      sprintf(lineOne, "R 1h: %s mm", rain_last_hour.c_str());
      sprintf(lineTwo, "R 24h: %s mm", rain_last_24hours.c_str());
      break;
    default:
      sprintf(lineOne, "WetterDisplay");
      sprintf(lineTwo, "By KojoteGit");
      break;
  }
rprintDln(lineOne);
rprintDln(lineTwo);

lcd.setCursor(0, 0);
  lcd.print(lineOne);
  lcd.setCursor(0, 1);
  lcd.print(lineTwo);
}

/*
* switch backlight on or off based on parameter
* @active true or false
*/
void switchBacklight(boolean active) {
  if (active) {
    lcd.backlight();
  }
  else {
    lcd.noBacklight();
  }
}

/*
* switch backlight of display on
*/
void switchBacklightOn() {
  backlight = true;
  switchBacklight(true);
  lastTimeStampLight = millis();
  backlightTimerActive = true;
}

/*
* switch backlight of display off
*/
void switchBacklightOff() {
  backlight = false;
  switchBacklight(false);
  backlightTimerActive = false;
}

/*
* increase page number for display
*/
void nextPage() {
  if (currentPage <3) {
    currentPage++;
  }
  else {
    currentPage=1;
  }
  rdebugD("nächste Seite %i \n" ,currentPage);
}

/*
* execute a check if backlight should be switched off
* @currentTimeStamp current timeStamp
*/
boolean checkTimerLight(unsigned long currentTimeStamp) {
  if (currentTimeStamp - lastTimeStampLight < TIMER_INTERVAL_LIGHT) {
    return false;
  } else {
    return true;
  }
}

/*
* execute a check if page should be switched
* @currentTimeStamp current timeStamp
*/
boolean checkTimerPage(unsigned long currentTimeStamp) {
  if (currentTimeStamp - lastTimeStampPage < TIMER_INTERVAL_PAGE) {
    return false;
  }
  else {
    return true;
  }
}

/*
* execute a check if new data should be retrieved
* @currentTimeStamp current timeStamp
*/
boolean checkTimerUpdate(unsigned long currentTimeStamp) {
  if (currentTimeStamp - lastTimeStampUpdate < TIMER_INTERVAL_UPDATE) {
    return false;
  }
  else {
    return true;
  }
}
