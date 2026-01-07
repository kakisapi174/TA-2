#include <AccelStepper.h>

AccelStepper stepper(AccelStepper::FULL4WIRE, 8, 9, 10, 11);

// =====================
// EMG PARAMETERS
// =====================
int baseline = 512;
float envelope = 0;

// Noise & smoothing
int noiseFloor = 35;     // potong noise kecil
float alpha = 0.03;     // envelope LPF (semakin kecil semakin halus)

// Scaling
int activationLevel = 143;   // mulai motor bergerak
int maxEMG = 220;           // kontraksi kuat (FIXED)

// Motor
int maxSpeed = 1800;

void setup() {
  Serial.begin(9600);
  stepper.setMaxSpeed(2000);
  stepper.setAcceleration(1000);
}

void loop() {

  int raw = analogRead(A0);
  int rectified = abs(raw - baseline);

  // =====================
  // NOISE GATE
  // =====================
  if (rectified < noiseFloor)
    rectified = 0;
  else
    rectified -= noiseFloor;

  // =====================
  // SMOOTH ENVELOPE
  // =====================
  envelope += alpha * (rectified - envelope);

  // =====================
  // SPEED MAPPING
  // =====================
  float norm = (envelope - activationLevel) / float(maxEMG - activationLevel);
  norm = constrain(norm, 0, 1);

  int motorSpeed = norm * maxSpeed;

  stepper.setSpeed(motorSpeed);
  stepper.runSpeed();

  // =====================
  // SERIAL PLOTTER
  // raw | envelope | speed
  // =====================
  Serial.print(raw);
  Serial.print(" ");
  Serial.print(envelope);
  Serial.print(" ");
  Serial.println(motorSpeed);
}
