#include <WiFi.h>
#include <ESPmDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include <ArduinoJson.h>
#include <Preferences.h>

#define SERVICE_UUID "6e400001-b5a3-f393-e0a9-e50e24dcca9e"
#define CHARACTERISTIC_UUID "6e400002-b5a3-f393-e0a9-e50e24dcca9e"

Preferences preferences;

// Replace with your network credentials
String ssid = "Ozark Escape";
String password = "0zark3scap3";
IPAddress local_IP(192, 168, 0, 200);
IPAddress subnet(255, 255, 255, 0);
IPAddress gateway(192, 168, 0, 1);

BLEServer* pServer = NULL;
BLECharacteristic* pCharacteristic = NULL;
bool deviceConnected = false;

class MyServerCallbacks : public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
        deviceConnected = true;
        Serial.println("Device connected.");
    }

    void onDisconnect(BLEServer* pServer) {
        deviceConnected = false;
        Serial.println("Device disconnected.");
    }
};

void setup() {
  Serial.begin(115200);
  Serial.println("Booting...");
  preferences.begin("Ozark Escape", false); 
  readData();

  BLEDevice::init("EZESP32"); // Set your device name here

  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  BLEService* pService = pServer->createService(SERVICE_UUID);

  pCharacteristic = pService->createCharacteristic(
      CHARACTERISTIC_UUID,
      BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE
  );

  pCharacteristic->addDescriptor(new BLE2902());

  pService->start();
  BLEAdvertising *pAdvertising = pServer->getAdvertising();
  pAdvertising->addServiceUUID(pService->getUUID());
  pAdvertising->setScanResponse(true);
  pAdvertising->setMinPreferred(0x06);
  pAdvertising->setMinPreferred(0x12);
  BLEDevice::startAdvertising();
  Serial.println("Waiting for a Bluetooth connection...");
  Serial.println("EZ Connect tool: https://purplecraft.net/ezesp");

  ArduinoOTA.setPort(3232);

  // Hostname defaults to esp3232-[MAC]
  ArduinoOTA.setHostname("myesp32");

  // No authentication by default
  // ArduinoOTA.setPassword("admin");

  // Password can be set with it's md5 value as well
  // MD5(admin) = 21232f297a57a5a743894a0e4a801fc3
  // ArduinoOTA.setPasswordHash("21232f297a57a5a743894a0e4a801fc3");

  ArduinoOTA
    .onStart([]() {
      String type;
      if (ArduinoOTA.getCommand() == U_FLASH)
        type = "sketch";
      else // U_SPIFFS
        type = "filesystem";

      // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
      Serial.println("Start updating " + type);
    })
    .onEnd([]() {
      Serial.println("\nEnd");
    })
    .onProgress([](unsigned int progress, unsigned int total) {
      Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
    })
    .onError([](ota_error_t error) {
      Serial.printf("Error[%u]: ", error);
      if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
      else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
      else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
      else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
      else if (error == OTA_END_ERROR) Serial.println("End Failed");
    });

}

bool startsWith(const char *pre, const char *str)
{
    size_t lenpre = strlen(pre),
           lenstr = strlen(str);
    return lenstr < lenpre ? false : memcmp(pre, str, lenpre) == 0;
}

void CheckBluetoothForData() {

  if (deviceConnected) {
    neopixelWrite(RGB_BUILTIN,0,0,RGB_BRIGHTNESS); // Blue
  } else if (!deviceConnected && WiFi.status() != WL_CONNECTED) {
    neopixelWrite(RGB_BUILTIN,RGB_BRIGHTNESS,0,0); // Red
  } else if (WiFi.status() == WL_CONNECTED) {
    neopixelWrite(RGB_BUILTIN,0,RGB_BRIGHTNESS,0); // Green
  }

  // Check if data is available
  std::string value = pCharacteristic->getValue();
  
  if (!value.empty()) {
    // Process the received data
    if (startsWith("Network Info: ", value.c_str())) {
      // Parse the JSON data
      DynamicJsonDocument doc(512); // Adjust the size according to your JSON data size
      DeserializationError error = deserializeJson(doc, value.c_str() + 14); // +14 to skip "Network Info: "

      if (!error) {
        // Extract data from JSON
        const char* newSsid = doc["ssid"];
        const char* newPassword = doc["password"];
        
        preferences.putString("ssid", newSsid); 
        preferences.putString("password", newPassword);

        // Extract IP addresses
        for (int i = 0; i < 4; i++) {
          if (doc["localIp"][i].is<int>()) {
            local_IP[i] = doc["localIp"][i].as<int>();
            preferences.putInt("localip"+i, local_IP[i]);
          }
          if (doc["subnet"][i].is<int>()) {
            subnet[i] = doc["subnet"][i].as<int>();
            preferences.putInt("subnet"+i, local_IP[i]);
          }
          if (doc["gateway"][i].is<int>()) {
            gateway[i] = doc["gateway"][i].as<int>();
            preferences.putInt("gateway"+i, local_IP[i]);
          }
        }
        Serial.println("Info Saved.");
        readData();
      } else {
        Serial.println("Failed to parse JSON");
      }
    }
    if (value.c_str() == "Wassup Homie") {
        Serial.println("Handshake Received");
    }

    // Clear the characteristic value after processing
    pCharacteristic->setValue(std::string(""));
  }
}

bool doOTA = false;
void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    attemptWifiConnection();
  } else {
    ArduinoOTA.handle();
  }
  delay(20);
  CheckBluetoothForData();
}

void readData() {

  // Extract credentials for wifi.
  ssid = preferences.getString("ssid");
  password = preferences.getString("password");
    
  // Read IP addresses from preferences
  for (int i = 0; i < 4; i++) {
    local_IP[i] = preferences.getInt("localip"+i);
    subnet[i] = preferences.getInt("subnet"+i);
    gateway[i] = preferences.getInt("gateway"+i);
  }

  // Print the read data
  Serial.println("Saved data:");
  Serial.print("SSID: "); Serial.println(ssid);
  Serial.print("Password: "); Serial.println(password);
  Serial.print("Local IP: "); Serial.println(local_IP);
  Serial.print("Subnet: "); Serial.println(subnet);
  Serial.print("Gateway: "); Serial.println(gateway);

}

void attemptWifiConnection() {
  neopixelWrite(RGB_BUILTIN,RGB_BRIGHTNESS,0,0); // Red

    // Configures static IP address
  if (!WiFi.config(local_IP, gateway, subnet)) {
    Serial.println("STA Failed to configure, DHCP error! Using random assigned IP!");
    Serial.println("Please update router settings to allow DHCP and static IP.");
  }

  // Connect to Wi-Fi network with SSID and password
  Serial.print("\n\n\nConnecting to ");
  Serial.println(ssid);
  Serial.println(password);
  WiFi.mode(WIFI_STA);
  WiFi.setSleep(WIFI_PS_NONE);
  WiFi.begin(ssid, password);
  bool blink = false;
  int timeout = millis() + 9500;
  while ( !(WiFi.status() == WL_CONNECTED || timeout < millis() ) ) {
    CheckBluetoothForData();
    Serial.print(".");
    delay(300);
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
    ArduinoOTA.begin();
    doOTA = true;
    neopixelWrite(RGB_BUILTIN,0,RGB_BRIGHTNESS,0); // Green
  }
}