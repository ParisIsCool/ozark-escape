#include <FastLED.h>

#define NUM_LEDS 5 // Change this to the number of LEDs you have
#define LED_PIN 2   // Define the Arduino pin connected to the data input of the LED strip
#define DELAY_INTERVAL 25 // Delay interval in milliseconds

CRGB leds[NUM_LEDS];
uint32_t lastUpdateTime = 0;
uint32_t ledUpdateIntervals[NUM_LEDS];

void setup() {
  FastLED.addLeds<WS2812B, LED_PIN, RGB>(leds, NUM_LEDS);
  FastLED.setBrightness(100); // Adjust brightness if needed
  randomSeed(analogRead(0)); // Seed the random number generator
  
  // Initialize delay intervals for each LED
  for (int i = 0; i < NUM_LEDS; i++) {
    ledUpdateIntervals[i] = random(100, 1000);
  }
}

void loop() {
  uint32_t currentTime = millis();
  
  if (currentTime - lastUpdateTime >= DELAY_INTERVAL) {
    lastUpdateTime = currentTime;
    
    for (int i = 0; i < NUM_LEDS; i++) {
      if (ledUpdateIntervals[i] <= millis()) {
        // Reset the delay for this LED
        ledUpdateIntervals[i] = currentTime + random(50, 500);
        int randomColor = random(3); // Generate a random number between 0 and 2
        switch (randomColor) {
          case 0:
            leds[i] = CRGB::Green; // Set LED to green
            break;
          case 1:
            leds[i] = CRGB::Blue; // Set LED to blue
            break;
          case 2:
            leds[i] = CRGB::Yellow; // Set LED to yellow
            break;
        }
      }
    }
    
    FastLED.show();
  }
}
