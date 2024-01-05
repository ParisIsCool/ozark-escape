// Load Wi-Fi library
#include <Arduino.h>
#include <WiFi.h>
#include <ArduinoHttpClient.h>

#define SWITCH_SENSOR 18
#define RELAY 4
#define RESET_SWITCH 5
#define START_SWITCH 1
#define LED_BUILTIN 2


// Replace with your network credentials
const char* ssid = "Ozark Escape";
const char* password = "0zark3scap3";

// Set your Static IP address
// IMPORTANT: This must be unique or each arduino!
IPAddress local_IP(192, 168, 0, 91);

IPAddress gateway(192, 168, 0, 1); // may vary per router
IPAddress subnet(255, 255, 255, 0); // check cmd ipconfig to check
IPAddress primaryDNS(8, 8, 8, 8);    //optional
IPAddress secondaryDNS(8, 8, 4, 4);  //optional

char serverAddress[] = "192.168.0.11";  // hue bridge ip
int port = 80;
WiFiClient wifi;
HttpClient client = HttpClient(wifi, serverAddress, port); // hue connection
int status = WL_IDLE_STATUS;

// Set web server port number to 80
WiFiServer server(80);

void setup() {
  Serial.begin(115200);
  // Initialize pins
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(RELAY, OUTPUT);
  pinMode(SWITCH_SENSOR, INPUT_PULLUP);
  pinMode(RESET_SWITCH, INPUT_PULLUP);
  pinMode(START_SWITCH, INPUT_PULLUP);
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

bool triggered;
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
    listenForWifiClients();
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
        ESP.restart();
        //QuickReset();
      }    
    }
  } else {
    attemptWifiConnection();
  }
}

bool wait_check = false;
void CheckPin(WiFiClient& client, int pin) {
  String status = (digitalRead(pin) == LOW) ? "ready to trigger" : "not triggered";
  if (status == "ready to trigger" && wait_check) {
    wait_check = false;
    status = "triggered";
  } else if (status == "ready to trigger" && !wait_check) {
    wait_check = true;
  } else {
    wait_check = false;
  }
  client.println("HTTP/1.1 200 OK");
  client.println("Content-Type: text/plain");
  client.println("Access-Control-Allow-Origin: *");  // ERM will not be able to connect without this header!
  client.println();
  client.print(status);
}

// Actual request handler
void processRequest(WiFiClient& client, String requestStr) {
  // Send back different response based on request string
  if (requestStr.startsWith("GET /start")) {
    CheckPin(client,START_SWITCH);
  } else if (requestStr.startsWith("GET /reset")) {
    QuickReset();
    Serial.println("force reset");
    client.println("HTTP/1.1 200 OK");
    client.println("Content-Type: text/plain");
    client.println("force reset");
  } else if (requestStr.startsWith("GET /solve")) {
    TurnOnLights();
    Serial.println("force solved");
    client.println("HTTP/1.1 200 OK");
    client.println("Content-Type: text/plain");
    client.println("force solved");
  } else if (requestStr.startsWith("GET /alarm")) {
    String status = triggered ? "triggered" : "not triggered";
    client.println("HTTP/1.1 200 OK");
    client.println("Content-Type: text/plain");
    client.println("Access-Control-Allow-Origin: *");  // ERM will not be able to connect without this header!
    client.println();
    client.print(status);
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
    client.println("<h3>Mission 02 Lights Switches.</h3>");
    client.println("<h3>/start - checks start button</h3>");
    client.println("<h3>/alarm - checks triggered status</h3>");
    client.println("<h3>/solve - force solves</h3>");
    client.println("<h3>/reset - force resets</h3>");
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
    delay(10);  // change based off lag.
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
    delay(500);  // LED BLINKS WHEN TRYING TO CONNECT
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
    // BLINK A FEW TIMES FAST TO SIGNIFIY CONNECTION SUCCESS
    digitalWrite(LED_BUILTIN, LOW);
    delay(250);
    digitalWrite(LED_BUILTIN, HIGH);
    delay(250);
    digitalWrite(LED_BUILTIN, LOW);
    delay(250);
    digitalWrite(LED_BUILTIN, HIGH);
  }
}

