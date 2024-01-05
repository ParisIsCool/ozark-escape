#define FASTLED_ALLOW_INTERRUPTS 0
#include <FastLED.h>
#define LED_PIN 11
#define NUM_LEDS 106
CRGB leds[NUM_LEDS];

int brightness = 255;
#define buzzer 13
#define light 35

#define startPin 19
#define failPin 47
#define endPin 45
#define sound 3
#define resetPin 8

// fix for freezing on core 1
void Task1code( void * parameter) {
  while(true) {
    FastLED.show();
    delay(16);
  }
}

enum GameState {FAILED, IN_PROGRESS, SUCCESS, STANDBY};
GameState gameState = GameState::FAILED;

void setup()
{

  FastLED.addLeds<WS2812, LED_PIN, GRB>(leds, NUM_LEDS);
  FastLED.setMaxPowerInVoltsAndMilliamps(5, 500);
  FastLED.clear();
  FastLED.show();

  xTaskCreatePinnedToCore(
                  Task1code,   /* Task function. */
                  "Task1code",     /* name of task. */
                  10000,       /* Stack size of task */
                  NULL,        /* parameter of the task */
                  1,           /* priority of the task */
                  NULL,      /* Task handle to keep track of created task */
                  1);          /* pin task to core 0 */   
  
  pinMode(startPin, INPUT_PULLUP);
  pinMode(failPin, INPUT_PULLUP);
  pinMode(endPin, INPUT_PULLUP);
  pinMode(resetPin, INPUT_PULLUP);
  pinMode(sound, OUTPUT);
  pinMode(light, OUTPUT);
  //digitalWrite(light, LOW);

  Serial.begin(9600);
}

#define ANIM_LENGTH 50
int moving_Start = 0;
int cheatHelp = 0;
void loop() {
  delay(50);
  if (digitalRead(resetPin) == LOW) {
    gameState = GameState::STANDBY;
    Serial.println("Manuel Resetting...");
    for(int i=0; i<NUM_LEDS; i++){
      leds[i] = CRGB(0,0,255);
    }
    tone(buzzer,100);
    delay(1000);
    noTone(buzzer);
    digitalWrite(light, LOW);
    digitalWrite(sound, LOW);
  }
  if (digitalRead(startPin) == LOW && (gameState == GameState::STANDBY || gameState == GameState::FAILED)) {
    cheatHelp = 500; // ms you get.
    gameState = GameState::IN_PROGRESS;
    Serial.println(" Let the game begin! ");
    for(int i=0; i<NUM_LEDS; i++) {
      leds[i] = CRGB(0,255,0);
    }
    tone(buzzer, 600);
    delay(1000);
    noTone(buzzer);
  }
  switch(gameState) {
    case GameState::STANDBY:
      for(int i=0; i<NUM_LEDS; i++) {
        leds[i] = CRGB(255,110,0);
      }
      break;
    case GameState::IN_PROGRESS:
      for(int i=0; i<NUM_LEDS; i++) {
        leds[i] = CRGB(0,255,0);
      }
      if (moving_Start > NUM_LEDS) {
        moving_Start = 0;
      }
      for(int i=0; i<ANIM_LENGTH; i++) {
        int index = i+moving_Start;
        if (index > NUM_LEDS) {
          index = index - NUM_LEDS;
        }
        leds[index] = CRGB(255,110,0);
      }
      moving_Start = moving_Start + 1;
      if (digitalRead(endPin) == LOW) {
        delay(300);
        gameState = GameState::SUCCESS;
        Serial.println("Success!");
        digitalWrite(sound, HIGH);
        // flash leds
        delay(300);
        digitalWrite(sound, LOW);
        delay(200);
        digitalWrite(light, HIGH);
        delay(1000);
        for(int i=0; i<5; i++) {
          for(int i=0; i<NUM_LEDS; i++) {
            leds[i] = CRGB(255,255,255);
          }
          delay(500);
          for(int i=0; i<NUM_LEDS; i++) {
            leds[i] = CRGB(0,0,0);
          }
          delay(500);
        }
        //blue
        for(int i=0; i<13; i++)
        leds[i] = CRGB(0,0,255);
        //yellow
        for(int i=13; i<26; i++)
        leds[i] = CRGB(255,110,0);
        //green
        for(int i=26; i<39; i++)
        leds[i] = CRGB(0,255,0);
        //blue
        for(int i=39; i<52; i++)
        leds[i] = CRGB(0,0,255);
        //red
        for(int i=52; i<65; i++)
        leds[i] = CRGB(255,0,0);
        //yellow
        for(int i=65; i<78; i++)
        leds[i] = CRGB(255,110,0);
        //red
        for(int i=78; i<91; i++)
        leds[i] = CRGB(255,0,0);
        //green
        for(int i=91; i<104; i++)
        leds[i] = CRGB(0,255,0);
        for(int i=104; i<106; i++)
        leds[i] = CRGB(0,0,0);
      } else if (digitalRead(failPin) == LOW) {
        cheatHelp = cheatHelp - 50;
        if (cheatHelp <= 0) {
          gameState = GameState::FAILED;
          for(int i=0; i<NUM_LEDS; i++){
            leds[i] = CRGB(255,0,0);
          }
          Serial.println("Fail! Go back to the beginning!");
          tone(buzzer,100);
          delay(1000);
          noTone(buzzer);
        }
      }
      break;
    case GameState::FAILED:
      break;
    case GameState::SUCCESS:
      break;
  }
}