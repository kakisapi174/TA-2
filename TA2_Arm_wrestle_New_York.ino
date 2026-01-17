#define SERIAL_PLOTTER 1  

#include <AccelStepper.h>

// =====================
// STEPPER
// =====================
AccelStepper stepper(AccelStepper::FULL4WIRE, 8, 9, 10, 11);

// =====================
// EMG CONFIG
// =====================
const int emgPin1 = A0;   // Player 1
const int emgPin2 = A1;   // Player 2

// Windowing
const int samplesPerWindow = 50;
const int avgWindows = 5;

// Buffer P2P
int p2pBuf1[avgWindows];
int p2pBuf2[avgWindows];
int idx = 0;

// Threshold & scaling
int noiseFloor = 20;
int maxP2P = 160;

// Deadzone agar tidak jitter
int diffDeadzone = 10;

// Motor
int maxSpeed = 1800;

void setup() {
  Serial.begin(9600);

  stepper.setMaxSpeed(2000);
  stepper.setAcceleration(1200);

  for (int i = 0; i < avgWindows; i++) {
    p2pBuf1[i] = 0;
    p2pBuf2[i] = 0;
  }
}

int computeP2P(int pin) {
  int sigMax = 0;
  int sigMin = 1023;

  for (int i = 0; i < samplesPerWindow; i++) {
    int v = analogRead(pin);
    if (v > sigMax) sigMax = v;
    if (v < sigMin) sigMin = v;
  }
  return sigMax - sigMin;
}

int averageBuffer(int *buf) {
  int sum = 0;
  for (int i = 0; i < avgWindows; i++)
    sum += buf[i];
  return sum / avgWindows;
}

void loop() {

  // =====================
  // PEAK TO PEAK
  // =====================
  int p2p1 = computeP2P(emgPin1);
  int p2p2 = computeP2P(emgPin2);

  // Store for averaging
  p2pBuf1[idx] = p2p1;
  p2pBuf2[idx] = p2p2;
  idx = (idx + 1) % avgWindows;

  int avg1 = averageBuffer(p2pBuf1);
  int avg2 = averageBuffer(p2pBuf2);

  // =====================
  // NOISE GATE
  // =====================
  avg1 = (avg1 < noiseFloor) ? 0 : avg1 - noiseFloor;
  avg2 = (avg2 < noiseFloor) ? 0 : avg2 - noiseFloor;

  // =====================
  // DIFFERENTIAL CONTROL
  // =====================
  int diff = avg1 - avg2;

  int motorSpeed = 0;

  if (abs(diff) > diffDeadzone) {
    float norm = float(abs(diff)) / float(maxP2P);
    norm = constrain(norm, 0, 1);

    motorSpeed = norm * maxSpeed;

    // Direction
    if (diff > 0)
      stepper.setSpeed(motorSpeed);     // Player 1 menang
    else
      stepper.setSpeed(-motorSpeed);    // Player 2 menang
  } else {
    stepper.setSpeed(0);  // imbang
  }

  stepper.runSpeed();

  // =====================
  // SERIAL DEBUG
  // =====================
#if SERIAL_PLOTTER
  Serial.print(avg1);
  Serial.print(" ");
  Serial.print(avg2);
  Serial.print(" ");
  Serial.println(diff);
#else
  Serial.print("P1: ");
  Serial.print(avg1);
  Serial.print(" | P2: ");
  Serial.print(avg2);
  Serial.print(" | Diff: ");
  Serial.print(diff);
  Serial.print(" | Speed: ");
  Serial.println(motorSpeed);
#endif

}
