// INCLUDES
// Library for addressable LED strips. Download from https://github.com/FastLED/FastLED/releases
#include <FastLED.h>

// DEFINES
// The total number of LEDs across all track segments
#define NUM_LEDS 129
// The greatest number of LEDs on any given track down which a particle can travel
#define MAX_NUM_LEDS_PER_TRACK 130
 
// STRUCTS
// We'll define a structure to keep all the related properties of an LED particle together
struct Particle {
  // What "type" of particle is this? Determines its colour, length, and the track it is meant to end on
  byte type;
  // Note that "position" doesn't correspond to a particular LED - rather, it represents the "distance travelled" by this particle along the track it is on.
  // The advantage of abstracting this from the LED array itself is that it allows us to jump around the set of LEDs, and also allows us to do sub-pixel positioning
  // So "distance travelled" is actually 16x the distance of one LED.
  unsigned int position;
  // How fast does it advance? Must be between 1-16
  int speed;
  // How long is this particle
  unsigned int length;
  // What track is the particle currently travelling down?
  int track;
  // Is this particle currently active?
  bool alive;
};

// CONSTANTS
// How many switch points are there in the track
const byte numSwitchPins = 5;
// The GPIO pins to which switches are attached
const byte switchPins[numSwitchPins] = { 6, 5, 4, 3, 2 };
// The maximum number of particles that will be alive at any one time
const byte maxParticles = 2;
// Number of milliseconds between each particle being spawned
const int _rate = 8500;
// How many different types of particle are there?
const byte numParticleTypes = 5;
// Define a colour associated with each type of particle
const CRGB colours[numParticleTypes] = { CRGB::Green, CRGB::White, CRGB::Red, CRGB(200,255,0), CRGB::Blue };
// Specify the order in which LEDs are traversed down each possible track from start to finish. -1 indicates beyond the end of the track

