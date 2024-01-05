#include <FastLED.h>
#include <ESPAsyncWebSrv.h>
#define NUM_LEDS 5
#define DATA_PIN 13

#define LED_BUILTIN 1

// Replace with your network credentials
const char* ssid = "OzarkEscape";
const char* password = "Spunky44!";

// Set your Static IP address
// This is what is used to connect to it in ERM
// IMPORTANT: This must be unique or each arduino!
IPAddress local_IP(192, 168, 1, 102);

// Set your Gateway IP address
IPAddress gateway(192, 168, 1, 1);

IPAddress subnet(255, 255, 255, 0);
IPAddress primaryDNS(8, 8, 8, 8);    //optional
IPAddress secondaryDNS(8, 8, 4, 4);  //optional

int pull = 23;
int black = 25;
int blue = 26;
int red = 27;
int yellow = 32;
int reset = 18;
int solve = 2;
int start = 12;




// Define the array of leds
CRGB leds[NUM_LEDS];

// Set web server port number to 80
AsyncWebServer server(80);

bool soundPlay = false;
unsigned int long lastPlay;
int audioLength = 4200;
bool handleERMAudio(bool trigger) {
  if (soundPlay) { soundPlay = false; }
  if (trigger) {
    if (lastPlay + audioLength <= millis()) {
      soundPlay = true;
      lastPlay = millis();
    }
  }
  return soundPlay;
}

bool buttonnoise = true;
void setup() {
    FastLED.addLeds<NEOPIXEL, DATA_PIN>(leds, NUM_LEDS);  // GRB ordering is assumed
    FastLED.setBrightness(128); // Set the overall brightness (0-255)
    
    pinMode(pull, INPUT_PULLUP);
    pinMode(black, INPUT_PULLUP);
    pinMode(blue, INPUT_PULLUP);
    pinMode(red, INPUT_PULLUP);
    pinMode(yellow, INPUT_PULLUP);
    pinMode(reset, INPUT_PULLUP);
    pinMode(start, INPUT_PULLUP);
    pinMode(solve, OUTPUT);
    Serial.begin(115200);

  server.on("/completed", HTTP_GET, [](AsyncWebServerRequest *request){
    AsyncResponseStream *response = request->beginResponseStream("text/html");
    CheckPin(response, digitalRead(solve) == HIGH);
    response->addHeader("Access-Control-Allow-Origin","*");
    request->send(response);
  });

  server.on("/start", HTTP_GET, [](AsyncWebServerRequest *request){
    AsyncResponseStream *response = request->beginResponseStream("text/html");
    CheckPin(response, digitalRead(start) == HIGH);
    response->addHeader("Access-Control-Allow-Origin","*");
    request->send(response);
  });

  server.on("/reset", HTTP_GET, [](AsyncWebServerRequest *request){
    AsyncResponseStream *response = request->beginResponseStream("text/html");
    response->print("resetting prop");
    response->addHeader("Access-Control-Allow-Origin","*");
    request->send(response);
    delay(500);
    ESP.restart();
  });

  server.on("/buttonnoise", HTTP_GET, [](AsyncWebServerRequest *request){
    AsyncResponseStream *response = request->beginResponseStream("text/html");
    CheckPin(response, handleERMAudio(buttonnoise));
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
void loop() {
  int pullState = digitalRead(pull);
  int blackState = digitalRead(black);
  int blueState = digitalRead(blue);
  int redState = digitalRead(red);
  int yellowState = digitalRead(yellow);
  int resetState = digitalRead(reset);

  runWifiLoop();

  /*
  Serial.println(pullState);
  Serial.println(blackState);
  Serial.println(blueState);
  Serial.println(redState);
  Serial.println(yellowState);
  */


  if(resetState == LOW) {
    triggered = false;
  }

  if (pullState == LOW && (blackState == LOW || blueState == LOW || redState == LOW || yellowState == LOW)) {
    buttonnoise = true;
  } else {
    buttonnoise = false;
  }

  if ((pullState == LOW && blackState == HIGH && blueState == LOW && redState == HIGH && yellowState == LOW) || triggered  ) {
    Serial.println("Complete");
    for(int i = 0; i < NUM_LEDS; i++ )
    leds[i] = CRGB::Red;
    FastLED.show();
    if (!triggered) {
      digitalWrite(solve, HIGH);
      delay(5000);
      digitalWrite(solve, LOW);
    }
    triggered = true;
  } else {
    triggered = false;
    leds[0] = CRGB::White;
    FastLED.show();
    delay(500);
    // Now turn the LED off, then pause
    leds[0] = CRGB::Black;
    FastLED.show();
    delay(500);


    leds[4] = CRGB::Red;
    FastLED.show();
    delay(500);
    // Now turn the LED off, then pause
    leds[4] = CRGB::Black;
    FastLED.show();
    delay(250);
  
    leds[3] = CRGB::Red;
    FastLED.show();
    delay(500);
    // Now turn the LED off, then pause
    leds[0] = CRGB::Black;
    FastLED.show();
    delay(500);


    leds[2] = CRGB::Red;
    FastLED.show();
    delay(500);
    // Now turn the LED off, then pause
    leds[0] = CRGB::White;
    FastLED.show();
    delay(500);


    // Turn the LED on, then pause
    leds[0] = CRGB::White;
    FastLED.show();
    delay(500);
    // Now turn the LED off, then pause
    leds[1] = CRGB::Black;
    FastLED.show();
    delay(500);


    leds[4] = CRGB::Red;
    FastLED.show();
    delay(1000);
    // Now turn the LED off, then pause
    leds[2] = CRGB::Black;
    FastLED.show();
    delay(500);


    leds[1] = CRGB::White;
    FastLED.show();
    delay(500);
    // Now turn the LED off, then pause
    leds[3] = CRGB::Black;
    FastLED.show();
    delay(100);


    leds[2] = CRGB::Red;
    FastLED.show();
    delay(500);
    // Now turn the LED off, then pause
    leds[4] = CRGB::Black;
    FastLED.show();
    delay(250);


    leds[4] = CRGB::Red;
    FastLED.show();
    delay(100);
    // Now turn the LED off, then pause
    leds[2] = CRGB::Black;
    FastLED.show();
    delay(500);


    leds[1] = CRGB::White;
    FastLED.show();
    delay(500);
    // Now turn the LED off, then pause
    leds[3] = CRGB::Black;
    FastLED.show();
    delay(100);


    leds[1] = CRGB::Red;
    FastLED.show();
    delay(500);
    // Now turn the LED off, then pause
    leds[4] = CRGB::White;
    FastLED.show();
    delay(250);


    leds[3] = CRGB::White;
    FastLED.show();
    delay(500);
    // Now turn the LED off, then pause
    leds[3] = CRGB::Black;
    FastLED.show();
    delay(100);


    leds[0] = CRGB::Red;
    FastLED.show();
    delay(500);
    // Now turn the LED off, then pause
    leds[4] = CRGB::White;
    FastLED.show();
    delay(250);
  }
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
  response->print("<h3>Starship Down 4 Button Prop.</h3>");
  response->print("<h3>/completed - when prop is triggered.</h3>");
  response->print("<h3>/start - when start button is pushed.</h3>");
  response->print("<h3>/reset - runs ESP.restart();</h3>");
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

