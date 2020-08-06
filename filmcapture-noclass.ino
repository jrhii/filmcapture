//Film Scanner V08.06.20a
//Author: Jim Harter
//Purpose: Control movie-film digitizer.  A take-up reel is powered by a stepper motor and a servo serves to actuate
//  the shutter of a digital camera pointed at a frame.  Frame placement is controlled by a photo-resistor sensing
//  light from an LED through the films sprocket holes.  In this iteration, can capture 16mm, 8mm, and super film,
//  while a sprocket counter function would allow for the proper advancement of 35mm film.

#include <Stepper.h>
#include <Servo.h>
bool isTimerOneOccupied = false;

const int LED_PIN_ARRAY[3] = {11,5,3};

const int PHOTOCELL_AIN = A0;
const int PHOTOCELL_DIN = 2;

const int STEPPER_PIN_ARRAY[4] = {13,4,7,8};
const int STEPPER_STEPS=513;
const int STEPPER_RPM=15;

const int SERVO_PIN = 9;
const int SERVO_MIN_WIDTH = 900;
const int SERVO_MAX_WIDTH = 2100;
const int SERVO_SWEEP = 120;
const unsigned int SHUTTER_SPEED = 1000/30;//probably needs some extra time for pre shutter
const unsigned int SERVO_DEGREE_SPEED = 2; //time to swing one degree
const int SERVO_ANGLE = 45;//angle of servo swing

unsigned long nextStepTime = 0;//minimum time to next step
int nextStep = 0;//next step to execute in loop
int photocellLast;//keep track of photocell to track falling
int stepperBeginTime;//track how long stepper has run in one go


int getServoPulseWidth(float sweepDegrees) {
  float uSecOverDegree = (float)(SERVO_MAX_WIDTH - SERVO_MIN_WIDTH)/(float)SERVO_SWEEP;
  int periodOffset = (int)(uSecOverDegree*sweepDegrees);

  return min(periodOffset+SERVO_MIN_WIDTH, SERVO_MAX_WIDTH);
}

unsigned long getServoTime() {//calculate time for servo to actuate (in one direction)
  return max((unsigned long)(SERVO_DEGREE_SPEED*SERVO_ANGLE*2), 50);
}

Servo shutterServo; 

Stepper takeupReel(STEPPER_STEPS, STEPPER_PIN_ARRAY[2],STEPPER_PIN_ARRAY[0],STEPPER_PIN_ARRAY[1],STEPPER_PIN_ARRAY[3]);

void setup() {
  //initiate LED
  for(int i = 0; i < 3; i++) {
    pinMode(LED_PIN_ARRAY[i],OUTPUT);
    analogWrite(LED_PIN_ARRAY[0],20);
  }

  shutterServo.attach(SERVO_PIN, SERVO_MIN_WIDTH, SERVO_MAX_WIDTH);
  takeupReel.setSpeed(STEPPER_RPM);

  pinMode(PHOTOCELL_DIN, INPUT);
}

void loop() {
  switch(nextStep) {
    case 0://advance film
      if (millis() > nextStepTime) {
        nextStepTime = 0;
        nextStep = 1;
        photocellLast = digitalRead(PHOTOCELL_DIN);
        stepperBeginTime = millis();
      }
      break;
    case 1:
      if (millis() - stepperBeginTime > 10000) {
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
      {
        shutterServo.writeMicroseconds(getServoPulseWidth(SERVO_ANGLE));
        nextStepTime = millis() + getServoTime();
        
        while(millis() < nextStepTime) {};//wait for servo to actuate;
       
        shutterServo.writeMicroseconds(SERVO_MIN_WIDTH);
        
        unsigned long captureTime = (unsigned long)SHUTTER_SPEED;
        nextStepTime = millis() + captureTime;
        
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
