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

// Global runtime state (defined here)
bool waterDetected = false;
bool alarmEmailSent = false;
bool motionState = false;
unsigned long bootMillis;
bool startupEmailSent = false;

String lastAlarmTime = "Never";
String lastTestEmailTime = "Never";
String lastStatusEmailDate = "Never";
int alarmCounter = 0;
int emailCounter = 0;

unsigned long lastWetEmailTime = 0;
int wetEmailCounter = 0;
int currentAlertIntervalIndex = 0;
const unsigned long ALERT_INTERVALS[] = {60, 120, 240, 480, 960, 1920, 3600};
const int NUM_ALERT_INTERVALS = 7;



//====================================================
// Motion
//====================================================

unsigned long motionCount = 0;

// Number of motion emails sent today
unsigned long motionEmailsToday = 0;

// Number of motion detections suppressed during cooldown
unsigned long suppressedMotionCount = 0;

// millis() when last motion email was sent
unsigned long lastMotionEmailMillis = 0;

// Configurable cooldown (loaded from Preferences)
int motionEmailCooldownMinutes = 15;

// Runtime timestamps
String lastMotionTime = "Never";
String lastMotionEmailTimestamp = "Never";

// Date used to reset daily email counter
String lastMotionEmailDate = "";

// Heartbeat
String lastHeartbeatDate = "";

void setup()
{
    bootMillis = millis();

    Serial.begin(115200);

    // settings
setupSettings();
motionEmailCooldownMinutes = getMotionEmailCooldownMinutes();

Serial.print("[SETTINGS] Motion cooldown = ");
Serial.print(motionEmailCooldownMinutes);
Serial.println(" minutes");

while (!connectToWiFi())
{
    delay(30000);
}

    configTime(-5 * 3600, 3600, "pool.ntp.org", "time.nist.gov");
    Serial.println("Waiting for NTP time sync...");
    struct tm timeinfo;

    while (!getLocalTime(&timeinfo))
    {
        Serial.println("Waiting for NTP sync...");
        delay(1000);
    }

    Serial.println("NTP synchronized");
    Serial.println(&timeinfo, "%Y-%m-%d %H:%M:%S");

    addEvent("System started - v" + String(FIRMWARE_VERSION));

    // Initialize modules
    setupWaterSensor();
    setupMotionSensor();
    setupWebUI();

    server.begin();
    Serial.println("Web Server Started");
}

void loop()
{
    server.handleClient();
    maintainWiFiConnection();

    // startup email
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

    // Periodic status email



    // Wet alerts scheduling
    if (waterDetected && lastWetEmailTime > 0)
    {
        time_t now = time(nullptr);
        unsigned long secondsSinceLast = (unsigned long)now - lastWetEmailTime;
        unsigned long interval = ALERT_INTERVALS[currentAlertIntervalIndex];
        if (secondsSinceLast >= interval)
        {
            sendWetAlertEmail();
        }
    }

    // Let modules poll hardware
    loopWaterSensor();
    loopMotionSensor();

    // Check heartbeat (every other day)
    checkHeartbeat();

    // small delay to avoid hogging CPU
    delay(50);
}
