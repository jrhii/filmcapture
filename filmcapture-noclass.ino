//Film Scanner V08.05.20
//Author: Jim Harter
//Purpose: Control movie-film digitizer.  A take-up reel is powered by a stepper motor and a servo serves to acuate
//  the shutter of a digital camera pointed at a frame.  Frame placement is controlled by a photo-resistor sensing
//  light from an LED through the films sprocket holes.  In this iteration, can capture 16mm, 8mm, and super film,
//  while a sprocket counter function would allow for the proper advancement of 35mm film.  The Stepper and Servo
//  driving functions utilize Timer1 and are non-blocking.  

#include <TimerOne.h>
bool isTimerOneOccupied = false;

const int LED_PIN_ARRAY[3] = {11,5,3};

const int PHOTOCELL_AIN = A0;
const int PHOTOCELL_DIN = 2;

const int STEPPER_PIN_ARRAY[4] = {13,4,7,8};
const int STEPPER_STEPS=513;
const int STEPPER_RPM=15;

const int SERVO_PIN = 9;
const int SERVO_MIN_PERIOD = 900;
const int SERVO_MAX_PERIOD = 2100;
const int SERVO_SWEEP = 120;
const unsigned int SHUTTER_SPEED = 1000/30;//probably needs some extra time for pre shutter
const unsigned int SERVO_DEGREE_SPEED = 2; //time to swing one degree
const int SERVO_ANGLE = 45;//angle of servo swing

unsigned long nextStepTime = 0;//minimum time to next step
int nextStep = 0;//next step to execute in loop

//STEPPER SECTION
volatile int stepperLowPin;
bool stepperIsDirectionForward;
int stepperStepsPerRev = -1;
int stepperRPM = -1;
const int stepperPinLookup[4][4] = 
{
  {1,0,0,1},
  {1,1,0,0},
  {0,1,1,0},
  {0,0,1,1}
};

void stepperStep() {
  stepperLowPin = stepperIsDirectionForward ? stepperLowPin + 1: stepperLowPin -1;
  stepperLowPin = stepperLowPin < 0 ? 3 : stepperLowPin;
  stepperLowPin = stepperLowPin > 3 ? 0 : stepperLowPin;

  for(int i = 0; i < 4; i++) {
    digitalWrite(STEPPER_PIN_ARRAY[i], stepperPinLookup[stepperLowPin][i] == 1 ? LOW : HIGH);
  }
}//execute singleStep

unsigned long stepperGetTimerOneDelay() {
  if (stepperStepsPerRev > 0 && stepperRPM > 0) {
    return 60000000/((unsigned long)stepperStepsPerRev * (unsigned long)stepperRPM);
  } else {
    return 5000;
  }
}

void stepperSetup(const int pinArray[], int stepsPerRev = -1, int rpm = -1) {
    stepperLowPin = 0;//this is from the private variables perspective
    stepperIsDirectionForward = true;
    stepperStepsPerRev = stepsPerRev;
    stepperRPM = rpm;

    for(int i = 0; i < 4; i++) {
      pinMode(STEPPER_PIN_ARRAY[i],OUTPUT);
      digitalWrite(STEPPER_PIN_ARRAY[i], HIGH);
    }
}

void stepperRun(bool forward = true) {
  if (isTimerOneOccupied) {
    //throw 1;
    return;
  }
  isTimerOneOccupied = true;
  stepperIsDirectionForward = forward;
  
  Timer1.setPeriod(stepperGetTimerOneDelay());
  Timer1.attachInterrupt(stepperStep); 
  delay(0);//apparently needed
  Timer1.start();
}
void stepperStop(){
  Serial.println("stopping");
  Timer1.stop();
  Timer1.detachInterrupt();
  isTimerOneOccupied = false;
  nextStep = 1;
}
//END stepper section





//START SERVO section
class Servo {
  private:
    int periodMin;
    int periodMax;
    int servoPin;
    int servoSweep;
    bool* isTimerOneOccupied;
    bool hasControl;

    int getFrequency(float sweepDegrees) {
      float periodOverDegree = (float)(this->periodMax - this->periodMin)/(float)this->servoSweep;
      int periodOffset = (int)(periodOverDegree*(float)min(sweepDegrees, this->servoSweep));

      return periodOffset+periodMin;
    }

