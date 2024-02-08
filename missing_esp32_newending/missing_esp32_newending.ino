// Load Wi-Fi library
#include <Arduino.h>
#include <WiFi.h>
#include <ESPAsyncWebSrv.h>

#define BUTTON_SAVE_HER 1
#define BUTTON_RISKY_SHUTDOWN 0
#define BUTTON_FINISH 4
#define LED_BUILTIN 2

// Replace with your network credentials
const char* ssid = "Ozark Escape";
const char* password = "0zark3scap3";

// Set your Static IP address
// IMPORTANT: This must be unique or each arduino!
IPAddress local_IP(192, 168, 0, 82);

IPAddress gateway(192, 168, 0, 1); // may vary per router
IPAddress subnet(255, 255, 255, 0); // check cmd ipconfig to check
IPAddress primaryDNS(8, 8, 8, 8);    //optional
IPAddress secondaryDNS(8, 8, 4, 4);  //optional

// Set web server port number to 80
AsyncWebServer server(80);

void setup() {
  Serial.begin(9600);
  // Initialize pins
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(BUTTON_SAVE_HER, INPUT_PULLUP);
  pinMode(BUTTON_RISKY_SHUTDOWN, INPUT_PULLUP);
  pinMode(BUTTON_FINISH, INPUT_PULLUP);

  server.on("/ending", HTTP_GET, [](AsyncWebServerRequest *request){
    AsyncResponseStream *response = request->beginResponseStream("text/html");
    if (digitalRead(BUTTON_FINISH) == LOW) {
      if (digitalRead(BUTTON_SAVE_HER) == LOW && digitalRead(BUTTON_RISKY_SHUTDOWN) == LOW) {
        response->print("Both");
      } else {
        if (digitalRead(BUTTON_SAVE_HER) == LOW) {
          response->print("Saved");
        }
        if (digitalRead(BUTTON_RISKY_SHUTDOWN) == LOW) {
          response->print("Not Saved");
        }
      }
    } else {
      response->print("In Limbo");
    }
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

void loop() {
  runWifiLoop();
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
  response->print("<h3>Missing New Ending.</h3>");
  response->print("<h3>/ending - checks ending status</h3>");
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