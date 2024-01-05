// Load Wi-Fi library
#include <Arduino.h>
#include <WiFi.h>
#include <ArduinoHttpClient.h>

#define LED 2

#define DOOR_SENSOR 34
#define DOOR_PIN 13
#define RESET 0

// Replace with your network credentials
const char* ssid = "Ozark Escape";
const char* password = "0zark3scap3";

// Set your Static IP address
// IMPORTANT: This must be unique or each arduino!
IPAddress local_IP(192, 168, 0, 90);

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
  Serial.begin(9600);
  // Initialize pins
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(DOOR_PIN, OUTPUT);
  pinMode(DOOR_SENSOR, INPUT);
  pinMode(RESET, INPUT_PULLUP);
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
bool triggered = false;
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
bool allowedtoclose = false;
int hold = 0;
int lastRead = 0;
void loop() {
  if (WiFi.status() == WL_CONNECTED) {
    listenForWifiClients();
    if (triggered && doordelay <= millis() && !triggeredonce) {
      digitalWrite(DOOR_PIN, LOW);
      ChangeLight("33","{\"on\":true,\"bri\":254,\"hue\":25000,\"sat\":140}");
      ChangeLight("28","{\"on\":true,\"bri\":254,\"hue\":25000,\"sat\":140}");
      triggeredonce = true;
    }
    valve_reading = analogRead(DOOR_SENSOR);
    Serial.println(valve_reading);
    if (valve_reading >= 800 && !triggered) {
      if (hold <= millis()) {
        OpenDoor();
      }
    } else if (valve_reading < 100 && triggered && allowedtoclose) {
        allowedtoclose = false;
        CloseDoor();
    } else {
      hold = millis() + 1000;
    }
    if (digitalRead(RESET) == LOW) {
      Serial.println("Magnet resetting, allowing to close.");
      allowedtoclose = true;
    }
  } else {
    attemptWifiConnection();
  }
}

String BasicHTML(String text) {
  String ptr = "HTTP/1.1 200 OK";
  ptr += "Content-Type: text/html\n";
  ptr += "Access-Control-Allow-Origin: *\n\n";
  ptr += text;
  return ptr;
}

// Actual request handler
void processRequest(WiFiClient& client, String requestStr) {
  // Send back different response based on request string
  if (requestStr.startsWith("GET /check")) {
    String status = (triggered) ? "triggered" : "not triggered";
    client.print(BasicHTML(status));
  } else if (requestStr.startsWith("GET /open")) {
    digitalWrite(DOOR_PIN, LOW);
    OpenDoor();
    Serial.println("ONLINE opened door");
    String status = "OPENED DOOR";
    client.print(BasicHTML(status));
  } else if (requestStr.startsWith("GET /close")) {
    CloseDoor();
    Serial.println("ONLINE closed door");
    String status = "CLOSED DOOR";
    client.print(BasicHTML(status));
  } else if (requestStr.startsWith("GET /reset")) {
    allowedtoclose = true;
    Serial.println("Remotely allowing door to close.");
    String status = "Door lock reset.";
    client.print(BasicHTML(status));
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
    client.println("<h3>Mission 02 Sliding Door.</h3>");
    client.println("<h3>/open - opens door</h3>");
    client.println("<h3>/close - closes door</h3>");
    client.println("<h3>/reset - allows door to close</h3>");
    client.println("<h3>/check - checks if door opened</h3>");
    client.println("</body>");
    client.println("</html>");
  }
}

// DO NOT TOUCH
// If having trouble connecting, signal may be weak.
int timeout = 0;
String requestStr;
void listenForWifiClients() {
  // listen for incoming clients
  WiFiClient client = server.available();
  if(!client)
  {
    return;
  }
  if (client) {
    // Grab the first HTTP header (GET /status HTTP/1.1)
    boolean firstLine = true;
    // an http request ends with a blank line
    boolean currentLineIsBlank = true;
    if (client.available()) {
      timeout = millis() + 10000; 
      while (client.connected()) {
        char c = client.read();
        // if you've gotten to the end of the line (received a newline
        // character) and the line is blank, the http request has ended,
        // so you can send a reply
        if (millis() > timeout) {
          // didnt reach end of line, dont want to freeze.
          break;
        }
        if (c == '\n' && currentLineIsBlank) {
          processRequest(client, requestStr);
          //free(c);
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
    delay(5);  // change based off lag.
    // close the connection:
    requestStr = "";
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
  WiFi.setSleep(WIFI_PS_NONE);
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

