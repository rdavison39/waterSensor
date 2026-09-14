#include <Arduino.h>
#include <WiFi.h>
#include <time.h>

#include "secrets.h"
#include "config.h"
#include "event_log.h"
#include "utils.h"
#include "email_manager.h"
#include "water_sensor.h"
#include "motion_sensor.h"
#include "web_ui.h"
#include "heartbeat.h"
#include "settings.h"
#include "wifi_manager.h"

WebServer server(80);

//====================================================
// Global Runtime State
//====================================================

bool waterDetected = false;
bool alarmEmailSent = false;
bool motionState = false;

unsigned long bootMillis;
bool startupEmailSent = false;

//====================================================
// Water
//====================================================

String lastAlarmTime = "Never";
String lastTestEmailTime = "Never";
String lastStatusEmailDate = "Never";

int alarmCounter = 0;
int emailCounter = 0;

unsigned long lastWetEmailTime = 0;

int wetEmailCounter = 0;
int currentAlertIntervalIndex = 0;

const unsigned long ALERT_INTERVALS[] =
{
    60,
    120,
    240,
    480,
    960,
    1920,
    3600
};

const int NUM_ALERT_INTERVALS = 7;

//====================================================
// Motion
//====================================================

unsigned long motionCount = 0;

unsigned long motionEmailsToday = 0;

unsigned long suppressedMotionCount = 0;

unsigned long motionDetectionsDuringCooldown = 0;

unsigned long lastMotionEmailMillis = 0;

String lastMotionTime = "Never";

String lastMotionEmailTimestamp = "Never";

String lastMotionEmailDate = "";

String lastMotionResetDate = "";

//====================================================
// Heartbeat
//====================================================

String lastHeartbeatDate = "";

//====================================================
// Setup
//====================================================

void setup()
{
    bootMillis = millis();

    Serial.begin(115200);

    //------------------------------------------------
    // Settings
    //------------------------------------------------

    setupSettings();

    //------------------------------------------------
    // WiFi
    //------------------------------------------------

    while (!connectToWiFi())
    {
        delay(30000);
    }

    //------------------------------------------------
    // Time
    //------------------------------------------------

    configTime(-5 * 3600, 3600,
               "pool.ntp.org",
               "time.nist.gov");

    Serial.println("Waiting for NTP time sync...");

    struct tm timeinfo;

    while (!getLocalTime(&timeinfo))
    {
        Serial.println("Waiting for NTP sync...");
        delay(1000);
    }

    Serial.println("NTP synchronized");
    Serial.println(&timeinfo, "%Y-%m-%d %H:%M:%S");

    //------------------------------------------------
    // Startup
    //------------------------------------------------

    addEvent("System started - v" + String(FIRMWARE_VERSION));

    setupWaterSensor();
    setupMotionSensor();
    setupWebUI();

    server.begin();

    Serial.println("Web Server Started");
}

//====================================================
// Main Loop
//====================================================

void loop()
{
    server.handleClient();

    maintainWiFiConnection();

    //------------------------------------------------
    // Startup Email
    //------------------------------------------------

    if (!startupEmailSent)
    {
        time_t now = time(nullptr);

        if (now > 1700000000)
        {
            Serial.println("*** SENDING STARTUP EMAIL ***");

            sendStartupEmail();

            startupEmailSent = true;
        }
    }

    //------------------------------------------------
    // Repeating Wet Alerts
    //------------------------------------------------

    if (waterDetected && lastWetEmailTime > 0)
    {
        time_t now = time(nullptr);

        unsigned long secondsSinceLast =
            (unsigned long)now - lastWetEmailTime;

        unsigned long interval =
            ALERT_INTERVALS[currentAlertIntervalIndex];

        if (secondsSinceLast >= interval)
        {
            sendWetAlertEmail();
        }
    }

    //------------------------------------------------
    // Sensors
    //------------------------------------------------

    loopWaterSensor();
    loopMotionSensor();

    //------------------------------------------------
    // Heartbeat
    //------------------------------------------------

    checkHeartbeat();

    delay(50);
}