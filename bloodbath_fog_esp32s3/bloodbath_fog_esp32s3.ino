#include <esp_now.h>
#include <WiFi.h>

#define CHANNEL 1

#define FOG_POWER 12
#define FOG_TOGGLE 19
#define BUTTON 21

// Init ESP Now with fallback
void InitESPNow() {
  WiFi.disconnect();
  if (esp_now_init() == ESP_OK) {
    Serial.println("ESPNow Init Success");
  }
  else {
    Serial.println("ESPNow Init Failed");
    // Retry InitESPNow, add a counte and then restart?
    // InitESPNow();
    // or Simply Restart
    ESP.restart();
  }
}

// config AP SSID
void configDeviceAP() {
  const char *SSID = "Bloodbath Fog Slave";
  bool result = WiFi.softAP(SSID, "0zark3scap3", CHANNEL, 0);
  if (!result) {
    Serial.println("AP Config failed.");
  } else {
    Serial.println("AP Config Success. Broadcasting with AP: " + String(SSID));
    Serial.print("AP CHANNEL "); Serial.println(WiFi.channel());
  }
}

void setup() {
  pinMode(FOG_POWER, OUTPUT);
  pinMode(FOG_TOGGLE, OUTPUT);
  pinMode(BUTTON, INPUT_PULLUP);
  Serial.begin(115200);
  Serial.println("Bloodbath Slave");
  //Set device in AP mode to begin with
  WiFi.mode(WIFI_AP);
  // configure device AP mode
  configDeviceAP();
  // This is the mac address of the Slave in AP Mode
  Serial.print("AP MAC: "); Serial.println(WiFi.softAPmacAddress());
  // Init ESPNow with a fallback logic
  InitESPNow();
  // Once ESPNow is successfully Init, we will register for recv CB to
  // get recv packer info.
  esp_now_register_recv_cb(OnDataRecv);
}

// callback when data is recv from Master
void OnDataRecv(const uint8_t *mac_addr, const uint8_t *data, int data_len) {
  char macStr[18];
  snprintf(macStr, sizeof(macStr), "%02x:%02x:%02x:%02x:%02x:%02x",
           mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
  Serial.print("Last Packet Recv from: "); Serial.println(macStr);
  Serial.print("Last Packet Recv Data: "); Serial.println(*data);
  Serial.println("");
  if (*data == 1) {
    digitalWrite(FOG_POWER,HIGH);
  } else {
    digitalWrite(FOG_POWER,LOW);
  }
}

bool isHeld = false;
int holdfor5 = 0;
void loop() {
  if (digitalRead(BUTTON) == LOW) {
    if (isHeld == false) { // first time
      Serial.println("Button Pushed");
      holdfor5 = millis() + 5000;
    }
    isHeld = true;
    if (holdfor5 < millis()) {
      Serial.println("Button Held for 5 Seconds!");
      digitalWrite(FOG_TOGGLE,HIGH);
      delay(15000);
      digitalWrite(FOG_TOGGLE,LOW);
      isHeld = false;
    }
  } else {
    isHeld = false;
  }

}
