#include <FastLED.h>
#include <ESPAsyncWebSrv.h>

#define NUM_LEDS 11
#define DATA_PIN 13
#define LED_BUILTIN 2
#define RELAY 4
#define SWITCH 17

// Load Wi-Fi library
#include <Arduino.h>
#include <WiFi.h>

CRGB leds[NUM_LEDS];

const int numOfCylinders = 5;
const int cylinders[numOfCylinders] = {32,33,25,26,27};
const int lights[numOfCylinders] = {0,2,4,6,8};
int cylindersStates[numOfCylinders];

// Replace with your network credentials
const char* ssid = "OzarkEscape";
const char* password = "Spunky44!";

// Set your Static IP address
// This is what is used to connect to it in ERM
// IMPORTANT: This must be unique or each arduino!
IPAddress local_IP(192, 168, 1, 100);

// Set your Gateway IP address
IPAddress gateway(192, 168, 0, 1);
IPAddress subnet(255, 255, 255, 0);
IPAddress primaryDNS(8, 8, 8, 8);    //optional
IPAddress secondaryDNS(8, 8, 4, 4);  //optional

// Set web server port number to 80
AsyncWebServer server(80);

bool soundPlay = false;
bool triggered_real = false;
void setup() {
  Serial.begin(9600);
  while (!Serial) {
  }
  Serial.println("Starting up...");
  FastLED.addLeds<WS2812B, DATA_PIN, RGB>(leds, NUM_LEDS);
  FastLED.setBrightness(100);
  for (int i = 0; i < numOfCylinders; i++) {
    pinMode(cylinders[i], INPUT_PULLUP);
  }
  for(int i = 0; i < NUM_LEDS; i++ ) {
    leds[i] = CRGB::Red;
    FastLED.show();
  }
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(RELAY, OUTPUT);
  pinMode(SWITCH, INPUT_PULLUP);

  server.on("/sound", HTTP_GET, [](AsyncWebServerRequest *request){
    AsyncResponseStream *response = request->beginResponseStream("text/html");
    CheckPin(response, soundPlay);
    if (soundPlay) { soundPlay = false; }
    response->addHeader("Access-Control-Allow-Origin","*");
    request->send(response);
  });

  server.on("/completed", HTTP_GET, [](AsyncWebServerRequest *request){
    AsyncResponseStream *response = request->beginResponseStream("text/html");
    CheckPin(response, triggered_real);
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

bool triggered = false;
bool flash = true;
unsigned long flashDelay = 0;
void loop() {
  runWifiLoop();
  triggered = true; // next logic will set false if not triggered
  for (int i = 0; i < numOfCylinders; i++) {
    int prestate = cylindersStates[i];
    int state = digitalRead(cylinders[i]);
    if (state == LOW && prestate == HIGH) {
      soundPlay = true;
      leds[lights[i]] = CRGB::Green;
      Serial.print("Cylinder ");
      Serial.print(i);
      Serial.print(" (pin ");
      Serial.print(cylinders[i]);
      Serial.println(") has been added");
    } else if (state == HIGH && prestate == LOW) {
      soundPlay = false;
      leds[lights[i]] = CRGB::Red;
      triggered = false;
      Serial.print("Cylinder ");
      Serial.print(i);
      Serial.print(" (pin ");
      Serial.print(cylinders[i]);
      Serial.println(") has been removed");
    } else if (state == HIGH) {
      soundPlay = false;
      triggered = false;
    }
    cylindersStates[i] = state;
  }
  if (digitalRead(SWITCH) == HIGH) {
    triggered = false;
  }
  triggered_real = triggered;
  if (triggered) { 
    soundPlay = false;
    digitalWrite(RELAY, HIGH);
    if (flashDelay < millis()) {
      flashDelay = millis() + 1000;
      for (int i = 0; i < numOfCylinders; i++) {
        if (flash) {
          leds[lights[i]] = CRGB::White;
          flash = false;
        } else {
          leds[lights[i]] = CRGB::Black;
          flash = true;
        }
      }
    }
  } else {
    if (digitalRead(RELAY) == HIGH) {
      digitalWrite(RELAY, LOW);
      Serial.println("Puzzle reset, invalidating states.");
      for (int i = 0; i < numOfCylinders; i++) {
        cylindersStates[i] = HIGH;
      }
    }
  }
  FastLED.show();
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
  response->print("<h3>Starship Down Tanks.</h3>");
  response->print("<h3>/completed is changed when all tanks added</h3>");
  response->print("<h3>/sound is changed when 1 tank added</h3>");
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
  //esp_wifi_set_ps(WIFI_PS_NONE);
  esp_sleep_pd_config(ESP_PD_DOMAIN_MAX,ESP_PD_OPTION_OFF);
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