const unsigned int ledTrack[numParticleTypes][MAX_NUM_LEDS_PER_TRACK] = {
  
  // LHS
  {0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
  {0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 77, 78, 79, 80, 81, 82, 83, 84, 85, 86, 87, 88, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
  {0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 77, 78, 79, 80, 81, 82, 83, 84, 85, 89, 90, 91, 92, 93, 94, 95, 96, 97, 98, 99, 100, 101, 102, -1, -1, -1, -1, -1, -1,-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
  {0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 77, 78, 79, 80, 81, 82, 83, 84, 85, 89, 90, 91, 92, 93, 94, 95, 96, 97, 98, 99, 103, 104, 105, 106, 107, 108, 109, 110, 111, 112, 113, 114, 115, -1 , -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
  {0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 77, 78, 79, 80, 81, 82, 83, 84, 85, 89, 90, 91, 92, 93, 94, 95, 96, 97, 98, 99, 103, 104, 105, 106, 107, 108, 109, 110, 111, 112, 116, 117, 118, 119, 120, 121, 122, 123, 124, 125, 126, 127, 128, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1}
  
  // RHS
  
};

// Specify which LEDs should be used as a score counter for each track
const unsigned int scoreLEDs[numParticleTypes][3] = {
  {74, 75, 76},
  {86, 87, 88}, 
  {100, 101, 102},
  {113, 114, 115},
  {126, 127, 128}
};

// GLOBALS
// How many of the correct-coloured particle are currently sitting at the bottom of each track?
int score[numParticleTypes] = {0, 0, 0, 0, 0};
// The array of LEDs
CRGB leds[NUM_LEDS];
// The time at which a particle was last created
unsigned long _lastSpawned = 0;
// Rather than create a new object each time we spawn a particle, we'll create a fixed "pool" of particles
// and re-use them
Particle particlePool[maxParticles];

// Initial setup
void setup() {
  // Start a serial connection
  Serial.begin(9600);
  pinMode(7, INPUT_PULLUP);

  // Initialise the LED strip, specifying the type of LEDs and the pin connected to the data line
  FastLED.addLeds<WS2812, A0, GRB>(leds, NUM_LEDS);

  // Instantiate a reusable pool of particles
  for(int i=0; i<maxParticles; i++) {
    // Type, position, speed, length, track, alive all set to default values
    particlePool[i] = Particle{0, 0, 0, 0, 0, false};
  }

  // Initialise all the input pins
  for(int i=0; i<numSwitchPins; i++){
    pinMode(switchPins[i], INPUT_PULLUP);
  }

  // Set a random seed by reading the input value of an (unconnected) analog pin
  randomSeed(analogRead(A5));
}

// This function sets the correct LEDs for a particular particle on the track
int setLEDs(Particle p){
  // Convert the colour into a HSV value so that it can be dimmed without changing hue
  CHSV temp = rgb2hsv_approximate(colours[p.type]);
 
  // Extract the 'fractional' part of the position - that is, what proportion are we into the front-most pixel (in 1/16ths)
  uint8_t frac = p.position % 16;
  // Starting at the front and working backwards, work out the brightness of each of the "n" pixels that the particle occupies
  for(int n=0; n<=p.length && n<=p.position/16; n++) {
    if(n==0) {
      // first pixel in the bar
      temp.v = (frac * 16);
    } else if( n == p.length ) {
      // last pixel in the bar
      temp.v = (255 - (frac * 16));
    } else {
      // middle pixels
      temp.v = (255);
    }
    // Two things to watch out for = first that we don't try to go negative and look up an element from the index that doesn't exist
    // And secondly whether we get to the end of a segment and get a -1 value, which *does* exist in the array but isn't actually an LED.
    unsigned int pos = constrain(p.position/16-n, 0, MAX_NUM_LEDS_PER_TRACK*16-1);
    unsigned int LEDtoLight = ledTrack[p.track][pos];
    if(LEDtoLight != -1) {
      // Note that we don't assign a value, but add to it to allow for cases where two particles might overlap etc.
      leds[LEDtoLight] += temp;
    }
  }
}

int randomLessAvailable(){ // returns score that is the lowest
  int i;
  int lowest = 100;
  int lowestindex;
  int randNumber;
  for (i = 0; i < 5; ++i)
  {
    if (score[i] < lowest) { // if truly the lowest, set as lowest
      lowestindex = i;
      lowest = score[i];
    }
    if (score[i] == lowest) { // if tied with lowest, chance it
      randNumber = random(10);
      if (randNumber <= 5) {
        lowestindex = i;
        lowest = score[i];
      }
    }
  }
  return lowestindex;
}

// Spawn a new particle
void spawnParticle(){
  // Loop over every element in the particle pool
  for(int i=0; i<maxParticles; i++){
    // If this particle is not currently alive
    if(!particlePool[i].alive){
      // Reset it as a new particle
      particlePool[i].position = 0;
      particlePool[i].speed = 5;
      particlePool[i].type = randomLessAvailable();
      particlePool[i].length = particlePool[i].type + 1;
      particlePool[i].track = 0;
      particlePool[i].alive = true;
      return;
    }
  }
}

// This function is triggered when every track has a maximum score
// Use it to activate a relay/release a maglock etc.
void onSolve() 
{
  while(true) 
  {
  for(int i=0; i<NUM_LEDS; i++)
  leds[i] = CRGB::Black;
  FastLED.show();
  delay(2000);
//4
  for(int i=0; i<NUM_LEDS; i++)
  leds[i] = CRGB::Red;
  FastLED.show();
  delay(500);
  for(int i=0; i<NUM_LEDS; i++)
  leds[i] = CRGB::Black;
  FastLED.show();
  delay(500);
  for(int i=0; i<NUM_LEDS; i++)
  leds[i] = CRGB::Red;
  FastLED.show();
  delay(500);
  for(int i=0; i<NUM_LEDS; i++)
  leds[i] = CRGB::Black;
  FastLED.show();
  delay(500);
  for(int i=0; i<NUM_LEDS; i++)
  leds[i] = CRGB::Red;
  FastLED.show();
  delay(500);
  for(int i=0; i<NUM_LEDS; i++)
  leds[i] = CRGB::Black;
  FastLED.show();
  delay(500);
  for(int i=0; i<NUM_LEDS; i++)
  leds[i] = CRGB::Red;
  FastLED.show();
  delay(500);
  for(int i=0; i<NUM_LEDS; i++)
  leds[i] = CRGB::Black;
  FastLED.show();
  delay(3000);
//9
  for(int i=0; i<NUM_LEDS; i++)
  leds[i] = CRGB::Red;
  FastLED.show();
  delay(500);
  for(int i=0; i<NUM_LEDS; i++)
  leds[i] = CRGB::Black;
  FastLED.show();
  delay(500);
  for(int i=0; i<NUM_LEDS; i++)
  leds[i] = CRGB::Red;
  FastLED.show();
  delay(500);
  for(int i=0; i<NUM_LEDS; i++)
  leds[i] = CRGB::Black;
  FastLED.show();
  delay(500);
  for(int i=0; i<NUM_LEDS; i++)
  leds[i] = CRGB::Red;
  FastLED.show();
  delay(500);
  for(int i=0; i<NUM_LEDS; i++)
  leds[i] = CRGB::Black;
  FastLED.show();
  delay(500);
  for(int i=0; i<NUM_LEDS; i++)
  leds[i] = CRGB::Red;
  FastLED.show();
  delay(500);
  for(int i=0; i<NUM_LEDS; i++)
  leds[i] = CRGB::Black;
  FastLED.show();
  delay(500);
  for(int i=0; i<NUM_LEDS; i++)
  leds[i] = CRGB::Red;
  FastLED.show();
  delay(500);
  for(int i=0; i<NUM_LEDS; i++)
  leds[i] = CRGB::Black;
  FastLED.show();
  delay(500);
  for(int i=0; i<NUM_LEDS; i++)
  leds[i] = CRGB::Red;
  FastLED.show();
  delay(500);
  for(int i=0; i<NUM_LEDS; i++)
  leds[i] = CRGB::Black;
  FastLED.show();
  delay(500);
  for(int i=0; i<NUM_LEDS; i++)
  leds[i] = CRGB::Red;
  FastLED.show();
  delay(500);
  for(int i=0; i<NUM_LEDS; i++)
  leds[i] = CRGB::Black;
  FastLED.show();
  delay(500);
  for(int i=0; i<NUM_LEDS; i++)
  leds[i] = CRGB::Red;
  FastLED.show();
  delay(500);
  for(int i=0; i<NUM_LEDS; i++)
  leds[i] = CRGB::Black;
  FastLED.show();
  delay(500);
  for(int i=0; i<NUM_LEDS; i++)
  leds[i] = CRGB::Red;
  FastLED.show();
  delay(500);
  for(int i=0; i<NUM_LEDS; i++)
  leds[i] = CRGB::Black;
  FastLED.show();
  delay(2000);
//1
  for(int i=0; i<NUM_LEDS; i++)
  leds[i] = CRGB::Red;
  FastLED.show();
  delay(500);
  for(int i=0; i<NUM_LEDS; i++)
  leds[i] = CRGB::Black;
  FastLED.show();
  delay(3000);
//2
  for(int i=0; i<NUM_LEDS; i++)
  leds[i] = CRGB::Red;
  FastLED.show();
  delay(500);
  for(int i=0; i<NUM_LEDS; i++)
  leds[i] = CRGB::Black;
  FastLED.show();
  delay(500);
  for(int i=0; i<NUM_LEDS; i++)
  leds[i] = CRGB::Red;
  FastLED.show();
  delay(500);
  for(int i=0; i<NUM_LEDS; i++)
  leds[i] = CRGB::Black;
  FastLED.show();
  delay(7000);
  }
  }

void loop() {

  int forceSolveVal = digitalRead(7);
  if(forceSolveVal == LOW)
  {
    onSolve();
  }

  // For debug use only, print out the state of each toggle switch
  for(int i=0; i<numSwitchPins; i++){
    //Serial.print(digitalRead(switchPins[i]));
    //if(i<numSwitchPins-1) { Serial.print(","); }
  }
  //Serial.println("");
 
  // Clear the LEDs
  FastLED.clear();

  // Get the current elapsed time
  unsigned long currentTime = millis();

  // Has it been too long since the last time we spawned a particle?
  if(currentTime > _lastSpawned + _rate){
    Serial.println("Time to spawn!");
    spawnParticle();
    _lastSpawned = currentTime;
  }

  // Loop through all the particles
  for(int i = 0; i<maxParticles; i++){
   
    // Only if it's alive!
    if(particlePool[i].alive){

      // If the particle is on one of the switch points, change track as appropriate
      // If the particle is on one of the switch points, change track as appropriate
      if(particlePool[i].position/16 == 70) {
        particlePool[i].track = (digitalRead(switchPins[0])) ? : 0;              //0 : 3;
      }
      // If we took a left at the first junction then we come across the next switch after 13 LEDs
      if(particlePool[i].position/16 == 70 && particlePool[i].track == 1) {
        particlePool[i].track = (digitalRead(switchPins[1])) ? : 1;              //0 : 1;
      }
      // If we took a right at the first junction then we come aross the next switch after 14 LEDs
      if(particlePool[i].position/16 == 70 && particlePool[i].track == 1) {
        particlePool[i].track = (digitalRead(switchPins[2])) ? : 2;              //3 : 4;
      }
      // If we went left, then right, there's a final junction after 24 LEDs
      if(particlePool[i].position/16 == 70 && particlePool[i].track == 1) {
        particlePool[i].track = (digitalRead(switchPins[3])) ? : 3;              //1 : 2;
      }
       // If we went left, then right, there's a final junction after 24 LEDs
      if(particlePool[i].position/16 == 70 && particlePool[i].track == 1) {
        particlePool[i].track = (digitalRead(switchPins[4])) ? : 4;              //1 : 2;
      }

      if (particlePool[i].position/16 == 70) {
        // if none flipped then track is nearest
        for (int d = 0; d < 5; ++d) {
          if (digitalRead(switchPins[d]) == 0) {
            break;          
          }
          if (d == 4) {
            particlePool[i].track = 0;
            for (int e = 0; e < 5; ++e)
            {
              if (score[e] >= 1) { // if truly the lowest, set as lowest
                particlePool[i].track = e;
                break;
              }
            }
          }
        }
      }
      

      // Move along the current track
      particlePool[i].position += particlePool[i].speed;

      // If the particle has reached the last LED of its track
      if(ledTrack[particlePool[i].track][particlePool[i].position/16] == -1 || particlePool[i].position > NUM_LEDS*16) {
        // Kill it
        particlePool[i].alive = false;

        // Did it end in the correct track?
        if(particlePool[i].type == particlePool[i].track && score[particlePool[i].track] < 3) {
          // Increase the score for this track
          score[particlePool[i].track]++;
        }
        // The particle ended in the wrong track
        else if(particlePool[i].type != particlePool[i].track && score[particlePool[i].track] > 0) {
          // Reduce score for this track
          score[particlePool[i].track]--;
        }
      }

      // Draw the particle (check if *still* alive after moving!)
      if(particlePool[i].alive) {
        setLEDs(particlePool[i]);
      }
    }
  }

  // Light up the final LEDs in each track corresponding to the corresponding "score"
  byte totalScore = 0;
  for(int i=0; i<5; i++){
    for(int j=0; j<score[i]; j++){
      leds[scoreLEDs[i][j]] += colours[i];
    }
    totalScore += score[i];
  }
  if(totalScore == 15) {
    onSolve();
  }
 
  // Update the LED display with the new data
  FastLED.show();
  FastLED.delay(20);
}

