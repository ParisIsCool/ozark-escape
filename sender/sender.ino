//1: B0:B2:1C:A7:C6:94
//2: 0C:B8:15:F3:C0:F0

// SENDER MAC: 0C:B8:15:F3:C0:F0

#include <esp_now.h>
#include <WiFi.h>

#define IN_PIN 15

// REPLACE WITH THE MAC Address of your receiver 
uint8_t broadcastAddress[] = {0xB0, 0xB2, 0x1C, 0xA7, 0xC6, 0x94};

// Variable to store if sending data was successful
String success;

//Structure example to send data
//Must match the receiver structure
typedef struct struct_message {
    bool action;
} struct_message;

struct_message outgoing;

esp_now_peer_info_t peerInfo;

// Callback when data is sent
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("\r\nLast Packet Send Status:\t");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
  if (status ==0){
    success = "Delivery Success :)";
  }
  else{
    success = "Delivery Fail :(";
  }
}
 
void setup() {
  // Init Serial Monitor
  Serial.begin(115200);
 
  pinMode(IN_PIN,INPUT_PULLUP);

  // Set device as a Wi-Fi Station
  WiFi.mode(WIFI_STA);

  // Init ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  // Once ESPNow is successfully Init, we will register for Send CB to
  // get the status of Trasnmitted packet
  esp_now_register_send_cb(OnDataSent);
  
  // Register peer
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.channel = 0;  
  peerInfo.encrypt = false;
  
  // Add peer        
  if (esp_now_add_peer(&peerInfo) != ESP_OK){
    Serial.println("Failed to add peer");
    return;
  }
}

bool lasttrigger;
void loop() {
  bool triggered = (digitalRead(IN_PIN) == LOW);
  // Send message via ESP-NOW
  if (triggered != lasttrigger) {
    outgoing.action = triggered;
    esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) &outgoing, sizeof(outgoing));
  }
  lasttrigger = triggered;

  delay(2000);
}