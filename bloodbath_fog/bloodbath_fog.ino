#include <Arduino.h>

#include "SoftwareSerial.h"
#include "DFRobotDFPlayerMini.h"

// Use pins 2 and 3 to communicate with DFPlayer Mini
static const uint8_t PIN_MP3_TX = 13; // Connects to module's RX
static const uint8_t PIN_MP3_RX = 14; // Connects to module's TX
SoftwareSerial softwareSerial(PIN_MP3_RX, PIN_MP3_TX);

// Create the Player object
DFRobotDFPlayerMini player;

const int button = 2;
const int relay = 4;
const int mag = 5;
 
void setup()
{
  Serial.begin(115200);
  pinMode(button, INPUT_PULLUP);
  pinMode(relay, OUTPUT);
  pinMode(mag, OUTPUT);
  digitalWrite(relay, LOW);
  digitalWrite(mag, LOW);
  delay(1000);

  softwareSerial.begin(9600);
  if (player.begin(softwareSerial))
  {
   Serial.println("Software Serial Player Began Successfull");
   player.volume(20);
   }
  else
  {
    Serial.println("Connecting to DFPlayer Mini failed!");
  }
  
  Serial.println("Setup Successful");

}

bool trig_check = false;
bool triggered = false;
float wait = 0;
void loop() {
  //Serial.println(digitalRead(button));
  int sensorVal = digitalRead(button);
  if (sensorVal == LOW && !triggered && !trig_check)
  {
    trig_check = true;
    player.volume(100);
    player.play(1);
    Serial.println("trigchec");
    wait = millis() + 5000;
  }
  if (sensorVal == LOW && !triggered && trig_check && wait < millis()) // trigger must be checked pushed for 5 seconds
  {
    triggered = true;
    trig_check = false;
    player.volume(100);
    player.play(2);
    digitalWrite(relay, HIGH);
    digitalWrite(mag, HIGH);
    delay(10000);
    Serial.println("trigger");
  }
  Serial.println(sensorVal);
  if (sensorVal == HIGH && trig_check) // PAUSE PLAYER, UNPRESSED WHEN PLAYING COUNTDOWN
  {
    Serial.println("pause");
    player.pause();
    trig_check = false;
  }
  if (sensorVal == HIGH && triggered) // UNPRESSED AFTER TRIGGERED
  {
    triggered = false;
    digitalWrite(relay, LOW);
    digitalWrite(mag, LOW);
    Serial.println("untrigger");
  }
  delay(100);
}
