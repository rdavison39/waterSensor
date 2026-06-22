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
        Serial.println("[EMAIL] Source: Water Alarm");
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
    static int lastRawState = HIGH;
    static unsigned long stateChangeMillis = 0;

    int waterStateRaw = digitalRead(WATER_PIN);

    // detect raw state change
    if (waterStateRaw != lastRawState)
    {
        lastRawState = waterStateRaw;
        stateChangeMillis = millis();

if (waterStateRaw == LOW)
{
    Serial.println("[WATER] Pin LOW (WET)");
}
else
{
    Serial.println("[WATER] Pin HIGH (DRY)");
}
    }

    // WET debounce (2 seconds)
    if (waterStateRaw == LOW &&
        !waterDetected &&
        (millis() - stateChangeMillis) >= 2000)
    {
        Serial.println("*** WATER CONFIRMED WET ***");

        triggerAlarm();
        addEvent("Water sensor: WET (hardware)");
    }

    // DRY debounce (60 seconds)
    if (waterStateRaw == HIGH &&
        waterDetected &&
        (millis() - stateChangeMillis) >= 60000)
    {
        Serial.println("*** WATER CONFIRMED DRY ***");

        triggerDry();
        addEvent("Water sensor: DRY (hardware)");
    }
}