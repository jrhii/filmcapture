//Film Scanner V08.07.20
//Author: Jim Harter
//Purpose: Control movie-film digitizer.  A take-up reel is powered by a stepper motor and a servo serves to actuate
//  the shutter of a digital camera pointed at a frame.  Frame placement is controlled by a photo-resistor sensing
//  light from an LED through the films sprocket holes.  In this iteration, can capture 16mm, 8mm, and super film,
//  while a sprocket counter function would allow for the proper advancement of 35mm film.

#include <Stepper.h>
#include <Servo.h>

const unsigned int SHUTTER_SPEED = 1000 / 40 + 100; //probably needs some extra time for pre shutter
const unsigned int MINIMUM_INTERVAL = 1000;

const int LED_PIN_ARRAY[3] = {11, 5, 3};

const int PHOTOCELL_AIN = A0;
const int PHOTOCELL_DIN = 2;

const int STEPPER_PIN_ARRAY[4] = {13, 4, 7, 8};
const int STEPPER_STEPS = 513;
const int STEPPER_RPM = 15;

const int LEDpin = 9;
//const int SERVO_PIN = 9;
//const int SERVO_MIN_WIDTH = 900;
//const int SERVO_MAX_WIDTH = 2100;
//const int SERVO_SWEEP = 120;
//const unsigned int SERVO_DEGREE_SPEED = 2; //time to swing one degree
//const int SERVO_ANGLE = 30;//angle of servo swing

unsigned long nextStepTime = 0;//minimum time to next step
unsigned long lastCaptureTime = 0;
int nextStep = 0;//next step to execute in loop
int photocellLast;//keep track of photocell to track falling
int stepperBeginTime;//track how long stepper has run in one go


//int getServoPulseWidth(float sweepDegrees) {
//  float uSecOverDegree = (float)(SERVO_MAX_WIDTH - SERVO_MIN_WIDTH) / (float)SERVO_SWEEP;
//  int periodOffset = (int)(uSecOverDegree * sweepDegrees);
//
//  return min(periodOffset + SERVO_MIN_WIDTH, SERVO_MAX_WIDTH);
//}

//uns+

void takePhoto(void) {
  int i;
  for (i = 0; i < 76; i++) {
    digitalWrite(LEDpin, HIGH);
    delayMicroseconds(7);
    digitalWrite(LEDpin, LOW);
    delayMicroseconds(7);
  }
  delay(27);
  delayMicroseconds(810);
  for (i = 0; i < 16; i++) {
    digitalWrite(LEDpin, HIGH);
    delayMicroseconds(7);
    digitalWrite(LEDpin, LOW);
    delayMicroseconds(7);
  }
  delayMicroseconds(1540);
  for (i = 0; i < 16; i++) {
    digitalWrite(LEDpin, HIGH);
    delayMicroseconds(7);
    digitalWrite(LEDpin, LOW);
    delayMicroseconds(7);
  }
  delayMicroseconds(3545);
  for (i = 0; i < 16; i++) {
    digitalWrite(LEDpin, HIGH);
    delayMicroseconds(7);
    digitalWrite(LEDpin, LOW);
    delayMicroseconds(7);
  }

}

//Servo shutterServo;

Stepper takeupReel(STEPPER_STEPS, STEPPER_PIN_ARRAY[2], STEPPER_PIN_ARRAY[0], STEPPER_PIN_ARRAY[1], STEPPER_PIN_ARRAY[3]);

void setup() {
  //initiate LED
  for (int i = 0; i < 3; i++) {
    pinMode(LED_PIN_ARRAY[i], OUTPUT);
    analogWrite(LED_PIN_ARRAY[0], 20);
  }

//  shutterServo.attach(SERVO_PIN, SERVO_MIN_WIDTH, SERVO_MAX_WIDTH);
//  shutterServo.writeMicroseconds(SERVO_MIN_WIDTH);
  takeupReel.setSpeed(STEPPER_RPM);

  pinMode(PHOTOCELL_DIN, INPUT);
}

void loop() {
  switch (nextStep) {
    case 0://advance film
      if (millis() > nextStepTime) {
        nextStepTime = 0;
        nextStep = 1;
        photocellLast = digitalRead(PHOTOCELL_DIN);
        stepperBeginTime = millis();
      }
      break;
    case 1://stepper loop
      if (millis() - stepperBeginTime > 10000 && false) {
        nextStep = -1;//end of reel, hopefully
        break;
      }
      if (photocellLast == HIGH && digitalRead(PHOTOCELL_DIN) == LOW) {
        nextStep = 2;
      } else {
        photocellLast = digitalRead(PHOTOCELL_DIN);
        takeupReel.step(1);
      }
      break;
    case 2://capture
      //actuate servo...set wait time;
      if (millis() > lastCaptureTime + MINIMUM_INTERVAL) {
//        shutterServo.writeMicroseconds(getServoPulseWidth(SERVO_ANGLE));
//        nextStepTime = millis() + getServoTime();
//        lastCaptureTime = millis();
//
//        while (millis() < nextStepTime) {}; //wait for servo to actuate;

//        shutterServo.writeMicroseconds(SERVO_MIN_WIDTH);

        takePhoto();

        nextStepTime = millis() + SHUTTER_SPEED;

        nextStep = 0;
      }
      break;
    case -1:
      {
        for (int i = 0; i < 3; i++) {
          analogWrite(LED_PIN_ARRAY[i], 255);
        }
        nextStep = -2;//exit
      }
      break;
  }
}
