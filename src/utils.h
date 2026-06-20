#ifndef UTILS_H
#define UTILS_H

#include <Arduino.h>

String getCurrentTime();
String getCurrentDate();
int getDayOfMonth();
bool isStatusEmailDay();
String getWiFiColor();
unsigned long getSecondsUntilNextWetEmail();
String formatSecondsToTime(unsigned long seconds);
String getUptime();

#endif // UTILS_H
