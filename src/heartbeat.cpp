#include "heartbeat.h"
#include "config.h"
#include "utils.h"
#include "email_manager.h"
#include "motion_sensor.h"
#include "settings.h"
#include "water_sensor.h"

#include <WiFi.h>
#include <time.h>

static unsigned long getLegacyLastHeartbeatTimestamp(const String& lastDate)
{
    if (lastDate.length() == 0)
        return 0;

    struct tm lastTm = {};

    if (strptime(lastDate.c_str(), "%Y-%m-%d", &lastTm) == nullptr)
        return 0;

    return (unsigned long)mktime(&lastTm);
}

void checkHeartbeat()
{
    static unsigned long lastCheckMillis = 0;

    // Only check once per minute
    if (millis() - lastCheckMillis < 60000)
        return;

    lastCheckMillis = millis();

    time_t now;
    time(&now);

    struct tm *timeinfo = localtime(&now);

    if (timeinfo == nullptr)
        return;

    char today[16];
    strftime(today, sizeof(today), "%Y-%m-%d", timeinfo);

    String todayString = String(today);

    unsigned long intervalSeconds =
        getHeartbeatIntervalMinutes() * 60UL;

    unsigned long lastHeartbeatTimestamp =
        getLastHeartbeatTimestamp();

    if (lastHeartbeatTimestamp == 0)
        lastHeartbeatTimestamp =
            getLegacyLastHeartbeatTimestamp(getLastHeartbeatDate());

    if (lastHeartbeatTimestamp > 0)
    {
        unsigned long secondsSinceLast =
            (unsigned long)now - lastHeartbeatTimestamp;

        if (secondsSinceLast < intervalSeconds)
            return;
    }

    Serial.println("[HEARTBEAT] Sending heartbeat email");

    String subject = "💚 ESP32 Water Sensor Heartbeat";

    String body;

    body += "ESP32 Water Sensor Heartbeat\n";
    body += "============================\n\n";

    body += "System Status: HEALTHY\n";
    body += "Date: " + getCurrentTime() + "\n";
    body += "Firmware Version: " + String(FIRMWARE_VERSION) + "\n\n";

    body += "Water Status: ";
    body += waterDetected ? "WET\n" : "DRY\n";

    body += "Sensor 1: ";
    body += sensor1Detected ? "WET\n" : "DRY\n";

    body += "Sensor 2: ";
    body += sensor2Detected ? "WET\n" : "DRY\n";

    body += "Motion Status: ";
    body += motionState ? "MOTION DETECTED\n" : "NO MOTION\n";

    body += "\n";

    body += "Uptime: " + getUptime() + "\n";
    body += "Motion Count: " + String(motionCount) + "\n";
    body += "Water Alarm Count: " + String(alarmCounter) + "\n";
    body += "WiFi Signal: " + String(WiFi.RSSI()) + " dBm\n";
    body += "IP Address: " + WiFi.localIP().toString() + "\n";

    if (lastAlarmTime.length() > 0)
        body += "Last Water Alarm: " + lastAlarmTime + "\n";

    if (lastMotionTime.length() > 0)
        body += "Last Motion: " + lastMotionTime + "\n";

    body += "\nSystem operating normally.\n";

    Serial.println("[EMAIL] Source: Heartbeat");

    sendEmail(subject, body);

    setLastHeartbeatDate(todayString);
    setLastHeartbeatTimestamp((unsigned long)now);

    Serial.println("[HEARTBEAT] Heartbeat sent");
}
