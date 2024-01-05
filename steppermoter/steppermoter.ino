// Define stepper motor connections:
#define dirPin 4
#define stepPin 18
#define motorInterfaceType 1

// Include the AccelStepper library:
#include <AccelStepper.h>

AccelStepper stepper = AccelStepper(motorInterfaceType, stepPin, dirPin);

void setup() {
  // Set the maximum speed and acceleration:
  stepper.setMaxSpeed(1000);
  stepper.setAcceleration(500);
}

void loop() {
  // Set the target position:
  stepper.moveTo(8000);
  // Run to target position with set speed and acceleration/deceleration:
  stepper.runToPosition();

  delay(1000);

  // Move back to zero:
  stepper.moveTo(0);
  stepper.runToPosition();

  delay(1000);
}