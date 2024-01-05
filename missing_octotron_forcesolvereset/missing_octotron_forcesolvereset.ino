// Load Wi-Fi library
#include <Arduino.h>
#include <WiFi.h>

// These pins correlate to polling from IP
// http://192.168.0.XXX/1 = PIN_1 below.
#define RELAY 13
#define RELAY2 12
#define LED 2

// Replace with your network credentials
const char* ssid = "Ozark Escape";
const char* password = "0zark3scap3";

// Set your Static IP address
// This is what is used to connect to it in ERM
// IMPORTANT: This must be unique or each arduino!
IPAddress local_IP(192, 168, 0, 81);


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
  // Initialize the input variables as input
  pinMode(RELAY, OUTPUT);
  pinMode(RELAY2, OUTPUT);
  pinMode(LED, OUTPUT);
}

void loop() {
  runWifiLoop();
}


// Actual request handler
void processRequest(WiFiClient& client, String requestStr) {
  // Send back different response based on request string
  if (requestStr.startsWith("GET /1")) {
    digitalWrite(RELAY, HIGH);
    Serial.println("Toggled 1 On");
    client.println("HTTP/1.1 200 OK");
    client.println("Content-Type: text/plain");
    client.println("toggled");
    delay(1000);
    digitalWrite(RELAY, LOW);
  } else if (requestStr.startsWith("GET /2")) {
    digitalWrite(RELAY2, HIGH);
    Serial.println("Toggled 2 On");
    client.println("HTTP/1.1 200 OK");
    client.println("Content-Type: text/plain");
    client.println("toggled");
    delay(1000);
    digitalWrite(RELAY2, LOW);
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
    client.println("<h3>4 Particle Reset Trigger.</h3>");
    client.println("<h3>/1 resets.</h3>");
    client.println("<h3>/2 force solves.</h3>");
    client.println("</body>");
    client.println("</html>");
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
      digitalWrite(LED, HIGH);
      blink = false;
    } else {
      digitalWrite(LED, LOW);
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
    digitalWrite(LED, LOW);
    delay(50);
    digitalWrite(LED, HIGH);
    delay(50);
    digitalWrite(LED, LOW);
    delay(50);
    digitalWrite(LED, HIGH);
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
    delay(100);  // change based off lag.
    // close the connection:
    client.stop();
  }
}

void runWifiLoop() {
  if (WiFi.status() == WL_CONNECTED) {
    listenForWifiClients();
  } else {
    attemptWifiConnection();
  }
}