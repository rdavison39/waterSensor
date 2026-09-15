#include <Arduino.h>
#include <WiFi.h>
#include <time.h>
#include <LittleFS.h>

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
// Timezone
//====================================================

// Eastern Time:
// EST = UTC-5
// EDT = UTC-4
// DST starts second Sunday in March
// DST ends first Sunday in November
const char *TIMEZONE_STRING =
    "EST5EDT,M3.2.0,M11.1.0";

//====================================================
// LittleFS / ESP-Mail-Client Timezone File
//====================================================

bool setupLittleFS()
{
    Serial.println("================================");
    Serial.println("Initializing LittleFS...");
    Serial.println("================================");

    if (!LittleFS.begin(true))
    {
        Serial.println("[LITTLEFS] Mount FAILED");
        return false;
    }

    Serial.println("[LITTLEFS] Mounted successfully");

    const char *timezoneFile = "/tze.txt";

    bool needsWrite = true;

    if (LittleFS.exists(timezoneFile))
    {
        File file = LittleFS.open(timezoneFile, "r");

        if (file)
        {
            String existing = file.readString();
            existing.trim();
            file.close();

            if (existing == TIMEZONE_STRING)
            {
                needsWrite = false;
                Serial.println("[LITTLEFS] tze.txt already correct");
            }
            else
            {
                Serial.println("[LITTLEFS] Existing tze.txt has wrong timezone");
            }
        }
    }

    if (needsWrite)
    {
        File file = LittleFS.open(timezoneFile, "w");

        if (!file)
        {
            Serial.println("[LITTLEFS] Could not create /tze.txt");
            return false;
        }

        file.print(TIMEZONE_STRING);
        file.close();

        Serial.println("[LITTLEFS] Created /tze.txt");
        Serial.print("[LITTLEFS] Timezone: ");
        Serial.println(TIMEZONE_STRING);
    }

    return true;
}

//====================================================
// Setup
//====================================================

void setup()
{
    bootMillis = millis();

    Serial.begin(115200);

    delay(500);

    Serial.println();
    Serial.println("================================");
    Serial.println("ESP32 WATER SENSOR CONTROLLER");
    Serial.print("Firmware: v");
    Serial.println(FIRMWARE_VERSION);
    Serial.println("================================");

    //------------------------------------------------
    // Settings
    //------------------------------------------------

    setupSettings();

    //------------------------------------------------
    // LittleFS
    //------------------------------------------------

    setupLittleFS();

    //------------------------------------------------
    // Timezone
    //------------------------------------------------

    setenv("TZ", TIMEZONE_STRING, 1);
    tzset();

    Serial.print("Timezone: ");
    Serial.println(TIMEZONE_STRING);

    //------------------------------------------------
    // WiFi
    //------------------------------------------------

    while (!connectToWiFi())
    {
        delay(30000);
    }

    //------------------------------------------------
    // Time / NTP
    //------------------------------------------------

    configTzTime(
        TIMEZONE_STRING,
        "pool.ntp.org",
        "time.nist.gov"
    );

    Serial.println("Waiting for NTP time sync...");

    struct tm timeinfo;

    while (!getLocalTime(&timeinfo))
    {
        Serial.println("Waiting for NTP sync...");
        delay(1000);
    }

    Serial.println("NTP synchronized");

    Serial.println(
        &timeinfo,
        "%Y-%m-%d %H:%M:%S"
    );

    //------------------------------------------------
    // Startup Event
    //------------------------------------------------

    addEvent(
        "System started - v" +
        String(FIRMWARE_VERSION)
    );

    //------------------------------------------------
    // Sensors
    //------------------------------------------------

    setupWaterSensor();

    setupMotionSensor();

    //------------------------------------------------
    // Web UI
    //------------------------------------------------

    setupWebUI();

    server.begin();

    Serial.println("Web Server Started");
}

//====================================================
// Main Loop
//====================================================

void loop()
{
    //------------------------------------------------
    // Web Server
    //------------------------------------------------

    server.handleClient();

    //------------------------------------------------
    // WiFi
    //------------------------------------------------

    maintainWiFiConnection();

    //------------------------------------------------
    // Startup Email
    //------------------------------------------------

    if (!startupEmailSent)
    {
        time_t now = time(nullptr);

        if (now > 1700000000)
        {
            Serial.println(
                "*** SENDING STARTUP EMAIL ***"
            );

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

    //------------------------------------------------
    // Small Loop Delay
    //------------------------------------------------

    delay(50);
}