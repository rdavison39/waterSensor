#ifndef MOTION_SENSOR_H
#define MOTION_SENSOR_H

#include <Arduino.h>

void setupMotionSensor();
void loopMotionSensor();

bool isMotionDetected();
unsigned long getMotionCount();
String getLastMotionTime();
String getLastMotionEmailTime();
unsigned long getMotionEmailsToday();
unsigned long getSuppressedMotionCount();
unsigned long getMotionDetectionsDuringCooldown();
unsigned long getMotionCooldownSecondsRemaining();

#endif // MOTION_SENSOR_H
