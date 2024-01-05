#include <esp_now.h>
#include <WiFi.h>

#include "SoftwareSerial.h"
#include "DFRobotDFPlayerMini.h"

// Use pins 2 and 3 to communicate with DFPlayer Mini
static const uint8_t PIN_MP3_TX = 7; // Connects to module's RX
static const uint8_t PIN_MP3_RX = 8; // Connects to module's TX
SoftwareSerial softwareSerial(PIN_MP3_RX, PIN_MP3_TX);
// Create the Player object
DFRobotDFPlayerMini player;

#define CHANNEL 1

#define FOG_POWER 12
#define FOG_TOGGLE 22
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
  /*softwareSerial.begin(9600);
  if (player.begin(softwareSerial))
  {
   Serial.println("Software Serial Player Began Successfull");
   player.volume(100);
   }
  else
  {
    Serial.println("Connecting to DFPlayer Mini failed!");
  }*/
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

bool trig_check = false;
bool triggered = false;
float wait = 0;
void loop() {
  int sensorVal = digitalRead(BUTTON);
  if (sensorVal == LOW && !triggered && !trig_check)
  {
    trig_check = true;
    //player.volume(100);
    //player.play(1);
    Serial.println("trigchec");
    wait = millis() + 5000;
  }
  if (sensorVal == LOW && !triggered && trig_check && wait < millis()) // trigger must be checked pushed for 5 seconds
  {
    triggered = true;
    trig_check = false;
    //player.volume(100);
    //player.play(2);
    digitalWrite(FOG_TOGGLE,HIGH);
    delay(15000);
    digitalWrite(FOG_TOGGLE,LOW);
    Serial.println("trigger");
  }
  if (sensorVal == HIGH && trig_check) // PAUSE PLAYER, UNPRESSED WHEN PLAYING COUNTDOWN
  {
    Serial.println("pause");
    //player.pause();
    trig_check = false;
  }
  if (sensorVal == HIGH && triggered) // UNPRESSED AFTER TRIGGERED
  {
    triggered = false;
    digitalWrite(FOG_TOGGLE,LOW);
    Serial.println("untrigger");
  }
  delay(100);
}
