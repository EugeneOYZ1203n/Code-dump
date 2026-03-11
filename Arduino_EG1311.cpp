#include <Servo.h>

int TRIG_PIN = 13;
int ECHO_PIN = 12;
int MOTOR1_PIN1 = 4;
int MOTOR1_PIN2 = 5;
int MOTOR2_PIN1 = 6;
int MOTOR2_PIN2 = 7;
int MOTOR3_PIN1 = 8;
int MOTOR3_PIN2 = 9;
int MOTOR4_PIN1 = 10;
int MOTOR4_PIN2 = 11;
int SERVO_PIN = 3;

const float SPEED_OF_SOUND = 0.0343;
const int threshold = 12;

const int SERVO_START = 10;
const int SERVO_THROW = 160;

const int REVERSE_TIME = 20000; // milliseconds

Servo servo;

enum RobotState {
  MOVING_FORWARD,
  THROWING_BALL,
  REVERSING,
  STOPPED
};

RobotState state = MOVING_FORWARD;
unsigned long reverseStartTime = 0;

void setup() {
  pinMode(MOTOR1_PIN1, OUTPUT);
  pinMode(MOTOR1_PIN2, OUTPUT);
  pinMode(MOTOR2_PIN1, OUTPUT);
  pinMode(MOTOR2_PIN2, OUTPUT);
  pinMode(MOTOR3_PIN1, OUTPUT);
  pinMode(MOTOR3_PIN2, OUTPUT);
  pinMode(MOTOR4_PIN1, OUTPUT);
  pinMode(MOTOR4_PIN2, OUTPUT);

  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  servo.attach(SERVO_PIN, 1000, 2000);
  servo.write(SERVO_START);

  Serial.begin(9600);

  Serial.println("Robot starting...");
}

void reverse() {
  digitalWrite(MOTOR1_PIN1, HIGH);
  digitalWrite(MOTOR1_PIN2, LOW);
  digitalWrite(MOTOR2_PIN1, HIGH);
  digitalWrite(MOTOR2_PIN2, LOW);
  digitalWrite(MOTOR3_PIN1, HIGH);
  digitalWrite(MOTOR3_PIN2, LOW);
  digitalWrite(MOTOR4_PIN1, HIGH);
  digitalWrite(MOTOR4_PIN2, LOW);
}

void forward() {
  digitalWrite(MOTOR1_PIN1, LOW);
  digitalWrite(MOTOR1_PIN2, HIGH);
  digitalWrite(MOTOR2_PIN1, LOW);
  digitalWrite(MOTOR2_PIN2, HIGH);
  digitalWrite(MOTOR3_PIN1, LOW);
  digitalWrite(MOTOR3_PIN2, HIGH);
  digitalWrite(MOTOR4_PIN1, LOW);
  digitalWrite(MOTOR4_PIN2, HIGH);
}

void stopMotors() {
  digitalWrite(MOTOR1_PIN1, LOW);
  digitalWrite(MOTOR1_PIN2, LOW);
  digitalWrite(MOTOR2_PIN1, LOW);
  digitalWrite(MOTOR2_PIN2, LOW);
  digitalWrite(MOTOR3_PIN1, LOW);
  digitalWrite(MOTOR3_PIN2, LOW);
  digitalWrite(MOTOR4_PIN1, LOW);
  digitalWrite(MOTOR4_PIN2, LOW);
}

void throwBall() {

  stopMotors();

  for (int angle = SERVO_START; angle <= SERVO_THROW; angle++) {
    servo.write(angle);
    delay(10);
  }

  delay(50);
}

float readDistance() {

  float total = 0;
  int samples = 3;

  for (int i = 0; i < samples; i++) {

    digitalWrite(TRIG_PIN, LOW);
    delayMicroseconds(2);

    digitalWrite(TRIG_PIN, HIGH);
    delayMicroseconds(10);
    digitalWrite(TRIG_PIN, LOW);

    unsigned long duration = pulseIn(ECHO_PIN, HIGH, 30000);

    if (duration == 0) {
      return 999; 
    }

    float distance = duration * SPEED_OF_SOUND / 2;
    total += distance;

    delay(5);
  }

  return total / samples;
}

void loop() {
  float distance = readDistance();

  Serial.print("Distance: ");
  Serial.println(distance);

  switch (state) {

    case MOVING_FORWARD:

      forward();
      if (distance <= threshold) {
        state = THROWING_BALL;
      }
      break;

    case THROWING_BALL:

      Serial.println("Wall reached. Throwing ball.");
      throwBall();
      reverseStartTime = millis();
      state = REVERSING;
      break;

    case REVERSING:

      reverse();
      if (millis() - reverseStartTime >= REVERSE_TIME) {
        stopMotors();
        state = STOPPED;
      }
      break;

    case STOPPED:
      stopMotors();
      break;
  }

  delay(20);
}