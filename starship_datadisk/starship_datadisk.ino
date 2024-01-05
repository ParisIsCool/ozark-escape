/**
 * Floppy Disc Console (c) 2022 Alastair Aitchison
 *
 * A device to simulate uploading/downloading data from a floppy disc,
 * such as those (allegedly) used to store the U.S. miltary nuclear launch codes.
 *
 * Players must find three floppy discs, insert them into the drive, and select the
 * correct file from each to upload. Once all three items of data have been loaded,
 * a message can be displayed, a relay triggered etc. etc.
 */


#include <ESPAsyncWebSrv.h>
#define LED_BUILTIN 1
// Replace with your network credentials
const char* ssid = "OzarkEscape";
const char* password = "Spunky44!";

// Set your Static IP address
// This is what is used to connect to it in ERM
// IMPORTANT: This must be unique or each arduino!
IPAddress local_IP(192, 168, 1, 103);

// Set your Gateway IP address
IPAddress gateway(192, 168, 1, 1);

IPAddress subnet(255, 255, 255, 0);
IPAddress primaryDNS(8, 8, 8, 8);    //optional
IPAddress secondaryDNS(8, 8, 4, 4);  //optional

// Set web server port number to 80
AsyncWebServer server(80);

// INCLUDES
// The LCD display uses an I2C interface provided by Arduino's "Wire" library
#include <Wire.h>
// LCD display module with I2C interface.See https://github.com/duinoWitchery/hd44780
#include <hd44780.h>
#include <hd44780ioClass/hd44780_I2Cexp.h>
// RFID reader library. See https://github.com/miguelbalboa/rfid
#include <MFRC522.h>




// STRUCTURES
// This data structure encapsulates all the properties associated with an individual file
struct file {
  char fileName[16]; // The name for this file as will appear on the display
  unsigned int fileSize; // From 0-65,535
  unsigned long fileDate; // Timestamp
};
// This data structure contains info relating to each "disk" (RFID card) that is scanned
struct card {
  byte rfidUID[4]; // Unique identifier of the RFID tag  
  file contents[4]; // The array of files this card contains
  byte correctFileID; // The file that needs uploading from this disc
};




// CONSTANTS
// Puzzle Complete Output
const byte relayPin = 2;
// The number of different cards players can insert
const int numTags = 5;
// The number of files contained on each card
const byte numFiles = 4;
// Define the data held on each "disk"
const card cardList[numTags] = {
  { {0x91,0x37, 0x78, 0x1D}, { {"bootsect.dos", 100, 1}, {"ntbootdd.sys", 200, 1}, {"ntldr", 100, 1}, {"ntoskrnl.exe", 100, 1}, }, 0 },
  { {0x89,0xE3, 0x44, 0x02}, { {"Flight Data v2", 300, 1}, {"ERROR_009", 200, 1}, {"NAVDATA", 100, 1}, {"Flight Data v3", 100, 1}, }, 2},
  { {0xEB,0xCC, 0xC44, 0x02}, { {"File 250", 400, 1}, {"File 284", 100, 1}, {"File 615", 100, 1}, {"File 813", 100, 1}, }, 1},
  // CHANGE THESE TWO
  { {0xC4,0xA7, 0x44, 0x02}, { {"D-Bus", 100, 1}, {"Network Manager", 100, 1}, {"Avahi", 100, 1}, {"Pulse", 100, 1}, }, 5},
  { {0x2B,0xC7, 0xC44, 0x02}, { {"I/O", 100, 1}, {"startup repair", 100, 1}, {"driver connect", 100, 1}, {"memory purge", 100, 1}, }, 5},
};
// GPIO pins attached to each component
// Rotary encoder/button
const byte encDataPin = 26;
const byte encClockPin = 25;
const byte buttonPin = 27;
// Green LED that indicates floppy drive is active
const byte floppyLedPin = 32;
// Chip select line to the RFID reader
const byte rfidSS = 5;
// Blue LEDs that indicate which data has been loaded
const byte ledPins[] = {4, 16, 17};




