//Film Scanner V10.08.21
//Author: Jim Harter
//Purpose: Control movie-film digitizer.  Made for 16mm film, using 3 steppers,
//one for feed, one for takeup, and one for a pull down claw.

#include <SpeedyStepper.h>

const long CAPTURE_RPM = 120;
const int CAPTURE_SPEED = 1000 / 8;

const int MOTOR_DIR_PIN = 0;
const int MOTOR_STEP_PIN = 1;
const int MOTOR_ENABLE_PIN = 14;

const long STEPS_PER_REV = 3200; //200 * 16 microsteps

const int IR_PIN = 18;

const int GO_PIN = 19;

//uses an IR LED to trigger a nikon remote shutter
void takePhoto(void)
{
    int i;
    for (i = 0; i < 76; i++)
    {
        digitalWrite(IR_PIN, HIGH);
        delayMicroseconds(7);
        digitalWrite(IR_PIN, LOW);
        delayMicroseconds(7);
    }
    delay(27);
    delayMicroseconds(810);
    for (i = 0; i < 16; i++)
    {
        digitalWrite(IR_PIN, HIGH);
        delayMicroseconds(7);
        digitalWrite(IR_PIN, LOW);
        delayMicroseconds(7);
    }
    delayMicroseconds(1540);
    for (i = 0; i < 16; i++)
    {
        digitalWrite(IR_PIN, HIGH);
        delayMicroseconds(7);
        digitalWrite(IR_PIN, LOW);
        delayMicroseconds(7);
    }
    delayMicroseconds(3545);
    for (i = 0; i < 16; i++)
    {
        digitalWrite(IR_PIN, HIGH);
        delayMicroseconds(7);
        digitalWrite(IR_PIN, LOW);
        delayMicroseconds(7);
    }
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

    pinMode(IR_PIN, OUTPUT);
    digitalWrite(IR_PIN, LOW);

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
            delay(200 + CAPTURE_SPEED);
            captureCount++;
            stage = 1;
        }
        break;
    }
}
