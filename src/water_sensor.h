#ifndef WATER_SENSOR_H
#define WATER_SENSOR_H

#include <Arduino.h>

extern bool sensor1Detected;
extern bool sensor2Detected;

void setupWaterSensor();
void loopWaterSensor();
void triggerAlarm();
void triggerDry();

#endif // WATER_SENSOR_H