#include <AccelStepper.h>

AccelStepper stepper(AccelStepper::FULL4WIRE, 8, 10, 9, 11);

int baseline = 512;
int activationLevel = 150;
int maxEMG = 300;

// Envelope smoothing (EMA)
float smoothAvg = 0;
float alpha = 0.05;

// Norm smoothing (anti-jerk)
float smoothNorm = 0;
float beta = 0.02;

// Motor parameters
int maxSpeed = 1800;

void setup() {
  Serial.begin(9600);
  stepper.setMaxSpeed(2000);
  stepper.setAcceleration(1000);
}

void loop() {

  int raw = analogRead(A0);
  int rectified = abs(raw - baseline);

  // Smooth amplitude
  smoothAvg = (1 - alpha) * smoothAvg + alpha * rectified;
  float avg = smoothAvg;

  // Auto-scale maxEMG
  if (avg > maxEMG) maxEMG = avg;

  int motorSpeed = 0;

  if (avg > activationLevel) {

    float norm = (avg - activationLevel) / float(maxEMG - activationLevel);
    norm = constrain(norm, 0.0, 1.0);

    // Smooth norm to prevent sudden jumps
    smoothNorm = (1 - beta) * smoothNorm + beta * norm;

    // Sigmoid for smooth acceleration
    float k = 6.0;
    float s = 1.0 / (1.0 + exp(-k * (smoothNorm - 0.5)));

    motorSpeed = s * maxSpeed;

  } else {
    motorSpeed = 0;
    smoothNorm = 0;
  }

  stepper.setSpeed(motorSpeed);
  stepper.runSpeed();

  Serial.print(avg);
  Serial.print(" ");
  Serial.print(smoothNorm);
  Serial.print(" ");
  Serial.println(motorSpeed);
}
