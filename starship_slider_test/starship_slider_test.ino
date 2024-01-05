#include <ESPAsyncWebSrv.h>
#include <FastLED.h>

#define NUM_LEDS 4
#define DATA_PIN 13
#define LED_BUILTIN 1

const char* ssid = "OzarkEscape";
const char* password = "Spunky44!";
IPAddress local_IP(192, 168, 1, 104);
IPAddress gateway(192, 168, 1, 1);
IPAddress subnet(255, 255, 255, 0);
IPAddress primaryDNS(8, 8, 8, 8);    
IPAddress secondaryDNS(8, 8, 4, 4);

AsyncWebServer server(80);

CRGB leds[NUM_LEDS];
int relay = 2;

#define potPin1 32
#define potPin2 33
#define potPin3 34
int potValue1 = 0;
int potValue2 = 0;
int potValue3 = 0;

bool indicatorOn = false;
unsigned long indicatorStartTime = 0;
const unsigned long indicatorInterval = 300; // Interval for the indicator LED (0.3 second)

bool triggered = false;

void setup()
{
  pinMode(relay, OUTPUT);
  pinMode(potPin1, INPUT_PULLUP);
  pinMode(potPin2, INPUT_PULLUP);
  pinMode(potPin3, INPUT_PULLUP);
  
  FastLED.addLeds<WS2812, DATA_PIN, RGB>(leds, NUM_LEDS);
  FastLED.setBrightness(64);
  
  Serial.begin(115200);
  delay(1000);

  xTaskCreatePinnedToCore(
    Task1code,
    "Task1code",
    10000,
    NULL,
    1,
    NULL,
    1
  );

  for (int i = 0; i < NUM_LEDS; i++)
  {
    leds[i] = CRGB::White;
  }

  server.on("/completed", HTTP_GET, [](AsyncWebServerRequest *request){
    AsyncResponseStream *response = request->beginResponseStream("text/html");
    CheckPin(response, triggered);
    response->addHeader("Access-Control-Allow-Origin", "*");
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
    response->addHeader("Access-Control-Allow-Origin", "*");
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

  potValue1 = analogRead(potPin1);
  potValue2 = analogRead(potPin2);
  potValue3 = analogRead(potPin3);

  Serial.print("Pot Values: ");
  Serial.print(potValue1);
  Serial.print(", ");
  Serial.print(potValue2);
  Serial.print(", ");
  Serial.print(potValue3);
  Serial.print(". ");
  Serial.println(indicatorStartTime);

  updateLEDColor(0, potValue1);
  updateLEDColor(1, potValue2);
  updateLEDColor(2, potValue3);

  if((potValue1 > 2048 && potValue1 < 2560 && potValue2 > 3584 && potValue3 > 1536 && potValue3 < 2048) || triggered) {
    if (!triggered) {
      digitalWrite(relay, HIGH);
      delay(1000);
      digitalWrite(relay, LOW);
    }
    triggered = true;
    if (indicatorOn) {
      if (indicatorStartTime < millis()) { indicatorOn = false; indicatorStartTime = millis() + indicatorInterval; }
      fadeLEDColor(3, CRGB::Green);
    } else {
      if (indicatorStartTime < millis()) { indicatorOn = true; indicatorStartTime = millis() + indicatorInterval; }
      fadeLEDColor(3, CRGB::Black);
    }

        // Check if the indicator LED (leds[3]) should blink green
  if (indicatorStartTime < millis() && triggered) {
    indicatorStartTime = millis() + indicatorInterval;
    if (indicatorOn) {
      indicatorOn = false;
      fadeLEDColor(3, CRGB::Black);
    } else {
      indicatorOn = true;
      fadeLEDColor(3, CRGB::Green);
    }
  }
  } else {
    fadeLEDColor(3, CRGB::Black);
    triggered = false;
  }
}

void Task1code(void * parameter)
{
  while(true)
  {
    FastLED.show();
    delay(20);
  }
}

void CheckPin(AsyncResponseStream *response, bool changed)
{
  String status = (changed) ? "triggered" : "not triggered";
  response->print(status);
}

void GetMainMenu(AsyncResponseStream *response)
{
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
  response->print("<h3>/reset - reset prop.</h3>");
  response->print("</body>");
  response->print("</html>");
}

void updateLEDColor(int index, int potValue)
{
  CRGB targetColor;
  
  if (potValue >= 0 && potValue < 512)
  {
    targetColor = CRGB::Black;
  }
  else if (potValue > 512 && potValue < 1024)
  {
    targetColor = CRGB::Red;
  }
  else if (potValue > 1024 && potValue < 1536)
  {
    targetColor = CRGB::Orange;
  }
  else if (potValue > 1536 && potValue < 2048)
  {
    targetColor = CRGB::Yellow;
  }
  else if (potValue > 2048 && potValue < 2560)
  {
    targetColor = CRGB::Green;
  }
  else if (potValue > 2560 && potValue < 3072)
  {
    targetColor = CRGB::Blue;
  }
  else if (potValue > 3072 && potValue < 3584)
  {
    targetColor = CRGB::Purple;
  }
  else if (potValue > 3584 && potValue <= 4095)
  {
    targetColor = CRGB::White;
  }
  
  fadeLEDColor(index, targetColor);
}

void fadeLEDColor(int index, CRGB targetColor)
{
  CRGB currentColor = leds[index];
  
  if (currentColor != targetColor)
  {
    nblend(leds[index], targetColor, 64);
    FastLED.show();
    delay(5);
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
  if (WiFi.status() != WL_CONNECTED) {
    attemptWifiConnection();
  }
}
