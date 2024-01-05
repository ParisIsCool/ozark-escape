#define FASTLED_ALLOW_INTERRUPTS 0
#include <FastLED.h>
#include <ESPAsyncWebSrv.h>

#define NUM_LEDS 30
#define DATA_PIN 12

// Load Wi-Fi library
#include <Arduino.h>
#include <WiFi.h>

CRGB leds[NUM_LEDS];
int brightness = 255;
#define RGB_BRIGHTNESS 150 // Change white brightness (max 255)

const int numOfCylinders = 5;
const int cylinders[numOfCylinders] = {19,18,5,4,15};
int cylindersStates[numOfCylinders];

// Replace with your network credentials
const char* ssid = "Ozark Escape";
const char* password = "0zark3scap3";

// Set your Static IP address
// This is what is used to connect to it in ERM
// IMPORTANT: This must be unique or each arduino!
IPAddress local_IP(192, 168, 0, 92);


// Set your Gateway IP address
IPAddress gateway(192, 168, 0, 1);

IPAddress subnet(255, 255, 255, 0);
IPAddress primaryDNS(8, 8, 8, 8);    //optional
IPAddress secondaryDNS(8, 8, 4, 4);  //optional

// Set web server port number to 80
AsyncWebServer server(80);

TaskHandle_t Task1;

void Task1code( void * parameter) {
  while(true) {
    Serial.print(".");
    FastLED.show();
    //delay(20);
  }
}

bool soundPlay = false;
bool triggered = false;
void setup() {
  Serial.begin(115200);
  while (!Serial) {
  }
  Serial.println("Starting up...");
  FastLED.addLeds<NEOPIXEL, DATA_PIN>(leds, NUM_LEDS);
  FastLED.setBrightness(255);
  for (int i = 0; i < numOfCylinders; i++) {
    pinMode(cylinders[i], INPUT_PULLUP);
    cylindersStates[i] = (digitalRead(cylinders[i]) * -1) + 1;
  }
  for(int i = 0; i < NUM_LEDS; i++ ) {
    leds[i] = CRGB::Black;
    FastLED.show();
  }
  pinMode(LED_BUILTIN, OUTPUT);

  xTaskCreatePinnedToCore(
                    Task1code,   /* Task function. */
                    "Task1code",     /* name of task. */
                    10000,       /* Stack size of task */
                    NULL,        /* parameter of the task */
                    1,           /* priority of the task */
                    NULL,      /* Task handle to keep track of created task */
                    1);          /* pin task to core 0 */   

  server.on("/sound", HTTP_GET, [](AsyncWebServerRequest *request){
    AsyncResponseStream *response = request->beginResponseStream("text/html");
    CheckPin(response, soundPlay);
    if (soundPlay) { soundPlay = false; }
    response->addHeader("Access-Control-Allow-Origin","*");
    request->send(response);
  });

  server.on("/completed", HTTP_GET, [](AsyncWebServerRequest *request){
    AsyncResponseStream *response = request->beginResponseStream("text/html");
    CheckPin(response, triggered);
    response->addHeader("Access-Control-Allow-Origin","*");
    request->send(response);
  });

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    AsyncResponseStream *response = request->beginResponseStream("text/html");
    GetMainMenu(response);
    request->send(response);
  });

  server.onNotFound([](AsyncWebServerRequest *request){
    AsyncResponseStream *response = request->beginResponseStream("text/html");
    GetMainMenu(response);
    request->send(response);
  });
}

void SetLightBarLights(int n, bool triggered) {
  if (triggered) {
    for(int i = 5*(n+1); i < 5*(n+2); i++ ) {
      leds[i] = CRGB::Red;
    }
  } else {
    for(int i = 5*(n+1); i < 5*(n+2); i++ ) {
      leds[i] = CRGB::Black;
    }
  }
}

void loop() {
  triggered = true; // next logic will set false if not triggered
  for (int i = 0; i < numOfCylinders; i++) {
    int prestate = cylindersStates[i];
    int state = digitalRead(cylinders[i]);
    if (state == LOW && prestate == HIGH) {
      leds[i] = CRGB::Green;
      Serial.print("Cylinder ");
      Serial.print(i);
      Serial.print(" (pin ");
      Serial.print(cylinders[i]);
      Serial.println(") has been added.");
      SetLightBarLights(i, true);
      soundPlay = true;
    } else if (state == HIGH) {
      if (prestate == LOW) {
        leds[i] = CRGB::Black;
        Serial.print("Cylinder ");
        Serial.print(i);
        Serial.print(" (pin ");
        Serial.print(cylinders[i]);
        Serial.println(") has been removed.");
        if (triggered) { // fix bug, lets invalidate all to reset!
          for (int a = 0; a < numOfCylinders; a++) {
            cylindersStates[a] = (digitalRead(cylinders[a]) * -1) + 1;
          }
        }
        SetLightBarLights(i, false);
      }
      triggered = false;
    }
    cylindersStates[i] = state;
  }
  if (triggered) {
    soundPlay = false;
  }
  runWifiLoop();
  delay(25);
}

void CheckPin(AsyncResponseStream *response, bool changed) {
  String status = (changed) ? "triggered" : "not triggered";
  response->print(status);
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
  response->print("<h1>Arduino to Escape Room Master</h1>");
  response->print("<h1>By Ryan</h1>");
  response->print("<h3>I'm working just fine.</h3>");
  response->print("<h3>Mission O2 Tanks. /completed /sound</h3>");
  response->print("</body>");
  response->print("</html>");
}



void attemptWifiConnection() {
    // Configures static IP address
  if (!WiFi.config(local_IP, gateway, subnet, primaryDNS, secondaryDNS)) {
    Serial.println("STA Failed to configure, DHCP error! Using random assigned IP!");
    Serial.println("Please update router settings to allow DHCP and static IP.");
  }

  // Connect to Wi-Fi network with SSID and password
  Serial.print("\n\n\nConnecting to ");
  Serial.println(ssid);
  WiFi.mode(WIFI_STA);
  WiFi.setSleep(WIFI_PS_NONE);
  WiFi.begin(ssid, password);
  bool blink = false;
  int timeout = millis() + 10000;
  while ( !(WiFi.status() == WL_CONNECTED || timeout < millis() ) ) {
    delay(100);  // LED BLINKS WHEN TRYING TO CONNECT
    if (blink) {
      digitalWrite(LED_BUILTIN, HIGH);
      blink = false;
    } else {
      digitalWrite(LED_BUILTIN, LOW);
      blink = true;
    }
    Serial.print(".");
  }
  // Print local IP address and start web server
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("");
    Serial.println("WiFi Failed to connect. Retrying.");
  } else {
    Serial.println("");
    Serial.println("WiFi connected.");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
    server.begin();
    // BLINK A FEW TIMES FAST TO SIGNIFIY INITIALIZATION COMPLETE
    digitalWrite(LED_BUILTIN, LOW);
    delay(50);
    digitalWrite(LED_BUILTIN, HIGH);
    delay(50);
    digitalWrite(LED_BUILTIN, LOW);
    delay(50);
    digitalWrite(LED_BUILTIN, HIGH);
  }
}

void runWifiLoop() {
  if (WiFi.status() == WL_CONNECTED) {
    //listenForWifiClients();
  } else {
    attemptWifiConnection();
  }
}