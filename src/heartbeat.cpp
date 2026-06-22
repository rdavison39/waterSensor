#include "heartbeat.h"
#include "config.h"
#include "utils.h"
#include "email_manager.h"
#include "motion_sensor.h"
#include "settings.h"

#include <WiFi.h>
#include <time.h>

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

    int dayOfMonth = timeinfo->tm_mday;
    int hour = timeinfo->tm_hour;
    int minute = timeinfo->tm_min;

    // Only send on odd-numbered days
    if ((dayOfMonth % 2) == 0)
        return;

    // Only send between 09:00 and 09:09 AM
    if (hour != 9 || minute > 9)
        return;

    char today[16];
    strftime(today, sizeof(today), "%Y-%m-%d", timeinfo);

    String todayString = String(today);

    // Already sent today?
    if (getLastHeartbeatDate() == todayString)
        return;

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

    Serial.println("[HEARTBEAT] Heartbeat sent");
}