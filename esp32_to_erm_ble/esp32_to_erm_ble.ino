#include <ESPAsyncWebSrv.h>
#include <Arduino.h>
#include <WiFi.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include <ArduinoJson.h>
#include <Preferences.h>

#define SERVICE_UUID "6e400001-b5a3-f393-e0a9-e50e24dcca9e"
#define CHARACTERISTIC_UUID "6e400002-b5a3-f393-e0a9-e50e24dcca9e"

const int numOfInputs = 5;
const int numOfOutputs = 10;
const int inputs[numOfInputs] = {19,20,21,47,48};
const int outputs[numOfOutputs] = {14,13,12,11,10,9,46,3,8,18};
#define BLUETOOTH_PIN 4
int soundStates[numOfInputs];
int soundPlayed[numOfInputs];
int wifiTimeout = 25000;
bool doBluetoothSeperation = false;

Preferences preferences;

// Replace with your network credentials
String ID = "ESP32 to ERM";
String ssid = "Ozark Escape";
String password = "0zark3scap3";
IPAddress local_IP(192, 168, 0, 200);
IPAddress subnet(255, 255, 255, 0);
IPAddress gateway(192, 168, 0, 1);

BLEServer* pServer = NULL;
BLECharacteristic* pCharacteristic = NULL;


bool deviceConnected = false; // bluetooth
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

bool bluetoothMode = false;

void disableBluetooth() {
  if (doBluetoothSeperation) {
    bluetoothMode = false;
    BLEDevice::deinit(); // Stop the Bluetooth stack
    Serial.println("Bluetooth disabled, reconnecting wifi...");
  }
}

void enableBluetooth() {
  bluetoothMode = true;
  Serial.print("Bluetooth enabled");
  if (doBluetoothSeperation) {
    Serial.println(", disabling wifi...");
    WiFi.disconnect(true);
  } else { Serial.println("!"); }
  // CREATE BLUETOOTH DEVICE
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
  Serial.println("Created bluetooth device...");
  Serial.println("Configure WiFi here: https://purplecraft.net/bluetooth.html");
}

// Set web server port number to 80
AsyncWebServer server(80);

void setup() {
  Serial.begin(115200);
  Serial.println("Booting...");

  pinMode(BLUETOOTH_PIN, INPUT_PULLUP);

  char buf[18];
  // CONFIGURE INPUTS
  for (int n = 0; n < numOfInputs; n++) {
    pinMode(inputs[n], INPUT_PULLUP);
    snprintf(buf, 18, "/input/%d", n+1);
    server.on(buf, HTTP_GET, [n](AsyncWebServerRequest *request){
      AsyncResponseStream *response = request->beginResponseStream("text/html");
      CheckPin(response, digitalRead(inputs[n]) == LOW);
      response->addHeader("Access-Control-Allow-Origin","*");
      request->send(response);
    });
    snprintf(buf, 18, "/input/%d/sound", n+1); // PLAYS SOUND ONLY ONCE
    server.on(buf, HTTP_GET, [n](AsyncWebServerRequest *request){
      AsyncResponseStream *response = request->beginResponseStream("text/html");
      if (digitalRead(inputs[n]) == LOW) {
        if ( soundStates[n] == HIGH ) {
          soundStates[n] = LOW;
          CheckPin(response, true);
        }
        if (soundStates[n] == LOW) {
          CheckPin(response, false);
        }
      } else {
        soundStates[n] = HIGH;
        CheckPin(response, false);
      }
      response->addHeader("Access-Control-Allow-Origin","*");
      request->send(response);
    });
  }

  char buf2[18];
  // CONFIGURE OUTPUTS
  for (int n = 0; n < numOfInputs; n++) {
    pinMode(outputs[n], OUTPUT);
    snprintf(buf2, 18, "/output/%d/high", n+1);
    server.on(buf2, HTTP_GET, [n](AsyncWebServerRequest *request){
      AsyncResponseStream *response = request->beginResponseStream("text/html");
      digitalWrite(outputs[n], HIGH);
      CheckPin(response, digitalRead(outputs[n]) == LOW);
      response->addHeader("Access-Control-Allow-Origin","*");
      request->send(response);
    });
    snprintf(buf2, 18, "/output/%d/low", n+1);
    server.on(buf2, HTTP_GET, [n](AsyncWebServerRequest *request){
      AsyncResponseStream *response = request->beginResponseStream("text/html");
      digitalWrite(outputs[n], LOW);
      CheckPin(response, digitalRead(outputs[n]) == LOW);
      response->addHeader("Access-Control-Allow-Origin","*");
      request->send(response);
    });
    snprintf(buf2, 18, "/output/%d", n+1);
    server.on(buf2, HTTP_GET, [n](AsyncWebServerRequest *request){
      AsyncResponseStream *response = request->beginResponseStream("text/html");
      if (digitalRead(outputs[n]) == LOW) {
        digitalWrite(outputs[n], HIGH);
      } else {
        digitalWrite(outputs[n], LOW);
      }
      CheckPin(response, digitalRead(outputs[n]) == LOW);
      response->addHeader("Access-Control-Allow-Origin","*");
      request->send(response);
    });
  }

  // CONFIGURE HOME PAGES
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    AsyncResponseStream *response = request->beginResponseStream("text/html");
    GetMainMenu(response);
    request->send(response);
  });
  server.onNotFound([](AsyncWebServerRequest *request){
    AsyncResponseStream *response = request->beginResponseStream("text/html");
    GetMainMenu(response);
    request->send(response);
  });

  // READ NETWORK CONFIG
  preferences.begin("Net-Config", false); 
  readData();

  // ENABLED BLUETOOTH
  if (!doBluetoothSeperation) {
    enableBluetooth();
  }
}

