#include <Servo.h>

Servo servo1;
Servo servo2;

#define PWMA 5
#define AIN1 7
#define AIN2 8

#define PWMB 6
#define BIN1 9
#define BIN2 10

#define SERVO1_PIN 11
#define SERVO2_PIN 12

#define LASER_PIN 2

#define TRIG_PIN 3
#define ECHO_PIN 4

#define JOY_X A0
#define JOY_Y A1
#define JOY_BUTTON A2

#define STEP_PIN A5
#define DIR_PIN A3
#define EN_PIN A4

int buttonPressCount = 0;
bool lastButtonState = HIGH;

int servo2StartPosition = 90;
int servo2Pos = servo2StartPosition;

unsigned long lastServoMove = 0;

const int servoMoveInterval = 50;

float ultraToBarrel = 22.589;
float barrelToLaser = 12.0;
float v = 16.92;

unsigned long lastStepTime = 0;


void setup() {

  Serial.begin(9600);

  pinMode(PWMA, OUTPUT);
  pinMode(AIN1, OUTPUT);
  pinMode(AIN2, OUTPUT);

  pinMode(PWMB, OUTPUT);
  pinMode(BIN1, OUTPUT);
  pinMode(BIN2, OUTPUT);

  servo1.attach(SERVO1_PIN);
  servo2.attach(SERVO2_PIN);

  servo1.write(0);

  servo2Pos = servo2StartPosition;
  servo2.write(servo2Pos);

  pinMode(LASER_PIN, OUTPUT);
  digitalWrite(LASER_PIN, HIGH);

  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  pinMode(JOY_BUTTON, INPUT_PULLUP);

  pinMode(STEP_PIN, OUTPUT);
  pinMode(DIR_PIN, OUTPUT);
  pinMode(EN_PIN, OUTPUT);

  digitalWrite(EN_PIN, LOW);

  digitalWrite(AIN1, HIGH);
  digitalWrite(AIN2, LOW);

  digitalWrite(BIN1, HIGH);
  digitalWrite(BIN2, LOW);

  stopMotors();

  Serial.println("==============================");
  Serial.println("SYSTEM READY");

  Serial.println("Servo 1: 0 degrees");

  Serial.print("Servo 2: ");
  Serial.print(servo2StartPosition);
  Serial.println(" degrees");

  Serial.println("Type 'target' to calculate target");
  Serial.println("==============================");
}


void loop() {

  int joystickX = analogRead(JOY_X);

  const int servoDeadZone = 120;

  if (millis() - lastServoMove >= servoMoveInterval) {

    if (joystickX > 512 + servoDeadZone) {

      servo2Pos++;

      if (servo2Pos > 180) {
        servo2Pos = 180;
      }

      servo2.write(servo2Pos);

      lastServoMove = millis();
    }

    else if (joystickX < 512 - servoDeadZone) {

      servo2Pos--;

      if (servo2Pos < 0) {
        servo2Pos = 0;
      }

      servo2.write(servo2Pos);

      lastServoMove = millis();
    }
  }

  int joystickY = analogRead(JOY_Y);

  const int stepperDeadZone = 100;

  if (joystickY > 512 + stepperDeadZone) {

    digitalWrite(DIR_PIN, HIGH);

    stepStepper(joystickY);
  }

  else if (joystickY < 512 - stepperDeadZone) {

    digitalWrite(DIR_PIN, LOW);

    stepStepper(joystickY);
  }

  bool buttonState = digitalRead(JOY_BUTTON);

  if (lastButtonState == HIGH && buttonState == LOW) {

    buttonPressCount++;

    Serial.print("BUTTON PRESS: ");
    Serial.println(buttonPressCount);

    if (buttonPressCount == 1) {

      Serial.println("MOTORS ON");

      startMotors();
    }

    else if (buttonPressCount == 2) {

      Serial.println("SERVO 1 ACTION");

      servo1Action();
    }

    else if (buttonPressCount == 3) {

      Serial.println("MOTORS OFF");

      stopMotors();

      buttonPressCount = 0;
    }

    delay(200);
  }

  lastButtonState = buttonState;

if (Serial.available() > 0) {

  String input = Serial.readStringUntil('\n');
  input.trim();

  if (input == "target") {

    Serial.println("TARGET COMMAND RECEIVED");

    float distance = measureDistance();

    if (distance > 0) {
      target(distance);
    } else {
      Serial.println("Ultrasonic measurement failed");
    }

  }
  else {

    int newAngle = input.toInt();

    if (newAngle >= 0 && newAngle <= 180) {

      servo2Pos = newAngle;
      servo2.write(servo2Pos);

      Serial.print("Servo 2 set to: ");
      Serial.println(servo2Pos);

    } else {

      Serial.println("Enter an angle from 0 to 180");
    }
  }
}

  delay(5);
}


