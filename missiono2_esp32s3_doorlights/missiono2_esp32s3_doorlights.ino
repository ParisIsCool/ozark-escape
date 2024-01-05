#include <ESPAsyncWebSrv.h>

#define DOOR_SENSOR 9
#define DOOR_PIN 20
#define RESET 19

// Load Wi-Fi library
#include <Arduino.h>
#include <WiFi.h>
#include <ArduinoHttpClient.h>

// Replace with your network credentials
const char* ssid = "Ozark Escape";
const char* password = "0zark3scap3";

// Set your Static IP address
// This is what is used to connect to it in ERM
// IMPORTANT: This must be unique or each arduino!
IPAddress local_IP(192, 168, 1, 102);

// Set your Gateway IP address
IPAddress gateway(192, 168, 1, 254);

IPAddress subnet(255, 255, 255, 0);
IPAddress primaryDNS(8, 8, 8, 8);    //optional
IPAddress secondaryDNS(8, 8, 4, 4);  //optional

char serverAddress[] = "192.168.0.11";  // hue bridge ip
int port = 80;
WiFiClient wifi;
HttpClient client = HttpClient(wifi, serverAddress, port); // hue connection
int status = WL_IDLE_STATUS;

// Set web server port number to 80
AsyncWebServer server(80);

bool allowedtoclose = false;
bool triggered = false;
void setup() {
  Serial.begin(115200);
  while (!Serial) {
  }
  Serial.println("Starting up...");
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(DOOR_PIN, OUTPUT);
  pinMode(DOOR_SENSOR, INPUT);
  pinMode(RESET, INPUT_PULLUP);

  server.on("/check", HTTP_GET, [](AsyncWebServerRequest *request){
    AsyncResponseStream *response = request->beginResponseStream("text/html");
    CheckPin(response, triggered);
    response->addHeader("Access-Control-Allow-Origin","*");
    request->send(response);
  });

  server.on("/open", HTTP_GET, [](AsyncWebServerRequest *request){
    AsyncResponseStream *response = request->beginResponseStream("text/html");
    digitalWrite(DOOR_PIN, LOW);
    response->addHeader("Access-Control-Allow-Origin","*");
    response->print("DOOR OPENED");
    request->send(response);
  });

  server.on("/close", HTTP_GET, [](AsyncWebServerRequest *request){
    AsyncResponseStream *response = request->beginResponseStream("text/html");
    CloseDoor();
    response->addHeader("Access-Control-Allow-Origin","*");
    response->print("DOOR CLOSED");
    request->send(response);
  });

  server.on("/reset", HTTP_GET, [](AsyncWebServerRequest *request){
    AsyncResponseStream *response = request->beginResponseStream("text/html");
    allowedtoclose = true;
    Serial.println("Remotely allowing door to close.");
    response->addHeader("Access-Control-Allow-Origin","*");
    response->print("Door lock reset");
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


void httpRequest(String type, String url, String body) {
  if (type == "post") {
    client.post(url, "application/x-www-form-urlencoded", body);
  } else if ( type == "put" ) {
    client.put(url, "application/x-www-form-urlencoded", body);
  }
  int statusCode = client.responseStatusCode();
  String response = client.responseBody();
  if ( statusCode == 200 ) {
    Serial.println("Http Request Succeeded 200");
  } else {
    Serial.print( type + " [" + url + "] Connection Failed. Code: ");
    Serial.println(statusCode);
  }
}

void ChangeLight(String lightNum, String Body) {
  httpRequest("put","/api/fnxBRnbmb9MwNOKZsvvJl7Ex-hphqPLX4k8mAqA0/lights/" + lightNum + "/state",Body);  
}

int doordelay = 0;
bool triggeredonce = false;
void OpenDoor() {
  triggered = true;
  doordelay = millis() + 2000;
  Serial.println("Door Triggered Open!");
  //delay(1000);
  //ChangeLight("33","{\"on\":true,\"bri\":254,\"hue\":25000,\"sat\":140}");
  //ChangeLight("28","{\"on\":true,\"bri\":254,\"hue\":25000,\"sat\":140}");
  //delay(5000);
  //digitalWrite(DOOR_PIN, LOW);
}

void CloseDoor() {
  triggered = false;
  triggeredonce = false;
  Serial.println("Door Triggered Closed!");
  digitalWrite(DOOR_PIN, HIGH);
  ChangeLight("33","{\"on\":true,\"bri\":254,\"hue\":65535,\"sat\":254}");
  ChangeLight("28","{\"on\":true,\"bri\":254,\"hue\":65535,\"sat\":254}");
}

int valve_reading;
int hold = 0;
int lastRead = 0;
void loop() {
  if (WiFi.status() == WL_CONNECTED) {
    if (triggered && doordelay <= millis() && !triggeredonce) {
      digitalWrite(DOOR_PIN, LOW);
      ChangeLight("33","{\"on\":true,\"bri\":254,\"hue\":25000,\"sat\":140}");
      ChangeLight("28","{\"on\":true,\"bri\":254,\"hue\":25000,\"sat\":140}");
      triggeredonce = true;
    }
    valve_reading = analogRead(DOOR_SENSOR);
    //Serial.println(valve_reading);
    if (valve_reading > 3500 && !triggered) {
      if (hold <= millis()) {
        OpenDoor();
      }
    } else if (valve_reading < 3500 && triggered && allowedtoclose) {
      if (hold <= millis()) {
        allowedtoclose = false;
        CloseDoor();
      }
    } else {
      hold = millis() + 500;
    }
    if (digitalRead(RESET) == LOW) {
      Serial.println("Magnet resetting, allowing to close.");
      allowedtoclose = true;
    }
  } else {
    attemptWifiConnection();
  }
  delay(20);
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
  response->print("<h3>Mission 02 Sliding Door.</h3>");
  response->print("<h3>/open - opens door</h3>");
  response->print("<h3>/close - closes door</h3>");
  response->print("<h3>/reset - allows door to close</h3>");
  response->print("<h3>/check - checks if door opened</h3>");
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