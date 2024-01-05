#include <ESPAsyncWebSrv.h>
#include <Arduino.h>
#include <WiFi.h>
#include <EEPROM.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include <ArduinoJson.h> // Include the ArduinoJSON library

#define SERVICE_UUID "6e400001-b5a3-f393-e0a9-e50e24dcca9e"
#define CHARACTERISTIC_UUID "6e400002-b5a3-f393-e0a9-e50e24dcca9e"

// Define the addresses in EEPROM where data will be stored
#define SSID_ADDR 0   // Starting address for SSID
#define PASS_ADDR 128  // Starting address for password
#define IP_ADDR 256    // Starting address for local IP address
#define SUB_ADDR 384    // Starting address for local IP address
#define GATE_ADDR 512    // Starting address for local IP address
#define MAX_LEN 128    // Maximum length for each data field

// Set web server port number to 80
AsyncWebServer server(80);

char ssid[MAX_LEN];
char password[MAX_LEN];
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
  response->print("<img src='https://static.wixstatic.com/media/6e6fcf_230f10c631da4717a2d87b0e96cd93f9~mv2_d_8001_4178_s_4_2.png/v1/crop/x_2,y_943,w_7999,h_2240/fill/w_489,h_137,al_c,q_85,usm_0.66_1.00_0.01,enc_auto/OE8_Primrary2_White.png' alt='Ozark Escape'>");
  response->print("<h1>EZEsp32</h1>");
  response->print("<h1>By Ryan</h1>");
  response->print("<h3>I'm working just fine.</h3>");
  response->print("<h3>Test ESP32.</h3>");
  response->print("<h3>/ - index</h3>");
  response->print("</body>");
  response->print("</html>");
}

void setup() {
    Serial.begin(115200);
    Serial.println("Starting up...");
    EEPROM.begin(1024); // Initialize EEPROM with a size of 1024 bytes
    readDataFromEEPROM();

    // Get and print the ESP32S3's MAC address
    uint8_t espMac[6];
    esp_read_mac(espMac, ESP_MAC_WIFI_STA);
    Serial.printf("ESP32S3 MAC Address: %02X:%02X:%02X:%02X:%02X:%02X\n", espMac[0], espMac[1], espMac[2], espMac[3], espMac[4], espMac[5]);

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
    Serial.println("Waiting for a connection...");

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
                const char* ssidValue = doc["ssid"];
                const char* passwordValue = doc["password"];

                // Safely copy the values into your character arrays
                strncpy(ssid, ssidValue, sizeof(ssid) - 1);
                strncpy(password, passwordValue, sizeof(password) - 1);

                // Ensure null-termination of the character arrays
                ssid[sizeof(ssid) - 1] = '\0';
                password[sizeof(password) - 1] = '\0';

                // Extract IP addresses
                for (int i = 0; i < 4; i++) {
                    local_IP[i] = doc["localIp"][i].as<int>();
                    subnet[i] = doc["subnet"][i].as<int>();
                    gateway[i] = doc["gateway"][i].as<int>();
                }

                Serial.println("Saving Info");
                // Save the data to EEPROM (you can call your writeDataToEEPROM function here)
                writeDataToEEPROM();
                Serial.println("Info Saved.");
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

void loop() {
  CheckBluetoothForData();
  if (WiFi.status() != WL_CONNECTED) {
    attemptWifiConnection();
  }
}

void readDataFromEEPROM() {
  // Read SSID from EEPROM
  for (int i = 0; i < MAX_LEN; i++) {
    ssid[i] = EEPROM.read(SSID_ADDR + i);
    if (ssid[i] == '\0') break; // Null-terminated string
  }

  // Read password from EEPROM
  for (int i = 0; i < MAX_LEN; i++) {
    password[i] = EEPROM.read(PASS_ADDR + i);
    if (password[i] == '\0') break; // Null-terminated string
  }

  // Read local IP address from EEPROM
  for (int i = 0; i < 4; i++) {
    local_IP[i] = EEPROM.read(IP_ADDR + i);
  }

  // Read subnet from EEPROM
  for (int i = 0; i < 4; i++) {
    subnet[i] = EEPROM.read(SUB_ADDR + i);
  }

  // Read default gateway from EEPROM
  for (int i = 0; i < 4; i++) {
    gateway[i] = EEPROM.read(GATE_ADDR + i);
  }

  // Print the read data
  Serial.println("Saved data:");
  Serial.print("SSID: "); Serial.println(ssid);
  Serial.print("Password: "); Serial.println(password);
  Serial.print("Local IP: "); Serial.println(local_IP);
  Serial.print("Subnet: "); Serial.println(subnet);
  Serial.print("Gateway: "); Serial.println(gateway);
}

void writeDataToEEPROM() {
  // Write SSID to EEPROM
  for (int i = 0; i < strlen(ssid); i++) {
    EEPROM.write(SSID_ADDR + i, ssid[i]);
  }

  // Write password to EEPROM
  for (int i = 0; i < strlen(password); i++) {
    EEPROM.write(PASS_ADDR + i, password[i]);
  }

  // Write local IP address to EEPROM
  for (int i = 0; i < 4; i++) {
    EEPROM.write(IP_ADDR + i, local_IP[i]);
  }

  // Write subnet to EEPROM
  for (int i = 0; i < 4; i++) {
    EEPROM.write(SUB_ADDR + i, subnet[i]);
  }

  // Write default gateway to EEPROM
  for (int i = 0; i < 4; i++) {
    EEPROM.write(GATE_ADDR + i, gateway[i]);
  }

  EEPROM.commit(); // Save changes to EEPROM
}

void attemptWifiConnection() {
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
  int timeout = millis() + 10000;
  while ( !(WiFi.status() == WL_CONNECTED || timeout < millis() ) ) {
    delay(100);  // LED BLINKS WHEN TRYING TO CONNECT
    if (blink) {
      digitalWrite(LED_BUILTIN, HIGH);
      blink = false;
    } else {
      digitalWrite(LED_BUILTIN, LOW);
      blink = true;
    }
    Serial.print(".");
    CheckBluetoothForData();
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
    digitalWrite(LED_BUILTIN, LOW);
    delay(50);
    digitalWrite(LED_BUILTIN, HIGH);
    delay(50);
    digitalWrite(LED_BUILTIN, LOW);
    delay(50);
    digitalWrite(LED_BUILTIN, HIGH);
  }
}