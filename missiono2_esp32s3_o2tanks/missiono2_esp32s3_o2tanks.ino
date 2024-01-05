//#define FASTLED_ESP32_I2S 1
#include <FastLED.h>

#define NUM_LEDS 30
#define DATA_PIN 14

// Load Wi-Fi library
#include <Arduino.h>
#include <WiFi.h>

CRGB leds[NUM_LEDS];
int brightness = 255;
#define RGB_BRIGHTNESS 150 // Change white brightness (max 255)

const int numOfCylinders = 5;
const int cylinders[numOfCylinders] = {19,20,47,0,13};
int cylindersStates[numOfCylinders];

// Replace with your network credentials
const char* ssid = "Ozark.Escape";
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
WiFiServer server(80);

void setup() {
  Serial.begin(115200);
  while (!Serial) {
  }
  Serial.println("Starting up...");
  FastLED.addLeds<NEOPIXEL, DATA_PIN>(leds, NUM_LEDS);
  FastLED.setBrightness(500);
  for (int i = 0; i < numOfCylinders; i++) {
    pinMode(cylinders[i], INPUT_PULLUP);
    cylindersStates[i] = (digitalRead(cylinders[i]) * -1) + 1;
  }
  for(int i = 0; i < NUM_LEDS; i++ ) {
    Serial.println(i);
    leds[i] = CRGB::Black;
  }
  pinMode(LED_BUILTIN, OUTPUT);
  //pinMode(RGB_BUILTIN, OUTPUT);
  //digitalWrite(RGB_BUILTIN, HIGH);
}

void SetLightBarLights(int n, bool triggered) {
  if (triggered) {
    for(int i = 5*(n+1); i < 5*(n+2); i++ ) {
      //leds[i] = CRGB::Red;
    }
  } else {
    for(int i = 5*(n+1); i < 5*(n+2); i++ ) {
      //leds[i] = CRGB::Black;
    }
  }
}


bool triggered = false;
bool soundPlay = false;
void loop() {
  triggered = true; // next logic will set false if not triggered
  for (int i = 0; i < numOfCylinders; i++) {
    int prestate = cylindersStates[i];
    int state = digitalRead(cylinders[i]);
    if (state == LOW && prestate == HIGH) {
      //leds[i] = CRGB::Green;
      Serial.print("Cylinder ");
      Serial.print(i);
      Serial.print(" (pin ");
      Serial.print(cylinders[i]);
      Serial.println(") has been added.");
      //SetLightBarLights(i, true);
      Serial.println("got here");
      FastLED.show();
      Serial.println("got here me");
      soundPlay = true;
      Serial.println("got here too");
    } else if (state == HIGH) {
      if (prestate == LOW) {
        //leds[i] = CRGB::Black;
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
        //SetLightBarLights(i, false);
        FastLED.show();
      }
      triggered = false;
    }
    cylindersStates[i] = state;
  }
  runWifiLoop();
  delay(15);
}

void CheckPin(WiFiClient& client, bool changed) {
  String status = (changed) ? "triggered" : "not triggered";
  client.println("HTTP/1.1 200 OK");
  client.println("Content-Type: text/plain");
  client.println("Access-Control-Allow-Origin: *");  // ERM will not be able to connect without this header!
  client.println();
  client.print(status);
}

// Actual request handler
void processRequest(WiFiClient& client, String requestStr) {
  // Send back different response based on request string
  if (requestStr.startsWith("GET /completed")) {
    CheckPin(client, triggered);
  } else if (requestStr.startsWith("GET /sound")) {
    CheckPin(client, soundPlay);
    if (soundPlay) { soundPlay = false; }
  } else {  // when ip is called with no request
    Serial.println("Polled.");
    client.println("HTTP/1.1 200 OK");
    client.println("Content-Type: text/html");
    client.println();
    client.println("<!DOCTYPE HTML>");
    client.println("<html>");
    client.println("<head>");
    client.println("<style>");
    client.println("body { background-color: #111111; font-family:Roboto }");
    client.println("h1 { color: #ffffff; }");
    client.println("h3 { color: #999999; }");
    client.println("</style>");
    client.println("</head>");
    client.println("<body {background-color: #111111;}>");
    client.println("<img src='https://static.wixstatic.com/media/6e6fcf_230f10c631da4717a2d87b0e96cd93f9~mv2_d_8001_4178_s_4_2.png/v1/crop/x_2,y_943,w_7999,h_2240/fill/w_489,h_137,al_c,q_85,usm_0.66_1.00_0.01,enc_auto/OE8_Primrary2_White.png' alt='Wix.com'>");
    client.println("<h1>Arduino to Escape Room Master</h1>");
    client.println("<h1>By Ryan</h1>");
    client.println("<h3>I'm working just fine.</h3>");
    client.println("<h3>Mission O2 Tanks. /completed /sound</h3>");
    client.println("</body>");
    client.println("</html>");
  }
}

// DO NOT TOUCH
// If having trouble connecting, signal may be weak.
void listenForWifiClients() {
  // listen for incoming clients
  WiFiClient client = server.available();
  if (client) {
    // Grab the first HTTP header (GET /status HTTP/1.1)
    String requestStr;
    boolean firstLine = true;
    // an http request ends with a blank line
    boolean currentLineIsBlank = true;
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        // if you've gotten to the end of the line (received a newline
        // character) and the line is blank, the http request has ended,
        // so you can send a reply
        if (c == '\n' && currentLineIsBlank) {
          processRequest(client, requestStr);
          break;
        }
        if (c == '\n') {
          // you're starting a new line
          currentLineIsBlank = true;
          firstLine = false;
        } else if (c != '\r') {
          // you've gotten a character on the current line
          currentLineIsBlank = false;

          if (firstLine) {
            requestStr.concat(c);
          }
        }
      }
    }
    // give the web browser time to receive the data
    delay(50);  // change based off lag.
    // close the connection:
    client.stop();
  }
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
    listenForWifiClients();
  } else {
    attemptWifiConnection();
  }
}