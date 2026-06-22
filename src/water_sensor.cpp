#include "water_sensor.h"
#include "config.h"
#include "event_log.h"
#include "email_manager.h"

bool sensor1Detected = false;
bool sensor2Detected = false;

void setupWaterSensor()
{
    pinMode(WATER_SENSOR_1_PIN, INPUT_PULLUP);
    pinMode(WATER_SENSOR_2_PIN, INPUT_PULLUP);
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

    Serial.println("SENSORS DRY - ALARM CLEARED");
    addEvent("All sensors returned to DRY");
}

void loopWaterSensor()
{
    static int lastRawState1 = HIGH;
    static int lastRawState2 = HIGH;

    static unsigned long stateChangeMillis1 = 0;
    static unsigned long stateChangeMillis2 = 0;

    int raw1 = digitalRead(WATER_SENSOR_1_PIN);
    int raw2 = digitalRead(WATER_SENSOR_2_PIN);

    // Sensor 1 state change
    if (raw1 != lastRawState1)
    {
        lastRawState1 = raw1;
        stateChangeMillis1 = millis();

        if (raw1 == LOW)
            Serial.println("[WATER] Sensor 1 LOW (WET)");
        else
            Serial.println("[WATER] Sensor 1 HIGH (DRY)");
    }

    // Sensor 2 state change
    if (raw2 != lastRawState2)
    {
        lastRawState2 = raw2;
        stateChangeMillis2 = millis();

        if (raw2 == LOW)
            Serial.println("[WATER] Sensor 2 LOW (WET)");
        else
            Serial.println("[WATER] Sensor 2 HIGH (DRY)");
    }

    // Sensor 1 WET debounce
    if (raw1 == LOW &&
        !sensor1Detected &&
        (millis() - stateChangeMillis1) >= 2000)
    {
        sensor1Detected = true;

        Serial.println("*** SENSOR 1 CONFIRMED WET ***");
        addEvent("Sensor 1 WET");
    }

    // Sensor 2 WET debounce
    if (raw2 == LOW &&
        !sensor2Detected &&
        (millis() - stateChangeMillis2) >= 2000)
    {
        sensor2Detected = true;

        Serial.println("*** SENSOR 2 CONFIRMED WET ***");
        addEvent("Sensor 2 WET");
    }

    // Sensor 1 DRY debounce
    if (raw1 == HIGH &&
        sensor1Detected &&
        (millis() - stateChangeMillis1) >= 60000)
    {
        sensor1Detected = false;

        Serial.println("*** SENSOR 1 CONFIRMED DRY ***");
        addEvent("Sensor 1 DRY");
    }

    // Sensor 2 DRY debounce
    if (raw2 == HIGH &&
        sensor2Detected &&
        (millis() - stateChangeMillis2) >= 60000)
    {
        sensor2Detected = false;

        Serial.println("*** SENSOR 2 CONFIRMED DRY ***");
        addEvent("Sensor 2 DRY");
    }

    bool overallWet = sensor1Detected || sensor2Detected;

    if (overallWet && !waterDetected)
    {
        triggerAlarm();
    }

    if (!overallWet && waterDetected)
    {
        triggerDry();
    }
}