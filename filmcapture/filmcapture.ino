//Film Scanner V10.08.21
//Author: Jim Harter
//Purpose: Control movie-film digitizer.  Made for 16mm film, using 3 steppers,
//one for feed, one for takeup, and one for a pull down claw.

#include <SpeedyStepper.h>

const long CAPTURE_RPM = 120;

const int MOTOR_DIR_PIN = 0;
const int MOTOR_STEP_PIN = 1;
const int MOTOR_ENABLE_PIN = 14;

const long STEPS_PER_REV = 3200; //200 * 16 microsteps

const int SHUTTER_PIN = 18;

const int GO_PIN = 19;

void takePhoto(void)
{
    digitalWrite(SHUTTER_PIN, HIGH);
    delay(100);
    digitalWrite(SHUTTER_PIN, LOW);
}

float angleToSteps(float angle)
{
    return STEPS_PER_REV * angle / 360;
}

SpeedyStepper sprocket;

void setup()
{
    sprocket.connectToPins(MOTOR_STEP_PIN, MOTOR_DIR_PIN);
    pinMode(MOTOR_ENABLE_PIN, OUTPUT);
    digitalWrite(MOTOR_ENABLE_PIN, LOW);
    sprocket.setSpeedInStepsPerSecond(STEPS_PER_REV * CAPTURE_RPM / 60);
    sprocket.setAccelerationInStepsPerSecondPerSecond(32000);

    pinMode(SHUTTER_PIN, OUTPUT);
    digitalWrite(SHUTTER_PIN, LOW);

    pinMode(GO_PIN, INPUT_PULLUP);
    delay(100); //establish stepper
    digitalWrite(MOTOR_ENABLE_PIN, HIGH);
}

unsigned long nextStepTime = 0;
int stage = 0;
int captureCount = 0;

void loop()
{
    switch (stage)
    {
    case 0:

        if (digitalRead(GO_PIN) == 0)
        {
            digitalWrite(MOTOR_ENABLE_PIN, LOW);
            stage = 1;
        }
        break;
    case 1:
        if (captureCount > 5000)
        {              //120ft of 16mm + some extra
            stage = 3; //complete;
            digitalWrite(MOTOR_ENABLE_PIN, HIGH);
            break;
        }
        if (millis() > nextStepTime)
        {
            nextStepTime = millis() + (60000 / CAPTURE_RPM);
            sprocket.moveRelativeInSteps(-400);
            stage = 2;
        }
        break;
    case 2:
        if (sprocket.motionComplete())
        {
            takePhoto();
            captureCount++;
            stage = 1;
        }
        break;
    }
}
