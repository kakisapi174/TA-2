#include <AccelStepper.h>

// =====================
// STEPPER
// =====================
AccelStepper stepper(AccelStepper::FULL4WIRE, 8, 9, 10, 11);

// =====================
// EMG CONFIG
// =====================
const int emgPin = A0;

// Windowing
const int samplesPerWindow = 50;   // ±20 ms @ ~2.5 kHz ADC
const int avgWindows = 5;           // smoothing ringan

int p2pBuffer[avgWindows];
int p2pIndex = 0;

// Threshold & scaling
int noiseFloor = 110;     // P2P saat rileks
int maxP2P = 250;        // P2P kontraksi kuat (sesuaikan)

// Motor
int maxSpeed = 1800;

void setup() {
  Serial.begin(9600);

  stepper.setMaxSpeed(2000);
  stepper.setAcceleration(1000);

  for (int i = 0; i < avgWindows; i++)
    p2pBuffer[i] = 0;
}

void loop() {

  // =====================
  // PEAK TO PEAK WINDOW
  // =====================
  int sigMax = 0;
  int sigMin = 1023;

  for (int i = 0; i < samplesPerWindow; i++) {
    int v = analogRead(emgPin);
    if (v > sigMax) sigMax = v;
    if (v < sigMin) sigMin = v;
  }

  int peakToPeak = sigMax - sigMin;

  // =====================
  // STORE FOR AVERAGING
  // =====================
  p2pBuffer[p2pIndex] = peakToPeak;
  p2pIndex = (p2pIndex + 1) % avgWindows;

  int p2pAvg = 0;
  for (int i = 0; i < avgWindows; i++)
    p2pAvg += p2pBuffer[i];
  p2pAvg /= avgWindows;

  // =====================
  // NOISE GATE
  // =====================
  if (p2pAvg < noiseFloor)
    p2pAvg = 0;
  else
    p2pAvg -= noiseFloor;

  // =====================
  // MOTOR MAPPING
  // =====================
  float norm = float(p2pAvg) / float(maxP2P);
  norm = constrain(norm, 0, 1);

  int motorSpeed = norm * maxSpeed;

  stepper.setSpeed(motorSpeed);
  stepper.runSpeed();

  // =====================
  // SERIAL MONITOR
  // raw P2P | avg P2P | speed
  // =====================
  Serial.print(peakToPeak);
  Serial.print(" ");
  Serial.print(p2pAvg);
  Serial.print(" ");
  Serial.println(motorSpeed);
}
