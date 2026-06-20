#include "motion_sensor.h"
#include "config.h"
#include "event_log.h"

void setupMotionSensor()
{
    pinMode(MOTION_PIN, INPUT);
}

void loopMotionSensor()
{
    int motionRaw = digitalRead(MOTION_PIN);
    if (motionRaw == HIGH && !motionState)
    {
        motionState = true;
        addEvent("Motion detected");
    }
    else if (motionRaw == LOW && motionState)
    {
        motionState = false;
        addEvent("Motion ended");
    }
}
