#include "motion_sensor.h"
#include "config.h"
#include "event_log.h"
#include "utils.h"
#include "email_manager.h"

#include <WiFi.h>

void setupMotionSensor()
{
    pinMode(MOTION_PIN, INPUT);
}

void loopMotionSensor()
{
    static int lastRawState = -1;
    static unsigned long lastMotionEmailTime = 0;

    int motionRaw = digitalRead(MOTION_PIN);

    // Debug raw pin changes
    if (motionRaw != lastRawState)
    {
        Serial.print("[MOTION] Raw Pin Changed: ");
        Serial.println(motionRaw);

        lastRawState = motionRaw;
    }

    if (motionRaw == HIGH && !motionState)
    {
        motionState = true;

        motionCount++;

        lastMotionTime = getCurrentTime();

        Serial.println("[MOTION] Motion detected");
        Serial.print("[MOTION] Count: ");
        Serial.println(motionCount);
        Serial.print("[MOTION] Time: ");
        Serial.println(lastMotionTime);

        addEvent("Motion detected");

        if (EMAIL_ENABLED)
        {
            unsigned long now = millis();

            // 5 minute cooldown between motion emails
            if ((now - lastMotionEmailTime) > 300000UL)
            {
                Serial.println("[MOTION] Sending motion email");

                String subject = "ESP32 Motion Detected";

                String body;
                body += "Motion detected by PIR sensor\n\n";
                body += "Time: " + lastMotionTime + "\n";
                body += "Motion Count: " + String(motionCount) + "\n";
                body += "IP Address: " + WiFi.localIP().toString() + "\n";
                body += "Status: ACTIVE\n";

                sendEmail(subject, body);

                lastMotionEmailTime = now;
            }
            else
            {
                Serial.println("[MOTION] Email suppressed (cooldown active)");
            }
        }
        else
        {
            Serial.println("[MOTION] Email disabled");
        }
    }
    else if (motionRaw == LOW && motionState)
    {
        motionState = false;

        Serial.println("[MOTION] Motion ended");

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