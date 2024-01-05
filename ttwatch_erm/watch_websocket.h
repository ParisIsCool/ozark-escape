
#include <WebSocketsServer.h>
#include <ArduinoJson.h>

#include "watch_screen.h"

WebSocketsServer webSocket = WebSocketsServer(81);

void handleWebSocketData(uint8_t* payload, size_t length) {
  // Parse JSON data received via WebSocket
  DynamicJsonDocument doc(512); // Adjust the buffer size as needed
  DeserializationError error = deserializeJson(doc, payload, length);
  if (error) {
    Serial.println("Failed to parse JSON data");
    return;
  }

  if (doc.containsKey("time")) {
    // Retrieve room name and time
    String roomName = doc["room"];
    String roomTime = doc["time"];

    // Display the room data
    displayRoom(roomName, roomTime);
  } else {
    // Handle an error or other data
    //displayError();
  }

  if (doc.containsKey("clue")) {
    String roomName = doc["room"];
    String roomClue = doc["clue"];

    bool Clue = false;
    if (roomClue == "true") {
      Clue = true;
    }
    RoomWantsClue(roomName,Clue);
  }
}

void webSocketEvent(uint8_t num, WStype_t type, uint8_t* payload, size_t length) {
  switch (type) {
    case WStype_DISCONNECTED:
      Serial.printf("[%u] Disconnected!\n", num);
      break;
    case WStype_CONNECTED:
      {
        IPAddress ip = webSocket.remoteIP(num);
        Serial.printf("[%u] Connected from %d.%d.%d.%d\n", num, ip[0], ip[1], ip[2], ip[3]);
      }
      break;
    case WStype_TEXT:
      handleWebSocketData(payload, length);
      break;
  }
}