// Load Wi-Fi library
#include <Arduino.h>
#include <WiFi.h>
#include <ArduinoHttpClient.h>

#define LED 2

// Replace with your network credentials
const char* ssid = "Ozark Escape";
const char* password = "0zark3scap3";

// Set your Static IP address
// IMPORTANT: This must be unique or each arduino!
IPAddress local_IP(192, 168, 0, 99);

IPAddress gateway(192, 168, 0, 1); // may vary per router
IPAddress subnet(255, 255, 255, 0); // check cmd ipconfig to check
IPAddress primaryDNS(8, 8, 8, 8);    //optional
IPAddress secondaryDNS(8, 8, 4, 4);  //optional

char serverAddress[] = "192.168.0.11";  // hue bridge ip
int port = 80;
WiFiClient wifi;
HttpClient client = HttpClient(wifi, serverAddress, port); // hue connection
int status = WL_IDLE_STATUS;

void setup() {
  Serial.begin(9600);
  // Initialize pins
  pinMode(LED, OUTPUT);
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

String command;

void loop() {
  if (Serial.available()) {
    command = Serial.readStringUntil('\n');
    command.trim();
    if (command.equals("open")) {
      ChangeLight("33","{\"on\":true,\"bri\":254,\"hue\":25000,\"sat\":140}");
      ChangeLight("28","{\"on\":true,\"bri\":254,\"hue\":25000,\"sat\":140}");
    } else if (command.equals("close")) {
      ChangeLight("33","{\"on\":true,\"bri\":254,\"hue\":65535,\"sat\":254}");
      ChangeLight("28","{\"on\":true,\"bri\":254,\"hue\":65535,\"sat\":254}");
    } else {
      ChangeLight("33","{\"on\":true,\"bri\":254,\"hue\":"+command+",\"sat\":254}");
      ChangeLight("28","{\"on\":true,\"bri\":254,\"hue\":"+command+",\"sat\":254}");
    }
  }
  if (WiFi.status() == WL_CONNECTED) {
    //
  } else {
    attemptWifiConnection();
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
    // BLINK A FEW TIMES FAST TO SIGNIFIY CONNECTION SUCCESS
    digitalWrite(LED, LOW);
    delay(50);
    digitalWrite(LED, HIGH);
    delay(50);
    digitalWrite(LED, LOW);
    delay(50);
    digitalWrite(LED, HIGH);
  }
}