void stepStepper(int joystickValue) {

  int distanceFromCentre =
    abs(joystickValue - 512);

  int stepDelay = map(
    distanceFromCentre,
    100,
    511,
    5000,
    800
  );

  unsigned long currentTime = micros();

  if (currentTime - lastStepTime >= stepDelay) {

    digitalWrite(STEP_PIN, HIGH);

    delayMicroseconds(2);

    digitalWrite(STEP_PIN, LOW);

    lastStepTime = currentTime;
  }
}


void startMotors() {

  digitalWrite(AIN1, HIGH);
  digitalWrite(AIN2, LOW);

  digitalWrite(BIN1, HIGH);
  digitalWrite(BIN2, LOW);

  analogWrite(PWMA, 255);
  analogWrite(PWMB, 255);

  Serial.println("Motors running");
}


void stopMotors() {

  analogWrite(PWMA, 0);
  analogWrite(PWMB, 0);

  Serial.println("Motors stopped");
}


void servo1Action() {

  Serial.println("Servo 1 moving");

  servo1.write(0);

  delay(200);

  for (int pos = 0; pos <= 180; pos++) {

    servo1.write(pos);

    delay(3);
  }

  for (int pos = 180; pos >= 0; pos--) {

    servo1.write(pos);

    delay(1);
  }

  Serial.println("Servo 1 complete");
}


float measureDistance() {

  digitalWrite(TRIG_PIN, LOW);

  delayMicroseconds(2);

  digitalWrite(TRIG_PIN, HIGH);

  delayMicroseconds(10);

  digitalWrite(TRIG_PIN, LOW);

  long duration = pulseIn(
    ECHO_PIN,
    HIGH,
    30000
  );

  if (duration == 0) {

    Serial.println("No ultrasonic echo");

    return -1;
  }

  float distanceMM =
    duration * 0.343 / 2.0;

  Serial.print("Distance: ");
  Serial.print(distanceMM);
  Serial.println(" mm");

  return distanceMM;
}


void target(float distanceMM) {

  float physicsAngleDegrees = servo2Pos - 90.0;

  float theta =
    physicsAngleDegrees
    * PI / 180.0;

  float distanceViaLaser =
    distanceMM
    - (ultraToBarrel + barrelToLaser)
    * tan(theta);

  float distanceViaBarrel =
    distanceViaLaser
    + barrelToLaser
    * tan(theta);

  float offsetAngle =
    atan(
      barrelToLaser / distanceViaLaser
    )
    * 180.0 / PI;

  float sHorz =
    distanceViaLaser
    * cos(theta);

  float sVert =
    distanceViaLaser
    * sin(theta);

  sHorz = sHorz / 1000.0;
  sVert = sVert / 1000.0;

  Serial.println();
  Serial.println("----- data ----");

  Serial.print("Ultrasonic distance: ");
  Serial.print(distanceMM);
  Serial.println(" mm");

  Serial.print("Servo angle: ");
  Serial.print(servo2Pos);
  Serial.println(" degrees");

  Serial.print("Physics angle: ");
  Serial.print(physicsAngleDegrees);
  Serial.println(" degrees");

  Serial.print("Horizontal distance: ");
  Serial.print(sHorz);
  Serial.println(" m");

  Serial.print("Vertical distance: ");
  Serial.print(sVert);
  Serial.println(" m");

  Serial.print("Offset angle: ");
  Serial.print(offsetAngle);
  Serial.println(" degrees");

  float A =
    4.905
    * sHorz
    * sHorz
    / (v * v);

  float discriminant =
    sHorz * sHorz
    - 4 * A * (A + sVert);

  if (discriminant < 0) {

    Serial.println();
    Serial.println("NO REAL ANGLES");

    return;
  }

  float x1 =
    (sHorz - sqrt(discriminant))
    / (2 * A);

  float x2 =
    (sHorz + sqrt(discriminant))
    / (2 * A);

  float angle1 =
    atan(x1)
    * 180.0 / PI;

  float angle2 =
    atan(x2)
    * 180.0 / PI;

  Serial.println();
  Serial.println("--- ANGLES --");

  Serial.print("Angle 1: ");
  Serial.print(angle1);
  Serial.println(" degrees");

  Serial.print("Angle 2: ");
  Serial.print(angle2);
  Serial.println(" degrees");

  float servoTargetAngle = 90.0 + angle1 - offsetAngle;

  Serial.print("Servo Target Angle: ");
  Serial.print(servoTargetAngle);
  Serial.println(" degrees");

  if (servoTargetAngle < 0) {
    servoTargetAngle = 0;
  }

  if (servoTargetAngle > 180) {
    servoTargetAngle = 180;
  }

  Serial.print("old pos: ");
  Serial.println(servo2Pos);

  Serial.print("target pos: ");
  Serial.println(servoTargetAngle);

  servo2Pos =
    (int)round(servoTargetAngle);

  Serial.print("moving to: ");
  Serial.println(servo2Pos);

  servo2.write(servo2Pos);

  delay(1000);

  Serial.print("moved to: ");
  Serial.print(servo2Pos);
  Serial.println(" degrees");

  Serial.println("------------------");
}
