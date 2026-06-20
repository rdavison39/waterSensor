#ifndef WATER_SENSOR_H
#define WATER_SENSOR_H

#include <Arduino.h>

void setupWaterSensor();
void loopWaterSensor();
void triggerAlarm();
void triggerDry();

#endif // WATER_SENSOR_H
