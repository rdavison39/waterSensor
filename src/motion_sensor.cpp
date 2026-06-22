#include "motion_sensor.h"
#include "config.h"
#include "event_log.h"
#include "utils.h"
#include "email_manager.h"
#include "settings.h"

#include <WiFi.h>

static void motionLog(const String& msg)
{
    Serial.print("[");
    Serial.print(getCurrentTime());
    Serial.print("] ");
    Serial.println(msg);
}

void setupMotionSensor()
{
    pinMode(MOTION_PIN, INPUT);
}

void loopMotionSensor()
{
    static int lastRawState = -1;
    static unsigned long lastMotionEmailMillis = 0;
    static bool firstMotionEmail = true;

unsigned long cooldownMillis =
    getMotionEmailCooldownMinutes() * 60UL * 1000UL;

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

if (firstMotionEmail ||
    (now - lastMotionEmailMillis) > cooldownMillis)
        {
            if (EMAIL_ENABLED)
            {
                motionLog("[MOTION] Sending motion email");

                String subject = "ESP32 Motion Detected";

                String body;
                body += "Motion detected by PIR sensor\n\n";
                body += "Time: " + lastMotionTime + "\n";
                body += "Motion Count: " + String(motionCount) + "\n";
                body += "IP Address: " + WiFi.localIP().toString() + "\n";
                body += "Status: ACTIVE\n";

                Serial.println("[EMAIL] Source: Motion");
                sendEmail(subject, body);

                lastMotionEmailTimestamp = lastMotionTime;
            }
            else
            {
                motionLog("[MOTION] Email disabled");
            }

            firstMotionEmail = false;
            lastMotionEmailMillis = now;
        }
        else
        {
unsigned long remaining =
    (cooldownMillis - (now - lastMotionEmailMillis)) / 1000;

            Serial.print("[");
            Serial.print(getCurrentTime());
            Serial.print("] [MOTION] Email suppressed (cooldown active) - ");
            Serial.print(remaining);
            Serial.println(" seconds remaining");
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