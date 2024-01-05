#include <ESPAsyncWebSrv.h>

#define SWITCH_SENSOR 19
#define RELAY 1
#define RESET_SWITCH 2
#define START_SWITCH 20
#define LED_BUILTIN 2

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
IPAddress local_IP(192, 168, 0, 91);

// Set your Gateway IP address
IPAddress gateway(192, 168, 0, 1);

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

bool triggered = false;
void setup() {
  Serial.begin(115200);
  while (!Serial) {
  }
  Serial.println("Starting up...");
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(RELAY, OUTPUT);
  pinMode(SWITCH_SENSOR, INPUT_PULLUP);
  pinMode(RESET_SWITCH, INPUT_PULLUP);
  pinMode(START_SWITCH, INPUT_PULLUP);

  server.on("/start", HTTP_GET, [](AsyncWebServerRequest *request){
    AsyncResponseStream *response = request->beginResponseStream("text/html");
    CheckPin(response, digitalRead(START_SWITCH) == LOW);
    response->addHeader("Access-Control-Allow-Origin","*");
    request->send(response);
  });

  server.on("/reset", HTTP_GET, [](AsyncWebServerRequest *request){
    AsyncResponseStream *response = request->beginResponseStream("text/html");
    QuickReset();
    response->addHeader("Access-Control-Allow-Origin","*");
    response->print("SWITCHES RESET");
    request->send(response);
  });

  server.on("/solve", HTTP_GET, [](AsyncWebServerRequest *request){
    AsyncResponseStream *response = request->beginResponseStream("text/html");
    TurnOnLights();
    response->addHeader("Access-Control-Allow-Origin","*");
    response->print("FORCE SOLVED");
    request->send(response);
  });

  server.on("/alarm", HTTP_GET, [](AsyncWebServerRequest *request){
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
void ChangeScene(String groupId, String Body) {
  httpRequest("put","/api/fnxBRnbmb9MwNOKZsvvJl7Ex-hphqPLX4k8mAqA0/groups/" + groupId + "/action",Body);  
}


void TurnOnLights() {
  triggered = true;
  Serial.println("Turning on lights! All switches flipped.");
  digitalWrite(RELAY, HIGH);
  delay(500);
  digitalWrite(RELAY, LOW);
  ChangeScene("2","{\"scene\": \"MOMgXUAx5Yad1RN\"}");
  Serial.println("Lights are on.");
}

void QuickReset() {
  triggered = false;
  //ChangeLight("33","{\"on\":true,\"bri\":254,\"hue\":65535,\"sat\":254}");
  //ChangeLight("28","{\"on\":true,\"bri\":254,\"hue\":65535,\"sat\":254}");
  Serial.println("reset called");
  digitalWrite(RELAY, LOW);
  //ChangeScene("2","{\"scene\": \"9b0FgyWbPuJgr3S\"}");
}

String command;
int bootup = millis() + 2000;
void loop() {
  if (WiFi.status() == WL_CONNECTED) {
    if (Serial.available()) {
      command = Serial.readStringUntil('\n');
      command.trim();
      if (command.equals("reset")) {
        QuickReset();
      }
    }
    if (bootup < millis()) {
      if (digitalRead(SWITCH_SENSOR) == LOW && !triggered) {
        Serial.println("Switches Complete. Turning on lights");
        TurnOnLights();
      }
      if (digitalRead(RESET_SWITCH) == LOW && triggered) {
        //ESP.restart();
        QuickReset();
      }    
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
  response->print("<h3>Mission 02 Lights Switches.</h3>");
  response->print("<h3>/start - checks start button</h3>");
  response->print("<h3>/alarm - checks triggered status</h3>");
  response->print("<h3>/solve - force solves</h3>");
  response->print("<h3>/reset - force resets</h3>");
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