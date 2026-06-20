#include "motion_sensor.h"
#include "config.h"
#include "event_log.h"
#include "utils.h"

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

        motionCount++;

        lastMotionTime = getCurrentTime();

        addEvent("Motion detected");
    }
    else if (motionRaw == LOW && motionState)
    {
        motionState = false;

        addEvent("Motion ended");
    }
}

bool isMotionDetected()
{
    return motionState;
}

unsigned long getMotionCount()
{
    return motionCount;
}

String getLastMotionTime()
{
    return lastMotionTime;
}