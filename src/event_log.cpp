#include "event_log.h"
#include "config.h"
#include "utils.h"

static String eventsArr[MAX_EVENTS];
static int eventIdx = 0;

void addEvent(const String &eventText)
{
    eventsArr[eventIdx] = getCurrentTime() + " - " + eventText;
    eventIdx = (eventIdx + 1) % MAX_EVENTS;
}

String getEvent(int index)
{
    if (index < 0 || index >= MAX_EVENTS)
        return String("");

    int idx = (eventIdx - 1 - index + MAX_EVENTS) % MAX_EVENTS;
    return eventsArr[idx];
}

int getEventCapacity()
{
    return MAX_EVENTS;
}
