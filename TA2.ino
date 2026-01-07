#include <Stepper.h>

const int stepsPerRevolution = 2048; // 28BYJ-48 = 2048 steps per revolution

Stepper myStepper(stepsPerRevolution, 8, 10, 9, 11);
// Note: Order of pins (8,10,9,11) matches the ULN2003 sequence

void setup() {
  myStepper.setSpeed(10); // speed in RPM
  Serial.begin(9600);
}

void loop() {
  Serial.println("Clockwise");
  myStepper.step(stepsPerRevolution);
  delay(1000);

  Serial.println("Counterclockwise");
  myStepper.step(-stepsPerRevolution);
  delay(1000);
}
