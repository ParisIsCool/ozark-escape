#include "config.h"
#include <WiFi.h>
#include <soc/rtc.h>
#include <time.h>
#include <ESPAsyncWebSrv.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include "watch_websocket.h"
//#include "wifi_config.h"

// CONFIG 
const char *ssid = "Ozark Escape";
const char *password = "0zark3scap3";
//IPAddress
IPAddress staticIP(192, 168, 0, 120);
IPAddress gateway(192, 168, 0, 1);
IPAddress subnet(255, 255, 255, 0);
IPAddress dns(8, 8, 8, 8);
IPAddress dns2(8, 8, 4, 4);


const long utcOffsetInSeconds = 0;       // Set your time zone offset in seconds here
const char *NTPServer = "pool.ntp.org";  // Use a reliable NTP server

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, NTPServer, utcOffsetInSeconds);

byte xcolon = 0;  // location of the colon

TTGOClass *ttgo = nullptr;
// Custom time zone offset in seconds (CST: UTC-6, CDT: UTC-5)
const int cstOffset = -6 * 3600;  // 6 hours in seconds

uint32_t targetTime = 0;           // for next 1 second display update
uint8_t hh, mm, ss, mmonth, dday;  // H, M, S variables
uint16_t yyear;                    // Year is 16 bit int


// Set web server port number to 80
AsyncWebServer server(80);

bool triggerClue = false;

void setup() {
  Serial.begin(115200);
  Serial.println("Booting...");

  for (int i = 0; i < MAX_ROOMS; i++) {
    rooms[i].name = "";
  }

  // Get Watch Instance
  ttgo = TTGOClass::getWatch();

  // Initialize watch
  ttgo->begin();
  // Register lvgl
  ttgo->lvgl_begin();
  // Turn on the backlight
  ttgo->openBL();

  server.on("/clue", HTTP_GET, [](AsyncWebServerRequest *request) {
    AsyncResponseStream *response = request->beginResponseStream("text/html");
    String status = (triggerClue) ? "triggered" : "not triggered";
    response->print(status);
    response->addHeader("Access-Control-Allow-Origin", "*");
    request->send(response);
  });

  server.on("/force", HTTP_GET, [](AsyncWebServerRequest *request) {
    AsyncResponseStream *response = request->beginResponseStream("text/html");
    response->print("forcing clues on all available rooms");
    response->addHeader("Access-Control-Allow-Origin", "*");
    request->send(response);
    webSocket.sendTXT(WStype_TEXT,"Clue*");
  });

  // CONFIGURE HOME PAGES
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    AsyncResponseStream *response = request->beginResponseStream("text/html");
    GetMainMenu(response);
    request->send(response);
  });
  server.onNotFound([](AsyncWebServerRequest *request) {
    AsyncResponseStream *response = request->beginResponseStream("text/html");
    GetMainMenu(response);
    request->send(response);
  });

  SetupWatchTasks();
  
}

