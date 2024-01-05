// RECEIVER MAC: B0:B2:1C:A7:C6:94
// SENDER MAC: 0C:B8:15:F3:C0:F0

#include <esp_now.h>
#include <WiFi.h>

#define OUT_PIN 15

// REPLACE WITH THE MAC Address of your receiver 
uint8_t broadcastAddress[] = {0x0C, 0xB8, 0x15, 0xF3, 0xC0, 0xF0};

//Structure example to send data
//Must match the receiver structure
typedef struct struct_message {
    bool action;
} struct_message;

struct_message incoming;

esp_now_peer_info_t peerInfo;

// Callback when data is received
void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
  memcpy(&incoming, incomingData, sizeof(incoming));
  if (incoming.action) {
    digitalWrite(OUT_PIN,HIGH);
  } else {
    digitalWrite(OUT_PIN,LOW);
  }
}
 
void setup() {
  // Init Serial Monitor
  Serial.begin(115200);
 
  pinMode(OUT_PIN,OUTPUT);

  // Set device as a Wi-Fi Station
  WiFi.mode(WIFI_STA);

  // Init ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }
  
  // Register peer
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.channel = 0;  
  peerInfo.encrypt = false;
  
  // Add peer        
  if (esp_now_add_peer(&peerInfo) != ESP_OK){
    Serial.println("Failed to add peer");
    return;
  }

  // Register for a callback function that will be called when data is received
  esp_now_register_recv_cb(OnDataRecv);
}

bool lasttrigger;
void loop() {
  // do nothing
}