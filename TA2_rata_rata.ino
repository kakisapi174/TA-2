#include <AccelStepper.h>

AccelStepper stepper(AccelStepper::FULL4WIRE, 8, 10, 9, 11);

float smoothed = 0;
int baseline = 512;            // Adjust per your SpikerBox signal
int activationLevel = 500;      // Sensitivity (lower = easier to activate)

void setup() {
  Serial.begin(9600);
  stepper.setMaxSpeed(2000);
  stepper.setAcceleration(800);
}

void loop() {
  // ---- RAW EMG READ ----
  int raw = analogRead(A0);

  // ---- FULL-WAVE RECTIFICATION ----
  int rectified = abs(raw - baseline);

  // ---- SMOOTHING / ENVELOPE ----
  smoothed = (0.90 * smoothed) + (0.10 * rectified);

  // ---- SEND TO SERIAL PLOTTER ----
  Serial.print(raw);                  //Value 1
  Serial.print(" ");
  Serial.print(smoothed);             //Value 2
  Serial.print(" ");
  Serial.println(activationLevel);    //Value 3

  // ---- MOTOR CONTROL ----
  if (smoothed > activationLevel) {
    stepper.setSpeed(1200);
  } else {
    stepper.setSpeed(0);
  }

  stepper.runSpeed();
}
