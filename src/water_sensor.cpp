#include "water_sensor.h"
#include "config.h"
#include "event_log.h"
#include "email_manager.h"

//====================================================
// Water sensor timing
//====================================================

const unsigned long WATER_WET_CONFIRM_MS = 1000;   // 1 second
const unsigned long WATER_DRY_CONFIRM_MS = 60000;  // 60 seconds

//====================================================
// Sensor state
//====================================================

bool sensor1Detected = false;
bool sensor2Detected = false;

//====================================================
// Setup
//====================================================

void setupWaterSensor()
{
    // HIGH = DRY
    // LOW  = WET
    pinMode(WATER_SENSOR_1_PIN, INPUT_PULLUP);
    pinMode(WATER_SENSOR_2_PIN, INPUT_PULLUP);

    Serial.println();
    Serial.println("========================================");
    Serial.println("WATER SENSOR");
    Serial.println("GPIO27 = Sensor 1");
    Serial.println("GPIO26 = Sensor 2");
    Serial.println("INPUT_PULLUP ENABLED");
    Serial.println("========================================");
}

//====================================================
// Trigger water alarm
//====================================================

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

//====================================================
// Clear water alarm
//====================================================

void triggerDry()
{
    bool wasWet = waterDetected;

    waterDetected = false;
    alarmEmailSent = false;
    currentAlertIntervalIndex = 0;

    Serial.println("SENSORS DRY - ALARM CLEARED");

    addEvent("All sensors returned to DRY");

    // Only send recovery email if there was
    // an actual water alarm.
    if (wasWet)
    {
        Serial.println("[EMAIL] Source: Water Alarm Cleared");

        sendWaterClearedEmail();
    }
}

//====================================================
// Main water sensor processing
//====================================================

void loopWaterSensor()
{
    static int lastRawState1 = HIGH;
    static int lastRawState2 = HIGH;

    static unsigned long stateChangeMillis1 = 0;
    static unsigned long stateChangeMillis2 = 0;

    static unsigned long lastStatusReportMillis = 0;

    int raw1 = digitalRead(WATER_SENSOR_1_PIN);
    int raw2 = digitalRead(WATER_SENSOR_2_PIN);

    //====================================================
    // Track Sensor 1 raw state changes
    //====================================================

    if (raw1 != lastRawState1)
    {
        lastRawState1 = raw1;
        stateChangeMillis1 = millis();

        Serial.print("[WATER] Sensor 1 raw state = ");
        Serial.println(raw1 == LOW ? "WET" : "DRY");
    }

    //====================================================
    // Track Sensor 2 raw state changes
    //====================================================

    if (raw2 != lastRawState2)
    {
        lastRawState2 = raw2;
        stateChangeMillis2 = millis();

        Serial.print("[WATER] Sensor 2 raw state = ");
        Serial.println(raw2 == LOW ? "WET" : "DRY");
    }

    //====================================================
    // Confirm Sensor 1 WET
    //
    // Must remain LOW continuously for
    // WATER_WET_CONFIRM_MS.
    //====================================================

    if (raw1 == LOW &&
        !sensor1Detected &&
        (millis() - stateChangeMillis1) >= WATER_WET_CONFIRM_MS)
    {
        sensor1Detected = true;

        Serial.println();
        Serial.println("*** SENSOR 1 CONFIRMED WET ***");
        Serial.println();

        addEvent("Sensor 1 WET");
    }

    //====================================================
    // Confirm Sensor 2 WET
    //
    // Must remain LOW continuously for
    // WATER_WET_CONFIRM_MS.
    //====================================================

    if (raw2 == LOW &&
        !sensor2Detected &&
        (millis() - stateChangeMillis2) >= WATER_WET_CONFIRM_MS)
    {
        sensor2Detected = true;

        Serial.println();
        Serial.println("*** SENSOR 2 CONFIRMED WET ***");
        Serial.println();

        addEvent("Sensor 2 WET");
    }

    //====================================================
    // Confirm Sensor 1 DRY
    //
    // Must remain HIGH continuously for
    // WATER_DRY_CONFIRM_MS.
    //====================================================

    if (raw1 == HIGH &&
        sensor1Detected &&
        (millis() - stateChangeMillis1) >= WATER_DRY_CONFIRM_MS)
    {
        sensor1Detected = false;

        Serial.println();
        Serial.println("*** SENSOR 1 CONFIRMED DRY ***");
        Serial.println();

        addEvent("Sensor 1 DRY");
    }

    //====================================================
    // Confirm Sensor 2 DRY
    //
    // Must remain HIGH continuously for
    // WATER_DRY_CONFIRM_MS.
    //====================================================

    if (raw2 == HIGH &&
        sensor2Detected &&
        (millis() - stateChangeMillis2) >= WATER_DRY_CONFIRM_MS)
    {
        sensor2Detected = false;

        Serial.println();
        Serial.println("*** SENSOR 2 CONFIRMED DRY ***");
        Serial.println();

        addEvent("Sensor 2 DRY");
    }

    //====================================================
    // Overall water status
    //
    // Either sensor being confirmed WET means
    // the overall system is WET.
    //
    // Both sensors must be confirmed DRY before
    // the overall system returns to DRY.
    //====================================================

    bool overallWet = sensor1Detected || sensor2Detected;

    if (overallWet && !waterDetected)
    {
        triggerAlarm();
    }

    if (!overallWet && waterDetected)
    {
        triggerDry();
    }

    //====================================================
    // Normal status report every 60 seconds
    //====================================================

    if (millis() - lastStatusReportMillis >= 60000)
    {
        lastStatusReportMillis = millis();

        Serial.print("[WATER] Status: Sensor 1 = ");
        Serial.print(sensor1Detected ? "WET" : "DRY");

        Serial.print(" | Sensor 2 = ");
        Serial.print(sensor2Detected ? "WET" : "DRY");

        Serial.print(" | Overall = ");
        Serial.println(waterDetected ? "WET" : "DRY");
    }
}