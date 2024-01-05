#include <FastLED.h>

#define NUM_LEDS 30
#define DATA_PIN 12

// Load Wi-Fi library
#include <Arduino.h>
#include <WiFi.h>

CRGB leds[NUM_LEDS];
int brightness = 255;

const int cylinder1 = 15;
const int cylinder2 = 4;
const int cylinder3 = 5;
const int cylinder4 = 18;
const int cylinder5 = 19;

int cylinder1State = 0;
int cylinder2State = 0;
int cylinder3State = 0;
int cylinder4State = 0;
int cylinder5State = 0;

int prestate1 = 0;
int prestate2 = 0;
int prestate3 = 0;
int prestate4 = 0;
int prestate5 = 0;
int currentStep = 0;
int lastStep = 0;

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
WiFiServer server(80);

void setup() {
  Serial.begin(9600);
  while (!Serial) {
  }
  Serial.println("Starting up...");
  FastLED.addLeds<NEOPIXEL, DATA_PIN>(leds, NUM_LEDS);
  FastLED.setBrightness(500);
  pinMode(cylinder1, INPUT_PULLUP);
  pinMode(cylinder2, INPUT_PULLUP);
  pinMode(cylinder3, INPUT_PULLUP);
  pinMode(cylinder4, INPUT_PULLUP);
  pinMode(cylinder5, INPUT_PULLUP);
  for(int i = 0; i < NUM_LEDS; i++ ) {
    leds[i] = CRGB::Black;
    FastLED.show();
  }
  pinMode(LED_BUILTIN, OUTPUT);
}

bool soundPlay = false;
void loop() {
  runWifiLoop();
  cylinder1State = digitalRead(cylinder1);
  cylinder2State = digitalRead(cylinder2);
  cylinder3State = digitalRead(cylinder3);
  cylinder4State = digitalRead(cylinder4);
  cylinder5State = digitalRead(cylinder5);
   
    if(cylinder1State == LOW && prestate1 == 0)
    {
      currentStep++;
      Serial.println("Cylinder Added");
      Serial.println(currentStep);
      prestate1 = 1;
      leds[4] = CRGB::Red;
    }


    else if(cylinder1State == HIGH && prestate1 == 1)
    {
      currentStep--;
      Serial.println("Cylinder Removed");
      Serial.println(currentStep);
      prestate1 = 0;
      leds[4] = CRGB::Black;
    }


    if(cylinder2State == LOW && prestate2 == 0)
    {
      currentStep++;
      Serial.println("Cylinder Added");
      Serial.println(currentStep);
      prestate2 = 1;
      leds[3] = CRGB::Red;
    }


    else if(cylinder2State == HIGH && prestate2 == 1)
    {
      currentStep--;
      Serial.println("Cylinder Removed");
      Serial.println(currentStep);
      prestate2 = 0;
      leds[3] = CRGB::Black;
    }


    if(cylinder3State == LOW && prestate3 == 0)
    {
      currentStep++;
      Serial.println("Cylinder Added");
      Serial.println(currentStep);
      prestate3 = 1;
      leds[2] = CRGB::Red;
    }


    else if(cylinder3State == HIGH && prestate3 == 1)
    {
      currentStep--;
      Serial.println("Cylinder Removed");
      Serial.println(currentStep);
      prestate3 = 0;
      leds[2] = CRGB::Black;
    }


    if(cylinder4State == LOW && prestate4 == 0)
    {
      currentStep++;
      Serial.println("Cylinder Added");
      Serial.println(currentStep);
      prestate4 = 1;
      leds[1] = CRGB::Red;
    }


    else if(cylinder4State == HIGH && prestate4 == 1)
    {
      currentStep--;
      Serial.println("Cylinder Removed");
      Serial.println(currentStep);
      prestate4 = 0;
      leds[1] = CRGB::Black;
    }


    if(cylinder5State == LOW && prestate5 == 0)
    {
      currentStep++;
      Serial.println("Cylinder Added");
      Serial.println(currentStep);
      prestate5 = 1;
      leds[0] = CRGB::Red;
    }


    else if(cylinder5State == HIGH && prestate5 == 1)
    {
      currentStep--;
      Serial.println("Cylinder Removed");
      Serial.println(currentStep);
      prestate5 = 0;
      leds[0] = CRGB::Black;
    }


    if(currentStep == 0)
    {
      for(int i = 5; i < 30; i++ )
      leds[i] = CRGB::Black;
    }
    if(currentStep == 1)
    {
      for(int i = 5; i < 10; i++ )
      leds[i] = CRGB::Red;
      for(int i = 10; i < 30; i++ )
      leds[i] = CRGB::Black;
    }
    if(currentStep == 2)
    {
      for(int i = 5; i < 15; i++ )
      leds[i] = CRGB::Red;
      for(int i = 15; i < 30; i++ )
      leds[i] = CRGB::Black;
    }
    if(currentStep == 3)
    {
      for(int i = 5; i < 20; i++ )
      leds[i] = CRGB::Red;
      for(int i = 20; i < 30; i++ )
      leds[i] = CRGB::Black;
    }
    if(currentStep == 4)
    {
      for(int i = 5; i < 25; i++ )
      leds[i] = CRGB::Red;
      for(int i = 25; i < 30; i++ )
      leds[i] = CRGB::Black;
    }
    if(currentStep == 5)
    {
      for(int i = 5; i < 30; i++ )
      leds[i] = CRGB::Red;
    }


    if((cylinder1State==LOW)&&(cylinder2State==LOW)&&(cylinder3State==LOW)&&(cylinder4State==LOW)&&(cylinder5State==LOW))
  {
      for(int i = 5; i < NUM_LEDS; i++ )
      leds[i] = CRGB::Green;
  }
  FastLED.show();
  if (currentStep > lastStep && currentStep != 5) {
    soundPlay = true;    
  }
  lastStep = currentStep;
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
    CheckPin(client, currentStep == 5);
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