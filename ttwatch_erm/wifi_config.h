#include <Preferences.h>

Preferences preferences;

// CONFIG 
const char *ssid = "Ryan&Angela";
const char *password = "14727890";
//IPAddress
IPAddress staticIP(192, 168, 0, 113);
IPAddress gateway(192, 168, 0, 1);
IPAddress subnet(255, 255, 255, 0);
IPAddress dns(8, 8, 8, 8);
IPAddress dns2(8, 8, 4, 4);

void saveNetworkConfigToPreferences() {
  preferences.begin("network_config", false);

  // Store WiFi credentials
  preferences.putString("ssid", "YourSSID");
  preferences.putString("password", "YourPassword");

  // Store individual IP address components
  preferences.putUInt("staticIP_1", 192);
  preferences.putUInt("staticIP_2", 168);
  preferences.putUInt("staticIP_3", 0);
  preferences.putUInt("staticIP_4", 113);

  preferences.putUInt("gateway_1", 192);
  preferences.putUInt("gateway_2", 168);
  preferences.putUInt("gateway_3", 0);
  preferences.putUInt("gateway_4", 1);

  preferences.putUInt("subnet_1", 255);
  preferences.putUInt("subnet_2", 255);
  preferences.putUInt("subnet_3", 255);
  preferences.putUInt("subnet_4", 0);

  preferences.putUInt("dns_1", 8);
  preferences.putUInt("dns_2", 8);
  preferences.putUInt("dns_3", 8);
  preferences.putUInt("dns_4", 8);

  preferences.putUInt("dns2_1", 8);
  preferences.putUInt("dns2_2", 8);
  preferences.putUInt("dns2_3", 4);
  preferences.putUInt("dns2_4", 4);

  preferences.end();
}

void applyNetworkConfigFromPreferences() {
  preferences.begin("network_config", true);

  // Retrieve individual IP address components
  IPAddress staticIP(preferences.getUInt("staticIP_1", 192), preferences.getUInt("staticIP_2", 168),
                    preferences.getUInt("staticIP_3", 0), preferences.getUInt("staticIP_4", 113));

  IPAddress gateway(preferences.getUInt("gateway_1", 192), preferences.getUInt("gateway_2", 168),
                   preferences.getUInt("gateway_3", 0), preferences.getUInt("gateway_4", 1));

  IPAddress subnet(preferences.getUInt("subnet_1", 255), preferences.getUInt("subnet_2", 255),
                  preferences.getUInt("subnet_3", 255), preferences.getUInt("subnet_4", 0));

  IPAddress dns(preferences.getUInt("dns_1", 8), preferences.getUInt("dns_2", 8),
               preferences.getUInt("dns_3", 8), preferences.getUInt("dns_4", 8));

  IPAddress dns2(preferences.getUInt("dns2_1", 8), preferences.getUInt("dns2_2", 8),
                preferences.getUInt("dns2_3", 4), preferences.getUInt("dns2_4", 4));

  preferences.end();

  // Use retrieved values to configure WiFi
  WiFi.config(staticIP, gateway, subnet, dns, dns2);
}

uint8_t extractOctet(const char *str, int octetNumber) {
  // This function extracts a specific octet (based on octetNumber) from the input string
  // For example, if the input string is "192.168.0.1", and octetNumber is 1, it will extract "192"
  char* octet = strtok((char*)str, ".");
  for (int i = 0; i < octetNumber; i++) {
    octet = strtok(NULL, ".");
  }
  return atoi(octet);
}

void updateNetworkConfig() {
  char userInput[50];

  Serial.print("Enter Local IP Address (current: ");
  Serial.print(preferences.getUInt("staticIP_1", 192));
  Serial.print(".");
  Serial.print(preferences.getUInt("staticIP_2", 0));
  Serial.print(".");
  Serial.print(preferences.getUInt("staticIP_3", 0));
  Serial.print(".");
  Serial.print(preferences.getUInt("staticIP_4", 0));
  Serial.println("): ");

  while (!Serial.available()) {}
  Serial.readBytesUntil('\n', userInput, sizeof(userInput));
  preferences.putUInt("staticIP_1", extractOctet(userInput, 0)); // Extract first octet and store
  preferences.putUInt("staticIP_2", extractOctet(userInput, 1)); // Extract second octet and store
  preferences.putUInt("staticIP_3", extractOctet(userInput, 2)); // Extract third octet and store
  preferences.putUInt("staticIP_4", extractOctet(userInput, 3)); // Extract fourth octet and store

  // Update other IP components in a similar manner

  Serial.println("Network Configuration updated.");
  applyNetworkConfigFromPreferences(); // Apply the updated configuration
}


void loopNetConfig() {
  if (Serial.available() > 0) {
    String input = Serial.readStringUntil('\n');
    input.trim();

    if (input.equals("netconfig")) {
      Serial.println("Starting Network Configuration Update...");
      updateNetworkConfig();
    }
  }
}