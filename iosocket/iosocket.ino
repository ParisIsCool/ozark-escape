#include <WebSocketsClient.h>
#include <WiFi.h>

const char* ssid = "Ryan&Angela";
const char* password = "14727890";
const char* serverIP = "192.168.0.213";

const int serverPort = 443; // The port you used for the server

WebSocketsClient webSocket;

void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");

  webSocket.begin(serverIP, serverPort);
  webSocket.onEvent(webSocketEvent);
}

void loop() {
  webSocket.loop();
}

void webSocketEvent(WStype_t type, uint8_t* payload, size_t length) {
  switch (type) {
    case WStype_CONNECTED:
    Serial.println("Connected to server");
    break;
    case WStype_TEXT:
    Serial.print("Message from server: ");
    for (int i = 0; i < length; i++) {
      Serial.print((char)payload[i]);
    }
      Serial.println();
      break;
      case WStype_DISCONNECTED:
      Serial.println("Disconnected from server");
      break;
    }
}