void displayTime(boolean fullUpdate) {

  byte xpos = 5;  // Stating position for the display
  byte ypos = 5;

  // Update NTP time
  timeClient.update();

  // Get the synchronized time
  time_t epochTime = static_cast<time_t>(timeClient.getEpochTime() + utcOffsetInSeconds);
  struct tm localTime;
  localtime_r(&epochTime, &localTime);  // Use localtime_r to convert to local time

  // Adjust the time for DST (starts on the 2nd Sunday in March, ends on the 1st Sunday in November)
  if (localTime.tm_mon >= 2 && localTime.tm_mon <= 10) {
    // DST is active (March to November)
    localTime.tm_hour += 1;  // Add 1 hour for DST
  }

  // Update your local time variables
  hh = localTime.tm_hour;
  mm = localTime.tm_min;
  ss = localTime.tm_sec;
  dday = localTime.tm_mday;
  mmonth = localTime.tm_mon + 1;     // tm_mon is 0-based, so add 1 to get the month number.
  yyear = localTime.tm_year + 1900;  // tm_year is years since 1900, so add 1900 to get the current year.

  ttgo->tft->setTextSize(1);

  if (fullUpdate) {
    // Font 7 is a 7-seg display but only contains
    // characters [space] 0 1 2 3 4 5 6 7 8 9 0 : .

    ttgo->tft->setTextColor(0x39C4, TFT_BLACK);  // Set desired color
    ttgo->tft->drawString("88:88", xpos, ypos, 7);
    ttgo->tft->setTextColor(0xFFFF, TFT_BLACK);  // Orange

    if (hh < 10) xpos += ttgo->tft->drawChar('0', xpos, ypos, 7);
    xpos += ttgo->tft->drawNumber(hh, xpos, ypos, 7);
    xcolon = xpos + 3;
    xpos += ttgo->tft->drawChar(':', xcolon, ypos, 7);
    if (mm < 10) xpos += ttgo->tft->drawChar('0', xpos, ypos, 7);
    ttgo->tft->drawNumber(mm, xpos, ypos, 7);

    ttgo->power->adc1Enable(AXP202_VBUS_VOL_ADC1 | AXP202_VBUS_CUR_ADC1 | AXP202_BATT_CUR_ADC1 | AXP202_BATT_VOL_ADC1, true);
    int per = ttgo->power->getBattPercentage();
    ttgo->tft->setTextSize(2);
    ttgo->tft->setTextColor(TFT_WHITE, TFT_BLACK);
    ttgo->tft->setCursor(160,10);
    String batteryInfo = String(per) + "%";
    ttgo->tft->println(batteryInfo);

  }

  ttgo->tft->setTextSize(1);
  if (ss % 2) {  // Toggle the colon every second
    ttgo->tft->setTextColor(0x39C4, TFT_BLACK);
    xpos += ttgo->tft->drawChar(':', xcolon, ypos, 7);
    ttgo->tft->setTextColor(0xFFFF, TFT_BLACK);
  } else {
    ttgo->tft->drawChar(':', xcolon, ypos, 7);
  }
}


void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    attemptWifiConnection();
  } else {
    webSocket.loop();
  }
  LoopWatchTasks();
  //loopNetConfig();
  if (targetTime < millis()) {
    targetTime = millis() + 1000;
    displayTime(ss == 0);  // Call every second but only update time every minute
  }
  delay(100);
}

void GetMainMenu(AsyncResponseStream *response) {
  //Serial.println("Polled.");
  response->print("<!DOCTYPE HTML>");
  response->print("<html>");
  response->print("<head>");
  response->print("<style>");
  response->print("body { background-color: #111111; font-family:Roboto }");
  response->print("h1 { color: #ffffff; }");
  response->print("h3 { color: #999999; }");
  response->print("</style>");
  response->print("</head>");
  response->print("<body {background-color: #111111;}>");
  response->print("<img src='https://static.wixstatic.com/media/6e6fcf_230f10c631da4717a2d87b0e96cd93f9~mv2_d_8001_4178_s_4_2.png/v1/crop/x_2,y_943,w_7999,h_2240/fill/w_489,h_137,al_c,q_85,usm_0.66_1.00_0.01,enc_auto/OE8_Primrary2_White.png' alt='Wix.com'>");
  response->print("<h1>ESP32 T-Watch v1</h1>");
  response->print("<h3>Experimental</h3>");
  response->print("<h3>By Ryan</h3>");
}

void attemptWifiConnection() {
  // Register wifi events
  Serial.println("Connecting to ");
  Serial.print(ssid);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  if (WiFi.config(staticIP, gateway, subnet, dns, dns2) == false) {
    Serial.println("Configuration failed.");
  }
  int timeout = 10000 + millis();
  while (WiFi.status() != WL_CONNECTED && millis() > timeout) {
    Serial.print(".");
    delay(50);
  }
  if (WiFi.status() == WL_CONNECTED)
  Serial.println();
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  // Initialize NTP client
  timeClient.begin();
  timeClient.update();

  // Set the NTP time zone offset
  timeClient.setTimeOffset(cstOffset);

  ttgo->rtc->check();

  webSocket.begin();
  webSocket.onEvent(webSocketEvent);

  server.begin();
}