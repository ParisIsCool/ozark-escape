#include <ESPAsyncWebSrv.h>
#define FASTLED_ALLOW_INTERRUPTS 0
#include <FastLED.h>
#define NUM_LEDS 4
#define DATA_PIN 13
#define LED_BUILTIN 1

// Replace with your network credentials
const char* ssid = "OzarkEscape";
const char* password = "Spunky44!";

// Set your Static IP address
// This is what is used to connect to it in ERM
// IMPORTANT: This must be unique or each arduino!
IPAddress local_IP(192, 168, 1, 104);

// Set your Gateway IP address
IPAddress gateway(192, 168, 1, 1);

IPAddress subnet(255, 255, 255, 0);
IPAddress primaryDNS(8, 8, 8, 8);    //optional
IPAddress secondaryDNS(8, 8, 4, 4);  //optional

// Set web server port number to 80
AsyncWebServer server(80);

TaskHandle_t Task1;

void Task1code( void * parameter) {
  while(true) {
    FastLED.show();
    delay(20);
  }
}

// Potentiometer is connected to GPIO 34 (Analog ADC1_CH6)
const int potPin1 = 32;
const int potPin2 = 33;
const int potPin3 = 34;
int relay = 2;


// variable for storing the potentiometer value
int potValue1 = 0;
int potValue2 = 0;
int potValue3 = 0;


//int slider1 =


CRGB leds[NUM_LEDS];


bool triggered = false;
void setup()
{
  pinMode(relay, OUTPUT);
  pinMode(potPin1, INPUT_PULLUP);
  pinMode(potPin2, INPUT_PULLUP);
  pinMode(potPin3, INPUT_PULLUP);
  FastLED.addLeds<NEOPIXEL, DATA_PIN>(leds, NUM_LEDS);
  FastLED.setBrightness(128); // Set the overall brightness (0-255)
  Serial.begin(115200);
  delay(1000);

    xTaskCreatePinnedToCore(
                    Task1code,   /* Task function. */
                    "Task1code",     /* name of task. */
                    10000,       /* Stack size of task */
                    NULL,        /* parameter of the task */
                    1,           /* priority of the task */
                    NULL,      /* Task handle to keep track of created task */
                    1);          /* pin task to core 0 */   

  for(int i = 0; i < NUM_LEDS; i++ ) {
    leds[i] = CRGB::White;
  }

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

  server.on("/reset", HTTP_GET, [](AsyncWebServerRequest *request){
    AsyncResponseStream *response = request->beginResponseStream("text/html");
    response->print("resetting prop");
    response->addHeader("Access-Control-Allow-Origin","*");
    request->send(response);
    delay(500);
    ESP.restart();
  });

  server.onNotFound([](AsyncWebServerRequest *request){
    AsyncResponseStream *response = request->beginResponseStream("text/html");
    GetMainMenu(response);
    request->send(response);
  });

}


void loop()
{

  runWifiLoop();

  // Reading potentiometer value
  Serial.print("POT 1 : ");
  potValue1 = analogRead(potPin1);
  Serial.println(potValue1);
  Serial.print("POT 2 : ");
  potValue2 = analogRead(potPin2);
  Serial.println(potValue2);
  Serial.print("POT 3 : ");
  potValue3 = analogRead(potPin3);
  Serial.println(potValue3);
  delay(30);


  if(potValue1 > 100 && potValue1 < 900 )
  {
  leds[0] = CRGB::Green;
  //FastLED.show();
  delay(30);
  }
  if(potValue1 > 1000 && potValue1 < 2000 )
  {
  leds[0] = CRGB::Red;
  //FastLED.show();
  delay(30);
  }
  else if(potValue1 > 2100 && potValue1 < 3000 )
  {
  leds[0] = CRGB::Blue;
  //FastLED.show();
  delay(30);
  }
  else if(potValue1 > 3100 && potValue1 < 3900 )
  {
  leds[0] = CRGB::White;
  //FastLED.show();
  delay(25);
  }
  else if(potValue1 > 4000)
  {
  leds[0] = CRGB::Orange;
  //FastLED.show();
  delay(25);
  }


 if(potValue2 > 100 && potValue2 < 900 )
  {
  leds[1] = CRGB::Green;
  //FastLED.show();
  delay(25);
  }
  if(potValue2 > 1000 && potValue2 < 2000 )
  {
  leds[1] = CRGB::Red;
  //FastLED.show();
  delay(25);
  }
  else if(potValue2 > 2100 && potValue2 < 3000 )
  {
  leds[1] = CRGB::Blue;
  //FastLED.show();
  delay(25);
  }
  else if(potValue2 > 3100 && potValue2 < 3900 )
  {
  leds[1] = CRGB::White;
  //FastLED.show();
  delay(25);
  }
  else if(potValue2 > 4000)
  {
  leds[1] = CRGB::Orange;
  //FastLED.show();
  delay(25);
  }


  if(potValue3 > 100 && potValue3 < 900 )
  {
  leds[2] = CRGB::Green;
  //FastLED.show();
  delay(25);
  }
  if(potValue3 > 1000 && potValue3 < 2000 )
  {
  leds[2] = CRGB::Red;
  //FastLED.show();
  delay(25);
  }
  else if(potValue3 > 2100 && potValue3 < 3000 )
  {
  leds[2] = CRGB::Blue;
  //FastLED.show();
  delay(25);
  }
  else if(potValue3 > 3100 && potValue3 < 3900 )
  {
  leds[2] = CRGB::White;
  //FastLED.show();
  delay(25);
  }
  else if(potValue3 > 4000)
  {
  leds[2] = CRGB::Orange;
  //FastLED.show();
  delay(25);
  }
  if((potValue1 > 1000 && potValue1 < 2000 && potValue2 > 3100 && potValue2 < 3900 && potValue3 > 4000) || triggered)
  {
  leds[3] = CRGB::Red;
  //FastLED.show();
  delay(25);
  leds[3] = CRGB::Black;
  //FastLED.show();
  delay(25);
  if (!triggered)
  {
    digitalWrite(relay, HIGH);
    delay(1000);
    digitalWrite(relay, LOW);
  }
  triggered = true;
  }
  else
  {
  triggered = false;
  leds[3] = CRGB::Black;
  //FastLED.show();
  delay(25);
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
  response->print("<h3>Starship Down Sliders.</h3>");
  response->print("<h3>/completed - when prop is triggered.</h3>");
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
