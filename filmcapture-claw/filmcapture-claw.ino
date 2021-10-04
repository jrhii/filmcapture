//Film Scanner V09.29.21
//Author: Jim Harter
//Purpose: Control movie-film digitizer.  Made for 16mm film, using 3 steppers,
//one for feed, one for takeup, and one for a pull down claw.

#include <Stepper.h> 

const int CAPTURE_FPS = 2;

const int STEPPERS[3][4] = {{2, 3, 4, 5}, {12, 13, 6, 7}, {8, 9, 10, 11}};
const int STEPS_PER_REV = 2052;            //.096
const float LEAP_STEP = 21374;             //add one step after this many steps
float STEP_CORRECTOR[3] = {0, 0, 0}; //number of steps since last correction
float STEP_DRIFT[3] = {0, 0, 0};     //if we need to round our desired angle
//to the nearest step, record.  Correct if over by one;

const int IR_PIN = A0;

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

void rotateAngle(float angle, int stepperIndex, Stepper *stepperMotor)
{
    float floatSteps = angleToSteps(angle);
    int steps = (int)floatSteps;

    const float stepRemainder = floatSteps - floor(floatSteps);

    //adjust the step drift for this motor
    if (stepRemainder >= 0.5)
    {
        STEP_DRIFT[stepperIndex] -= stepRemainder;
        steps++;
    }
    else
    {
        STEP_DRIFT[stepperIndex] += stepRemainder;
    }

    //add adjust steps for step drift
    while (abs(STEP_DRIFT[stepperIndex]) > 1)
    {
        steps += int(STEP_DRIFT[stepperIndex]);

        STEP_DRIFT[stepperIndex] -= floor(STEP_DRIFT[stepperIndex]);
    }

    //Track and adjust step per rev drift
    STEP_CORRECTOR[stepperIndex] += steps;
    while (STEP_CORRECTOR[stepperIndex] >= LEAP_STEP)
    {
        steps += 1;
        STEP_CORRECTOR[stepperIndex] -= LEAP_STEP;
    }

    stepperMotor->step(steps);
}

const int *FEED_STEP_A = STEPPERS[0];
Stepper feedReel(STEPS_PER_REV, FEED_STEP_A[0], FEED_STEP_A[1], FEED_STEP_A[2], FEED_STEP_A[3]);
const int *FEED_STEP_B = STEPPERS[1];
Stepper takeupReel(STEPS_PER_REV, FEED_STEP_B[0], FEED_STEP_B[1], FEED_STEP_B[2], FEED_STEP_B[3]);
const int *FEED_STEP_C = STEPPERS[2];
Stepper pullDown(STEPS_PER_REV, FEED_STEP_C[0], FEED_STEP_C[1], FEED_STEP_C[2], FEED_STEP_C[3]);

void setup()
{
    feedReel.setSpeed(5);
    takeupReel.setSpeed(5);
    pullDown.setSpeed(6);

    pinMode(IR_PIN, OUTPUT);
    digitalWrite(IR_PIN, LOW);
}

void loop()
{
    //rotateAngle(45, 0, &feedReel);  //feed
    rotateAngle(45, 2, &pullDown); //pulldown
    takePhoto();
    //rotateAngle(45, 1, &takeupReel); //takeup
}
