#include <Servo.h>

int TRIG_PIN = 13;
int ECHO_PIN = 12;
int MOTOR1_PIN1 = 2;
int MOTOR1_PIN2 = 7;
int MOTOR2_PIN1 = 8;
int MOTOR2_PIN2 = 4;
int SERVO_PIN = 3;

const float SPEED_OF_SOUND = 0.0343;
const int threshold = 10;

const int SERVO_START = 180;
const int SERVO_THROW = 0;

const int START_DELAY = 50;       // milliseconds for initial setup
const int REVERSE_TIME = 20000;   // milliseconds
const int THROW_DELAY = 3000;     // 3 second delay before first throw allowed

Servo servo;

enum RobotState {
  MOVING_FORWARD,
  THROWING_BALL,
  REVERSING,
  STOPPED
};

RobotState state = MOVING_FORWARD;
unsigned long reverseStartTime = 0;
unsigned long programStartTime = 0;

int consecutiveBelowThreshold = 0;  // count of consecutive readings below threshold

void setup() {
  pinMode(MOTOR1_PIN1, OUTPUT);
  pinMode(MOTOR1_PIN2, OUTPUT);
  pinMode(MOTOR2_PIN1, OUTPUT);
  pinMode(MOTOR2_PIN2, OUTPUT);

  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  servo.attach(SERVO_PIN, 1000, 2000);
  servo.write(SERVO_START);

  Serial.begin(9600);
  Serial.println("Robot starting...");

  programStartTime = millis(); // record program start time

  delay(START_DELAY);
}

void reverse() {
  digitalWrite(MOTOR1_PIN1, HIGH);
  digitalWrite(MOTOR1_PIN2, LOW);
  digitalWrite(MOTOR2_PIN1, HIGH);
  digitalWrite(MOTOR2_PIN2, LOW);
}

void forward() {
  digitalWrite(MOTOR1_PIN1, LOW);
  digitalWrite(MOTOR1_PIN2, HIGH);
  digitalWrite(MOTOR2_PIN1, LOW);
  digitalWrite(MOTOR2_PIN2, HIGH);
}

void stopMotors() {
  digitalWrite(MOTOR1_PIN1, LOW);
  digitalWrite(MOTOR1_PIN2, LOW);
  digitalWrite(MOTOR2_PIN1, LOW);
  digitalWrite(MOTOR2_PIN2, LOW);
}

void throwBall() {
  stopMotors();

  for (int angle = SERVO_START; angle >= SERVO_THROW; angle -= 10) {
    servo.write(angle);
    delay(5);
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
  Serial.print(distance);
  Serial.print(", State: ");

  switch (state) {

    case MOVING_FORWARD:
      Serial.println("Forward");
      forward();

      // Only allow throwing if 3 seconds have passed
      if (millis() - programStartTime >= THROW_DELAY) {
        if (distance <= threshold) {
          consecutiveBelowThreshold++;
        } else {
          consecutiveBelowThreshold = 0; // reset if reading is above threshold
        }

        if (consecutiveBelowThreshold >= 2) { // need 2 consecutive readings below threshold
          state = THROWING_BALL;
          consecutiveBelowThreshold = 0; // reset counter
        }
      }
      break;

    case THROWING_BALL:
      Serial.println("Throwing");
      Serial.println("Wall reached. Throwing ball.");
      throwBall();
      reverseStartTime = millis();
      state = REVERSING;
      break;

    case REVERSING:
      Serial.println("Reversing");
      reverse();
      if (millis() - reverseStartTime >= REVERSE_TIME) {
        stopMotors();
        state = STOPPED;
      }
      break;

    case STOPPED:
      Serial.println("Stopped");
      stopMotors();
      break;
  }

  delay(20);
}