void CheckPin(AsyncResponseStream *response, bool changed) {
  String status = (changed) ? "triggered" : "not triggered";
  response->print(status);
}

void GetMainMenu(AsyncResponseStream *response) {
  //Serial.println("Polled.");
  response->print("<!DOCTYPE HTML>");
  response->print("<html>");
  response->print("<head>");
  response->print("<style>");
  response->print("body { background-color: #111111; font-family:Roboto }");
  response->print("h1 { color: #ffffff; }");
  response->print("h3 { color: #999999; }");
  response->print("</style>");
  response->print("</head>");
  response->print("<body {background-color: #111111;}>");
  response->print("<img src='https://static.wixstatic.com/media/6e6fcf_230f10c631da4717a2d87b0e96cd93f9~mv2_d_8001_4178_s_4_2.png/v1/crop/x_2,y_943,w_7999,h_2240/fill/w_489,h_137,al_c,q_85,usm_0.66_1.00_0.01,enc_auto/OE8_Primrary2_White.png' alt='Wix.com'>");
  response->print("<h1>ESP32 to Escape Room Master</h1>");
  response->print("<h3>EZESP32 By Ryan</h3>");
  response->print("<h3></h3>");
  response->print("<h1>"+ID+"</h1>");
  response->print("<h3></h3>");
  char buf[50];
  snprintf(buf, 50, "<h3>/input/1-%d</h3>", numOfInputs);
  response->print(buf);
  snprintf(buf, 50, "<h3>/input/1-%d/sound</h3>", numOfInputs);
  response->print(buf);
  snprintf(buf, 50, "<h3>/output/1-%d</h3>", numOfOutputs);
  response->print(buf);
  snprintf(buf, 50, "<h3>/output/1-%d/high</h3>", numOfOutputs);
  response->print(buf);
  snprintf(buf, 50, "<h3>/output/1-%d/low</h3>", numOfOutputs);
  response->print(buf);
  snprintf(buf, 50, "<h3>/output/1-%d (toggle)</h3>", numOfOutputs);
  response->print(buf);
  response->print("</body>");
  response->print("</html>");
}

bool startsWith(const char *pre, const char *str)
{
    size_t lenpre = strlen(pre),
           lenstr = strlen(str);
    return lenstr < lenpre ? false : memcmp(pre, str, lenpre) == 0;
}

void CheckBluetoothForData() {

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
        const char* newID = doc["id"];
        const char* newSsid = doc["ssid"];
        const char* newPassword = doc["password"];
        
        preferences.putString("ID", newID); 
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
            preferences.putInt("subnet"+i, subnet[i]);
          }
          if (doc["gateway"][i].is<int>()) {
            gateway[i] = doc["gateway"][i].as<int>();
            preferences.putInt("gateway"+i, gateway[i]);
          }
        }
        Serial.println("Info Saved.");
        readData();
        disableBluetooth();
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

void readData() {

  // Extract credentials for wifi.
  ID = preferences.getString("ID");
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
  Serial.print("ID: "); Serial.println(ID);
  Serial.print("SSID: "); Serial.println(ssid);
  Serial.print("Password: "); Serial.println(password);
  Serial.print("Local IP: "); Serial.println(local_IP);
  Serial.print("Subnet: "); Serial.println(subnet);
  Serial.print("Gateway: "); Serial.println(gateway);

}

bool onlyOnce = false;
int totalTimeToConnect = 0;
void loop() {

  if (deviceConnected) { // BLUETOOTH CONNECTED
    neopixelWrite(RGB_BUILTIN,0,0,RGB_BRIGHTNESS); // Blue
  } else if (!deviceConnected && WiFi.status() != WL_CONNECTED && !(doBluetoothSeperation && !bluetoothMode)) { // WIFI DISCONNECTED, CONNECTING
    neopixelWrite(RGB_BUILTIN,RGB_BRIGHTNESS,0,0); // Red
  } else if (WiFi.status() == WL_CONNECTED) { // ALL SYSTEMS GOOD
    neopixelWrite(RGB_BUILTIN,0,RGB_BRIGHTNESS/2,0); // Green
  } else if (bluetoothMode) { // BLUETOOTH PAIRING MODE ENABLED
    neopixelWrite(RGB_BUILTIN,RGB_BRIGHTNESS,RGB_BRIGHTNESS,RGB_BRIGHTNESS); // White
  }


  if (totalTimeToConnect > wifiTimeout) {
    Serial.println("WiFi Timed Out! Switching to bluetooth config tool:");
    totalTimeToConnect = 0;
    enableBluetooth();
  }
  if (WiFi.status() != WL_CONNECTED) {
    attemptWifiConnection();
  }
  if (bluetoothMode) {
    totalTimeToConnect = 0;
    CheckBluetoothForData();
  }
  if (digitalRead(BLUETOOTH_PIN) == LOW && !onlyOnce) {
    onlyOnce = true;
    Serial.println("Bluetooth Button Pushed, Enabling bluetooth..");
    enableBluetooth();
  } else {
    onlyOnce = false;
  }
  delay(20);
}

void attemptWifiConnection() {
  if (doBluetoothSeperation && bluetoothMode) {
    return;
  }
    // Configures static IP address
  if (!WiFi.config(local_IP, gateway, subnet)) {
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
  int timeout = millis() + 9500;
  int nextDot = millis();
  while ( !(WiFi.status() == WL_CONNECTED || timeout < millis() ) ) {
    if (nextDot <= millis()) {
      nextDot = nextDot + 300;
      Serial.print(".");
    }
    if (doBluetoothSeperation) {
      totalTimeToConnect = totalTimeToConnect + 20;
    }
    delay(20);
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
  }
}