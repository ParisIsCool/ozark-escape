#include <FastLED.h>

#define NUM_LEDS 43 // Change this to the number of LEDs you have
#define LED_PIN 2   // Define the Arduino pin connected to the data input of the LED strip
#define DELAY_INTERVAL 15 // Delay interval in milliseconds

CRGB leds[NUM_LEDS];
#define NUM_RANDOM 31
uint8_t RandomLeds[NUM_RANDOM] = {6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,33,34,35,36};
#define NUM_WHITE 3
uint8_t WhiteLeds[NUM_WHITE] = {0,36,42};
#define NUM_SCROLL1 5
uint8_t ScrollingLeds1[NUM_SCROLL1] = {1,2,3,4,5};
uint32_t lastScroll1 = 0;
uint32_t nextScroll1 = 0;
#define NUM_SCROLL2 5
uint8_t ScrollingLeds2[NUM_SCROLL2] = {37,38,39,40,41};
uint32_t lastScroll2 = 0;
uint32_t nextScroll2 = 0;
uint32_t lastUpdateTime = 0;
uint32_t ledUpdateIntervals[NUM_RANDOM];
unsigned long fadeStartTime = 0;
unsigned long fadeDuration = 1000; // Adjust the duration of the fade in milliseconds

void setup() {
  Serial.begin(9600);
  Serial.println("Booting");
  FastLED.addLeds<WS2812B, LED_PIN, RGB>(leds, NUM_LEDS);
  FastLED.setBrightness(150); // Adjust brightness if needed
  randomSeed(analogRead(0)); // Seed the random number generator
  
  // Initialize delay intervals for each LED
  for (int i = 0; i < NUM_LEDS; i++) {
    ledUpdateIntervals[i] = random(100, 1000);
  }
}

CRGB getLedColor(int ledIndex) {
  // Define the color for each LED index
  switch (ledIndex) {
    case 1: return CRGB::Red;
    case 2: return CRGB::Yellow;
    case 3: return CRGB::Green;
    case 4: return CRGB::White;
    case 5: return CRGB::Blue;
    default: return CRGB::Black;
  }
}

void fadeToColor(int ledIndex, CRGB targetColor) {
  static const int fadeSteps = 50; // Adjust the number of steps for a smoother fade
  CRGB currentColor = leds[ledIndex];
  unsigned long currentTime = millis();
  unsigned long elapsedTime = currentTime - fadeStartTime;

  if (elapsedTime < fadeDuration) {
    float blendFactor = float(elapsedTime) / float(fadeDuration);
    CRGB blendedColor = blend(currentColor, targetColor, blendFactor);
    leds[ledIndex] = blendedColor;
    FastLED.show();
  } else {
    leds[ledIndex] = targetColor;
  }
}

void loop() {
  uint32_t currentTime = millis();

  if (currentTime - lastUpdateTime >= DELAY_INTERVAL) {
    lastUpdateTime = currentTime;

    for (int i = 0; i < NUM_WHITE; i++) {
      leds[WhiteLeds[i]] = CRGB::White; // Set LED to white
    }

    if (lastScroll1 + 2000 < millis()) {
      lastScroll1 = millis();
      for (int i = 0; i < NUM_SCROLL1; i++) {
        leds[ScrollingLeds1[i]] = CRGB::Black; // Set LED to off
      }
      nextScroll1 = nextScroll1 + 1;
      if (nextScroll1 > NUM_SCROLL1) {
        nextScroll1 = 0;
      }
      leds[ScrollingLeds1[nextScroll1]] = CRGB::White; // Set LED to white
    }

    // OLD CODE
    /*if (lastScroll2 + 2000 < millis()) {
      lastScroll2 = millis();
      for (int i = 0; i < NUM_SCROLL2; i++) {
        leds[ScrollingLeds2[i]] = CRGB::Black; // Set LED to off
      }
      nextScroll2 = nextScroll2 + 1;
      if (nextScroll2 > NUM_SCROLL2) {
        nextScroll2 = 0;
      }
      leds[ScrollingLeds2[nextScroll2]] = CRGB::White; // Set LED to white
    }*/

    if (lastScroll2 + 2000 < millis()) {
      lastScroll2 = millis();
      for (int i = 0; i < NUM_SCROLL2; i++) {
        int currentLED = ScrollingLeds2[i];
        fadeToColor(currentLED, CRGB::Black); // Fade to black
      }
      nextScroll2 = (nextScroll2 + 1) % NUM_SCROLL2;
      int nextLED = ScrollingLeds2[nextScroll2];
      fadeStartTime = millis();
      fadeToColor(nextLED, getLedColor(nextLED)); // Fade to the respective color
    }
        
    for (int i = 0; i < NUM_RANDOM; i++) {
      if (ledUpdateIntervals[i] <= millis()) {
        // Reset the delay for this LED
        ledUpdateIntervals[i] = currentTime + random(50, 500);
        FastLED.show();
        
        int randomColor = random(3); // Generate a random number between 0 and 2
        switch (randomColor) {
          case 0:
            leds[RandomLeds[i]] = CRGB::Green; // Set LED to green
            break;
          case 1:
            leds[RandomLeds[i]] = CRGB::Blue; // Set LED to blue
            break;
          case 2:
            leds[RandomLeds[i]] = CRGB::Yellow; // Set LED to yellow
            break;
        }
      }
    }
  }
}
