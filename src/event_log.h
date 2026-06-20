#ifndef EVENT_LOG_H
#define EVENT_LOG_H

#include <Arduino.h>

void addEvent(const String &eventText);
String getEvent(int index); // 0 = newest, MAX_EVENTS-1 = oldest
int getEventCapacity();

#endif // EVENT_LOG_H