  public:
    Servo(int pin, int pMin, int pMax, int servoSweep, bool *isTimerOneOccupiedPointer) {
      //throw error for pin != 9 or 10?
      this->servoPin = pin;
      this->periodMin = pMin;
      this->periodMax = pMax; 
      this->servoSweep=servoSweep;
      this->isTimerOneOccupied = isTimerOneOccupiedPointer;
      this->hasControl = false;

      pinMode(this->servoPin, OUTPUT);
    }
    void actuate(int degrees){
      if(!this->hasControl && *this->isTimerOneOccupied) {
        return;  
      }

      *this->isTimerOneOccupied = true;
      this->hasControl = true;

      Timer1.setPeriod(20000);
      int duty = (int)((float)(this->getFrequency((float)degrees)/20000.0)*1023.0);

      Serial.println(duty);
      Timer1.pwm(this->servoPin, duty);
      
    }
    void actuate(float degrees){
      if(!this->hasControl && *this->isTimerOneOccupied) {
        return;  
      }

      *this->isTimerOneOccupied = true;
      this->hasControl = true;

      Timer1.setPeriod(20000);
      int duty = (int)((float)(this->getFrequency((float)degrees)/20000.0)*1023.0);
      Timer1.pwm(this->servoPin, 511);}
    void releaseTimer() {
      Timer1.disablePwm(this->servoPin);
      this->hasControl = false;
      *this->isTimerOneOccupied = false;
    }
};
//END SERVO section

unsigned long getServoTime() {//calculate time for servo to actuate (in one direction)
  return max((unsigned long)(SERVO_DEGREE_SPEED*SERVO_ANGLE*2), 50);
}

Servo shutterServo(SERVO_PIN, SERVO_MIN_PERIOD, SERVO_MAX_PERIOD, SERVO_SWEEP, &isTimerOneOccupied); 

bool breakOut = false;

void serialEvent() {
  breakOut = true;  
}

void setup() {
  Serial.begin(9600);
  Serial.println("Begin Prog");
  Timer1.initialize(1000000);//arbitrary period
  Timer1.stop();
  stepperSetup(STEPPER_PIN_ARRAY, STEPPER_STEPS, STEPPER_RPM);
  //initiate LED
  for(int i = 0; i < 3; i++) {
    pinMode(LED_PIN_ARRAY[i],OUTPUT);
    analogWrite(LED_PIN_ARRAY[0],20);
  }
  //initiate AnalogIn
  //initiate readout
//  while(breakOut == false) {
//    Serial.println(analogRead(PHOTOCELL_AIN));
//  }

  //close analogIn, close rx, close readout
  //Serial.end();
  pinMode(PHOTOCELL_DIN, INPUT);
}

void loop() {
  //nextStep = -1;
  switch(nextStep) {
    case 0://advance film
      if (millis() > nextStepTime) {
        stepperRun();
        //add sensor interrupt
        attachInterrupt(digitalPinToInterrupt(PHOTOCELL_DIN),stepperStop,FALLING);
        nextStep = -1;//holding pattern
        nextStepTime = millis() + 6000;//wait a second for testing
      }
      break;
    case 1://capture
      detachInterrupt(digitalPinToInterrupt(PHOTOCELL_DIN));
      //actuate servo...set wait time;
      shutterServo.actuate(SERVO_ANGLE);
      nextStepTime = millis() + getServoTime();
      while(millis() < nextStepTime) {};//is this worst than delay?
      shutterServo.actuate(0);
      nextStepTime = millis() + getServoTime();
      nextStep = 2;
      break;
    case 2://release servo
      if (millis() > nextStepTime) {
        shutterServo.releaseTimer();
        unsigned long captureTime = (unsigned long)SHUTTER_SPEED;
        nextStepTime = millis() + captureTime;
        nextStep = 0;
      }
      break;
    case -1:
      if (millis() > nextStepTime && false) {
        detachInterrupt(digitalPinToInterrupt(PHOTOCELL_DIN));
        stepperStop();
        for (int i = 0; i < 3; i++) {
          analogWrite(LED_PIN_ARRAY[i], 255);
        }
        nextStep = -2;//exit
        }
      break;
  }
}
