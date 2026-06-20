#include "water_sensor.h"
#include "config.h"
#include "event_log.h"
#include "email_manager.h"

void setupWaterSensor()
{
    pinMode(WATER_PIN, INPUT_PULLUP);
}

void triggerAlarm()
{
    waterDetected = true;
    alarmCounter++;

    if (!alarmEmailSent)
    {
        sendAlarmEmail();
        addEvent("Water alarm triggered!");
        alarmEmailSent = true;
    }
}

void triggerDry()
{
    waterDetected = false;
    alarmEmailSent = false;
    currentAlertIntervalIndex = 0;

    Serial.println("SENSOR DRY - ALARM CLEARED");
    addEvent("Sensor returned to DRY");
}

void loopWaterSensor()
{
    int waterStateRaw = digitalRead(WATER_PIN);
    // Sensor wired with pullup: LOW means wet
    if (waterStateRaw == LOW && !waterDetected)
    {
        triggerAlarm();
        addEvent("Water sensor: WET (hardware)");
    }
    else if (waterStateRaw == HIGH && waterDetected)
    {
        triggerDry();
        addEvent("Water sensor: DRY (hardware)");
    }
}
