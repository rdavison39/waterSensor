#include "motion_sensor.h"
#include "config.h"
#include "event_log.h"
#include "utils.h"
#include "email_manager.h"
#include "settings.h"
#include "water_sensor.h"

#include <WiFi.h>

static const unsigned long SECONDS_PER_MINUTE = 60UL;
static const unsigned long MILLIS_PER_SECOND = 1000UL;

static void motionLog(const String& msg)
{
    Serial.print("[");
    Serial.print(getCurrentTime());
    Serial.print("] ");
    Serial.println(msg);
}

static void resetMotionDailyCountersIfNeeded()
{
    String today = getCurrentDate();

    if (today.length() == 0)
        return;

    if (lastMotionResetDate == today)
        return;

    motionEmailsToday = 0;
    lastMotionResetDate = today;

    Serial.print("[MOTION] Daily motion email counter reset for ");
    Serial.println(today);
    addEvent("Motion email daily counter reset");
}

static unsigned long getMotionCooldownMillis()
{
    return (unsigned long)getMotionEmailCooldownMinutes() *
        SECONDS_PER_MINUTE *
        MILLIS_PER_SECOND;
}

static String buildMotionEmailBody(
    const String& currentMotionTime,
    const String& previousMotionEmailTime,
    unsigned long nextMotionEmailNumberToday)
{
    String body;

    body += "ESP32 Motion Alert\n";
    body += "==================\n\n";

    body += "Motion detected by PIR sensor\n";
    body += "Alert Time: " + currentMotionTime + "\n";
    body += "Firmware Version: " + String(FIRMWARE_VERSION) + "\n\n";

    body += "===== Current Status =====\n\n";

    body += "Water Status: ";
    body += waterDetected ? "WET\n" : "DRY\n";

    body += "Sensor 1: ";
    body += sensor1Detected ? "WET\n" : "DRY\n";

    body += "Sensor 2: ";
    body += sensor2Detected ? "WET\n" : "DRY\n";

    body += "Motion Status: ";
    body += motionState ? "ACTIVE\n" : "IDLE\n";

    body += "\n===== Network =====\n\n";

    body += "IP Address: " + WiFi.localIP().toString() + "\n";
    body += "WiFi Signal: " + String(WiFi.RSSI()) + " dBm\n";
    body += "WiFi Quality: " + getWiFiQuality() + "\n\n";

    body += "===== Runtime =====\n\n";

    body += "Uptime: " + getUptime() + "\n";
    body += "Email Notifications: ";
    body += getEmailEnabled() ? "ENABLED\n" : "DISABLED\n";
    body += "Motion Email Cooldown: ";
    body += String(getMotionEmailCooldownMinutes());
    body += " minute(s)\n\n";

    body += "===== Motion Counters =====\n\n";

    body += "Total Motion Detections: " + String(motionCount) + "\n";
    body += "Motion Emails Today: " + String(nextMotionEmailNumberToday) + "\n";
    body += "Detections During Previous Cooldown: ";
    body += String(motionDetectionsDuringCooldown);
    body += "\n";
    body += "Total Suppressed Motion Detections: ";
    body += String(suppressedMotionCount);
    body += "\n\n";

    body += "===== Last Activity =====\n\n";

    body += "Last Motion: " + lastMotionTime + "\n";
    body += "Previous Motion Email: " + previousMotionEmailTime + "\n";
    body += "Last Water Alarm: " + lastAlarmTime + "\n";
    body += "Last Test Email: " + lastTestEmailTime + "\n";
    body += "Last Status Email: " + lastStatusEmailDate + "\n\n";

    body += "System is operational and monitoring for motion and water leaks.";

    return body;
}

void setupMotionSensor()
{
    pinMode(MOTION_PIN, INPUT);
    resetMotionDailyCountersIfNeeded();
}

