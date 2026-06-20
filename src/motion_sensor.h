#ifndef MOTION_SENSOR_H
#define MOTION_SENSOR_H

#include <Arduino.h>

void setupMotionSensor();
void loopMotionSensor();

bool isMotionDetected();
unsigned long getMotionCount();
String getLastMotionTime();

#endif // MOTION_SENSOR_H