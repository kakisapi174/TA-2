#include <AccelStepper.h>

// Final wiring based on your test
AccelStepper stepper(AccelStepper::FULL4WIRE, 8, 9, 10, 11);

// Baseline EMG midpoint
int baseline = 512;

// Envelopes
float envA = 0;
float envB = 0;

// Envelope dynamics
float attack  = 0.45;   // naik cepat
float release = 0.08;   // turun lambat

// Motor parameters
int maxSpeed = 1800;
float gain = 6.0;       // tingkatkan jika motor kurang kuat

void setup() {
  Serial.begin(9600);

  stepper.setMaxSpeed(2000);
  stepper.setAcceleration(1000);
}

void loop() {
  // ==========================
  // READ 2 EMG CHANNELS
  // ==========================
  int rawA = analogRead(A0);    // Person A
  int rawB = analogRead(A1);    // Person B

  // Rectification
  int rectA = abs(rawA - baseline);
  int rectB = abs(rawB - baseline);

  // ==========================
  // ENVELOPE FOLLOWER
  // ==========================
  if (rectA > envA) envA += attack * (rectA - envA);
  else              envA += release * (rectA - envA);

  if (rectB > envB) envB += attack * (rectB - envB);
  else              envB += release * (rectB - envB);

  // ==========================
  // DIFFERENCE LOGIC
  // ==========================
  float diff = envA - envB;   // positif = A lebih kuat, negatif = B lebih kuat

  // Speed magnitude
  float motorSpeed = abs(diff) * gain;
  if (motorSpeed > maxSpeed) motorSpeed = maxSpeed;

  // ==========================
  // MOTOR CONTROL
  // ==========================
  if (diff > 2) {
    stepper.setSpeed(+motorSpeed);   // CCW, A menang
  } 
  else if (diff < -2) {
    stepper.setSpeed(-motorSpeed);   // CW, B menang
  } 
  else {
    stepper.setSpeed(0);             // seimbang
  }

  stepper.runSpeed();

  // ==========================
  // SERIAL PLOTTER OUTPUT
  // Format: envA  envB  diff
  // ==========================
  Serial.print(envA);
  Serial.print(" ");
  Serial.print(envB);
  Serial.print(" ");
  Serial.println(diff);
}