void loopMotionSensor()
{
    static int lastRawState = -1;

    resetMotionDailyCountersIfNeeded();

    unsigned long cooldownMillis = getMotionCooldownMillis();

    int motionRaw = digitalRead(MOTION_PIN);

    // Debug raw pin changes
    if (motionRaw != lastRawState)
    {
        Serial.print("[");
        Serial.print(getCurrentTime());
        Serial.print("] [MOTION] Raw Pin Changed: ");
        Serial.println(motionRaw);

        lastRawState = motionRaw;
    }

    if (motionRaw == HIGH && !motionState)
    {
        motionState = true;

        motionCount++;

        lastMotionTime = getCurrentTime();

        motionLog("[MOTION] Motion detected");

        Serial.print("[");
        Serial.print(getCurrentTime());
        Serial.print("] [MOTION] Count: ");
        Serial.println(motionCount);

        Serial.print("[");
        Serial.print(getCurrentTime());
        Serial.print("] [MOTION] Time: ");
        Serial.println(lastMotionTime);

        addEvent("Motion detected");

        unsigned long now = millis();

        if (lastMotionEmailMillis == 0 ||
            (now - lastMotionEmailMillis) >= cooldownMillis)
        {
            if (getEmailEnabled())
            {
                motionLog("[MOTION] Sending motion email");

                String previousMotionEmailTime = lastMotionEmailTimestamp;
                unsigned long nextMotionEmailNumberToday =
                    motionEmailsToday + 1;

                String subject = "ESP32 Motion Detected - Email #" +
                    String(nextMotionEmailNumberToday) +
                    " Today";

                String body = buildMotionEmailBody(
                    lastMotionTime,
                    previousMotionEmailTime,
                    nextMotionEmailNumberToday);

                Serial.println("[EMAIL] Source: Motion");
                sendEmail(subject, body);

                motionEmailsToday = nextMotionEmailNumberToday;
                lastMotionEmailTimestamp = lastMotionTime;
                lastMotionEmailDate = getCurrentDate();
                motionDetectionsDuringCooldown = 0;

                addEvent("Motion email #" +
                    String(motionEmailsToday) +
                    " sent today");
            }
            else
            {
                motionLog("[MOTION] Email disabled");
                addEvent("Motion email skipped - email disabled");
            }

            lastMotionEmailMillis = now;
        }
        else
        {
            motionDetectionsDuringCooldown++;
            suppressedMotionCount++;

            unsigned long remaining =
                (cooldownMillis - (now - lastMotionEmailMillis)) /
                MILLIS_PER_SECOND;

            Serial.print("[");
            Serial.print(getCurrentTime());
            Serial.print("] [MOTION] Email suppressed (cooldown active) - ");
            Serial.print(remaining);
            Serial.println(" seconds remaining");

            addEvent("Motion email suppressed - cooldown active");
        }
    }
    else if (motionRaw == LOW && motionState)
    {
        motionState = false;

        motionLog("[MOTION] Motion ended");

        addEvent("Motion ended");
    }
}

bool isMotionDetected()
{
    return motionState;
}

unsigned long getMotionCount()
{
    return motionCount;
}

String getLastMotionTime()
{
    return lastMotionTime;
}

String getLastMotionEmailTime()
{
    return lastMotionEmailTimestamp;
}

unsigned long getMotionEmailsToday()
{
    resetMotionDailyCountersIfNeeded();
    return motionEmailsToday;
}

unsigned long getSuppressedMotionCount()
{
    return suppressedMotionCount;
}

unsigned long getMotionDetectionsDuringCooldown()
{
    return motionDetectionsDuringCooldown;
}

unsigned long getMotionCooldownSecondsRemaining()
{
    if (lastMotionEmailMillis == 0)
        return 0;

    unsigned long cooldownMillis = getMotionCooldownMillis();
    unsigned long elapsed = millis() - lastMotionEmailMillis;

    if (elapsed >= cooldownMillis)
        return 0;

    return (cooldownMillis - elapsed) / MILLIS_PER_SECOND;
}