// GLOBALS
// Initialise the LCD display (the HD44780 library auto-detects I2C address)
hd44780_I2Cexp lcd;
// Create an instance of the MFRC522 class, using the specified chip select pin
// We won't use a RST line, so set that to UINT8_MAX
MFRC522 rfid(rfidSS, UINT8_MAX);
// Is a disc currently inserted?
bool isDiscPresent = false;
// The index of the currently inserted disc
int8_t currentCard = -1;
// The file selected on the currently inserted disc
uint8_t selectedFile = 0;
// Timestamp at which we last checked whether disc was changed
unsigned long timeSinceLastScan;
// The cards whose data has been successfully laoded
bool dataLoaded[] = {false, false, false};


bool triggered = false;

void setup() {
  // Serial connection is intialised first, as used to print
  // debugging information to Arduino serial monitor
  Serial.begin(115200);
  Serial.println(__FILE__ __DATE__);




  Serial.print(F("Setting pin modes..."));
  // Encoder
  pinMode(buttonPin, INPUT_PULLUP);
  pinMode(encClockPin, INPUT_PULLUP);
  pinMode(encDataPin, INPUT_PULLUP);
  // LEDs
  pinMode(floppyLedPin, OUTPUT);
  digitalWrite(floppyLedPin, LOW);
  for(int i=0; i<3; i++){
    pinMode(ledPins[i], OUTPUT);
    digitalWrite(ledPins[i], LOW);
  }
  Serial.println(F("done."));




  Serial.print(F("Initialising I2C interface..."));
  // Initialise the I2C interface
  Wire.begin();
  Serial.println(F("done."));




  // LCD
  Serial.print(F("Initialising LCD display..."));
  // Initialise the display with specified number of columns/rows
  lcd.begin(20,4);
  lcd.setBacklight(255);
  // Create custom characters to draw the bargraph on the LCD display
  // Up to 8 custom chars can be saved, which can be drawn by their index, e.g. lcd.write(byte(0));
  // (Here we use 7 custom chars, so there's one spare left)
  // For a graphical tool to help create this data, see https://www.quinapalus.com/hd44780udg.html
  uint8_t customChars[][8] = {
    {B01111, B11000, B10000, B10000, B10000, B10000, B11000, B01111}, // Start of bar, empty
    {B01111, B11000, B10011, B10111, B10111, B10011, B11000, B01111}, // Start of bar, filled
    {B11111, B00000, B00000, B00000, B00000, B00000, B00000, B11111}, // Middle of bar, empty
    {B11111, B00000, B11000, B11000, B11000, B11000, B00000, B11111}, // Middle of bar, half-filled
    {B11111, B00000, B11011, B11011, B11011, B11011, B00000, B11111}, // Middle of bar, filled
    {B11110, B00011, B00001, B00001, B00001, B00001, B00011, B11110}, // End of bar, empty
    {B11110, B00011, B11001, B11101, B11101, B11001, B00011, B11110}, // End of bar, filled
  };
  // Copy the custom characters to the LCD memory
  for (int i=0; i<7; i++){
    lcd.createChar(i, customChars[i]);
  }
  lcd.clear();
  lcd.home();
  lcd.print("S.O.L. Interface");  
  lcd.setCursor(0, 2);
  lcd.print("Insert chip to");
  lcd.setCursor(0, 3);
  lcd.print("upload file data");
  Serial.println(F("done."));




  Serial.print(F("Initialising RFID reader..."));
  // Initialise the SPI bus
  SPI.begin();
  // Initialise the RFID reader
  rfid.PCD_Init();
  // Wait for stabilisation
  delay(100);
  rfid.PCD_DumpVersionToSerial();
  Serial.println(F("done."));


  server.on("/completed", HTTP_GET, [](AsyncWebServerRequest *request){
    AsyncResponseStream *response = request->beginResponseStream("text/html");
    CheckPin(response, triggered);
    response->addHeader("Access-Control-Allow-Origin","*");
    request->send(response);
  });

  server.on("/reset", HTTP_GET, [](AsyncWebServerRequest *request){
    AsyncResponseStream *response = request->beginResponseStream("text/html");
    response->print("resetting prop");
    response->addHeader("Access-Control-Allow-Origin","*");
    request->send(response);
    delay(500);
    ESP.restart();
  });

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




void loop() {
  // Check every frame whether the encoder has been turned (even if not required)
  int8_t knobChange = readRotary();

  runWifiLoop();


  // Has sufficient time elapsed since last checking the disc?
  if(millis() - timeSinceLastScan > 1000){
    timeSinceLastScan = millis();
    // Check if any tag within range can be read
    if (PICC_IsAnyCardPresent() && rfid.PICC_ReadCardSerial()) {




      // Light up the floppy disc active LED
      digitalWrite(floppyLedPin, HIGH);
     
      // If there wasn't previously a card present
      if(!isDiscPresent) {
        isDiscPresent = true;
       
        // Create a string of the tag ID (8 char ID + 1 byte terminator)
        char buffer[9];
        sprintf(buffer, "%02X%02X%02X%02X", rfid.uid.uidByte[0], rfid.uid.uidByte[1], rfid.uid.uidByte[2], rfid.uid.uidByte[3]);




        // Debug output to serial
        Serial.print(F("Card detected! "));
        Serial.println(buffer);




        // Update LCD screen
        lcd.clear();
        lcd.home();
        lcd.print("Chip ID:");
        lcd.print(buffer);
        lcd.setCursor(0,2);
        lcd.print("Accessing Files");




        // Draw progress bar
        for(int percent=0; percent<101; percent++){
          showProgressBar(percent, 0, 3, 20);
          delay(10);
        }




        // Find out what disk has been inserted
        currentCard = -1;
        for(int i=0; i<numTags; i++) {




          // If the ID of the disc current inserted matches one of those in the cardList array
          if(memcmp(cardList[i].rfidUID, rfid.uid.uidByte, sizeof cardList[i].rfidUID) == 0) {




            // Note the index of which card is inserted
            currentCard = i;
   
            // Print the disk contents on the LCD display
            lcd.clear();
            showFileList(currentCard);
            showCursor(selectedFile);
            break;
          }
        }
        // If we're *not* able to find a matching ID in the array of know tags
        if(currentCard == -1) {
          // This section of code will only be reached if a card is scanned that is not recognised
          // i.e. it shouldn't happen!
          lcd.setCursor(0,2);
          lcd.print(F("Chip Not Recognised"));
          lcd.setCursor(0,3);
          for(int n=0; n<20; n++) {
            lcd.print(" ");
          }
        }
      }
    }
    // We used to have a card inserted, but can no longer find one
    else if(isDiscPresent){
      Serial.println(F("Chip Removed!"));
      lcd.clear();
      lcd.home();
      lcd.print("S.O.L. Interface");  
      lcd.setCursor(0, 2);
      lcd.print("Insert chip to");
      lcd.setCursor(0, 3);
      lcd.print("upload file data");
      isDiscPresent = false;
      digitalWrite(floppyLedPin, LOW);
    }
     
    // Tell PICC to halt
    rfid.PICC_HaltA();
    // Stop the PCD
    rfid.PCD_StopCrypto1();
  }




  // We only scroll through files when a disc has been inserted!
  if(isDiscPresent){
    // And it has been recognised
    if(currentCard != -1) {
      // If scroll wheel has moved
      if(knobChange){
        // Update the selected file
        selectedFile = constrain(selectedFile + knobChange, 0, numFiles-1);
        Serial.print(selectedFile);
        showCursor(selectedFile);
      }
      // If the button is clicked
      if(digitalRead(buttonPin) == LOW){
        // Upload the currently selected file
        upload();
        // Wait for the button to be released
        while(digitalRead(buttonPin) == LOW){
          delay(10);
        }
      }
    }
  }
  // If the disc is *not* present
  else {
    // Nothing to see here...
  }








  // Check whether all data has been uploaded...
  bool allDataLoaded = true;
  for(int i=0; i<3; i++) {
    if(dataLoaded[i] == false) {
      allDataLoaded = false;
      break;
    }
  }
  // ...if so
  if(allDataLoaded){




    // Update the display
    lcd.clear();
    lcd.setCursor(1, 0);
    lcd.print(F("DATA LOAD COMPLETE"));
    lcd.setCursor(2, 2);
    lcd.print(F("SYSTEM RESTORED"));




    // Insert any desired logic here to, e.g. trigger a relay etc.
    while(true) {
      digitalWrite(relayPin, LOW);
      triggered = true;
      delay(100);
    }
  }
}




/**
 * Display the set of files contained on a disc
 */
void showFileList(uint8_t cardNum){
  if(cardNum < 0 || cardNum >= numTags) {
    Serial.print("ERROR");
    return;
  }
  Serial.print("Printing contents of disc #");
  Serial.println(cardNum);
  for(int i=0; i<numFiles; i++){
    // Choose what info to display about each file - using fileName, fileSize, fileDate
    char line[19];
    memset(line, 0, sizeof line);
    snprintf(line, 19, "%s", cardList[currentCard].contents[i].fileName);
    // Debug
    Serial.print(" -");
    Serial.println(line);
    // Output to LCD display
    // Start at first column in to leave room for cursor
    lcd.setCursor(1,i);
    lcd.print(line);
  }
}




/**
 * Display the cursor that highlights the currently selected file
 */
void showCursor(uint8_t lineNum) {
  // Show the cursor
  for(int i=0; i<numFiles; i++){
    lcd.setCursor(0,i);
    lcd.print( i==lineNum ? ">" : " ");
  }
}




/**
 * Simulates the action of "uploading" the currently selected file,
 * displaying a progress bar and then reporting if successful
 */
void upload() {
  // Update the LCD display
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(F("Uploading"));
  lcd.setCursor(0, 2);
  lcd.print(cardList[currentCard].contents[selectedFile].fileName);




  // Progress bar percentage value
  static byte percent = 0;
  for(percent=0; percent<=100; percent++){
    // Display numeric percent value on the top line
    lcd.setCursor(20-4, 0);
    char msg[4];
    // This can look confusing - %3d is a padded 3-digit number, whereas trailing %% prints a literal percent sign
    sprintf(msg, "%3d%%", percent);
    lcd.print(msg);
   
    // Draw bar 20 units wide starting at (0,3)
    showProgressBar(percent, 0, 3, 20 );




    delay(10);
  }
  // If the file uploaded is the correct file for the currently inserted disc...
  if(selectedFile == cardList[currentCard].correctFileID){
   
    // Update the array
    dataLoaded[currentCard] = true;
   
    // Light up the LED corresponding to this card
    digitalWrite(ledPins[currentCard], HIGH);




    // Update the LCD display
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(F("Success:"));
    lcd.setCursor(0, 2);
    lcd.print(F("Upload Complete"));
  }
 
  // ... the wrong file has been uploaded
  else {
    // Update the array
    dataLoaded[currentCard] = false;




    // Turn off the corresponding LED
    digitalWrite(ledPins[currentCard], LOW);




    // Update the LCD display
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(F("Error:"));
    lcd.setCursor(0, 2);
    lcd.print(F("Data Corrupt"));
  }  




  // Pause a little
  delay(3000);
  lcd.clear();




  // Return to the file listing of the currently inserted disc
  showFileList(currentCard);
  showCursor(selectedFile);
}




/**
 * Draw a progress bar on LCD display using custom characters
 * Based on code by Fabien: https://www.carnetdumaker.net/articles/faire-une-barre-de-progression-avec-arduino-et-liquidcrystal/
 */
void showProgressBar(byte percent, int x, int y, int width) {
 
  // Move the cursor to the specified start point
  lcd.setCursor(x, y);




  // Map value from 0-100 percentage range to width of the bar
  // Each character displays 2 vertical bars, but the first and last character display only one.
  // nb_columns is the number of columns that still need to be drawn
  byte nb_columns = map(percent, 0, 100, 0 , width * 2 - 2);




  // Draw each segment of the line
  for (byte i=0; i < width; ++i) {
    // First segment
    if (i==0 ) {
      if(nb_columns > 0 ) {
        lcd.write(1); // Filled left-end bar
        nb_columns -=  1 ;
      } else {
        lcd.write((byte) 0 ); // Empty left-end bar
      }
    }
    // Last segment
    else if (i == width -1 ) {
      // Filled end segment
      if (nb_columns >  0 ) {
        lcd.write(6);
      }
      // Empty end segment
      else {
        lcd.write(5);
      }
    }
    // Every middle segment
    else {
      if (nb_columns >=  2) {
        // Filled
        lcd.write(4);
        nb_columns -= 2;
      } else if(nb_columns == 1 ) {
        // Half-filled
        lcd.write(3);
        nb_columns -= 1;
      }
      else {
        // Empty
        lcd.write(2);
      }
    }
  }
}




/**
 * Reads output from rotary encoder
 * A valid CW move returns 1, CCW returns -1, invalid returns 0.
 * See https://www.best-microcontroller-projects.com/rotary-encoder.html
 */
int8_t readRotary() {
  // Enumeration of whether pairs of "previous state, next state" values are valid
  // e.g. 00->00 is invalid, so 0
  //      00->01 is valid, so 1
  //      00->10 is valid, so 1
  //      00->11 is invalid, so 0 etc., etc.
  static int8_t rot_enc_table[] = {0,1,1,0, 1,0,0,1, 1,0,0,1, 0,1,1,0};
  // Used to debounce knob reading
  static uint8_t prevNextCode = 0;
  static uint16_t store=0;




  // Create the 4bit PS-NS code for the current state
  prevNextCode <<= 2;
  if (digitalRead(encDataPin)) prevNextCode |= 0x02;
  if (digitalRead(encClockPin)) prevNextCode |= 0x01;
  prevNextCode &= 0x0f;




  // Check if state is valid
  if (rot_enc_table[prevNextCode] ) {
    // Store latest state
    store <<= 4;
    store |= prevNextCode;
    if ((store&0xff)==0x2b) return 1;
    if ((store&0xff)==0x17) return -1;
  }
  return 0;
}


/**
 * Returns true if a PICC responds to PICC_CMD_WUPA.
 * All cards in state IDLE or HALT are invited.
 *
 * @return bool
 */
bool PICC_IsAnyCardPresent() {
  byte bufferATQA[2];
  byte bufferSize = sizeof(bufferATQA);
 
  // Reset baud rates
  rfid.PCD_WriteRegister(rfid.TxModeReg, 0x00);
  rfid.PCD_WriteRegister(rfid.RxModeReg, 0x00);
  // Reset ModWidthReg
  rfid.PCD_WriteRegister(rfid.ModWidthReg, 0x26);
 
  MFRC522::StatusCode result = rfid.PICC_WakeupA(bufferATQA, &bufferSize);
  return (result == MFRC522::STATUS_OK || result == MFRC522::STATUS_COLLISION);
} // End PICC_IsAnyCardPresent()


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
  response->print("<h1>Arduino to Escape Room Master</h1>");
  response->print("<h1>By Ryan</h1>");
  response->print("<h3>I'm working just fine.</h3>");
  response->print("<h3>Starship Down Data Disk Puzzle V2.</h3>");
  response->print("<h3>/completed - when prop is triggered.</h3>");
  response->print("<h3>/reset - runs ESP.restart();</h3>");
  response->print("</body>");
  response->print("</html>");
}

void attemptWifiConnection() {
    // Configures static IP address
  if (!WiFi.config(local_IP, gateway, subnet, primaryDNS, secondaryDNS)) {
    Serial.println("STA Failed to configure, DHCP error! Using random assigned IP!");
    Serial.println("Please update router settings to allow DHCP and static IP.");
  }

  // Connect to Wi-Fi network with SSID and password
  Serial.print("\n\n\nConnecting to ");
  Serial.println(ssid);
  WiFi.mode(WIFI_STA);
  //esp_wifi_set_ps(WIFI_PS_NONE);
  esp_sleep_pd_config(ESP_PD_DOMAIN_MAX,ESP_PD_OPTION_OFF);
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

void runWifiLoop() {
  if (WiFi.status() == WL_CONNECTED) {
    //listenForWifiClients();
  } else {
    attemptWifiConnection();
  }
}


