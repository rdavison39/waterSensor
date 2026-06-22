#include "email_manager.h"
#include "config.h"
#include "event_log.h"
#include "utils.h"
#include "secrets.h"
#include "settings.h"
#include "motion_sensor.h"
#include "water_sensor.h"
#include <WiFi.h>

#include <ESP_Mail_Client.h>

void sendEmail(const String &subject, const String &body)
{
    Serial.println("================================");
    Serial.println("[EMAIL] SEND REQUEST");
    Serial.print("[EMAIL] Subject: ");
    Serial.println(subject);
    Serial.println("================================");

    if (!getEmailEnabled())
    {
        Serial.println("EMAIL DISABLED");
        return;
    }

    SMTPSession smtp;
    ESP_Mail_Session session;

    session.server.host_name = SMTP_HOST;
    session.server.port = SMTP_PORT;

    session.login.email = AUTHOR_EMAIL;
    session.login.password = AUTHOR_PASSWORD;

    SMTP_Message message;
    message.sender.name = "Eldorado Cottage Monitor";
    message.sender.email = AUTHOR_EMAIL;
    message.subject = subject;
    String recipients = getRecipientEmails();

    int start = 0;
    while (start < recipients.length())
    {
        int comma = recipients.indexOf(',', start);

        String email;

        if (comma == -1)
        {
            email = recipients.substring(start);
            start = recipients.length();
        }
        else
        {
            email = recipients.substring(start, comma);
            start = comma + 1;
        }

        email.trim();

        if (email.length() > 0)
        {
            message.addRecipient(email.c_str(), email.c_str());
        }
    }
    message.text.content = body.c_str();

    Serial.println("Connecting to Gmail...");

    if (!smtp.connect(&session))
    {
        Serial.println("SMTP Connect Failed");
        return;
    }

    if (!MailClient.sendMail(&smtp, &message))
    {
        Serial.print("Send Error: ");
        Serial.println(smtp.errorReason());
    }
    else
    {
        Serial.println("Email Sent Successfully");
        emailCounter++;
        addEvent("Email sent: " + subject);
    }

    smtp.closeSession();
}

void sendAlarmEmail()
{
    lastAlarmTime = getCurrentTime();
    wetEmailCounter = 1;
    lastWetEmailTime = (unsigned long)time(nullptr);
    currentAlertIntervalIndex = 0;

    String body = "WATER DETECTED - ALERT #" + String(wetEmailCounter) + "\n\n";

    body += "Time: " + lastAlarmTime + "\n";
    body += "Status: WET\n\n";

    body += "Sensor 1: ";
    body += sensor1Detected ? "WET" : "DRY";
    body += "\n";

    body += "Sensor 2: ";
    body += sensor2Detected ? "WET" : "DRY";
    body += "\n\n";

    body += "This is the first alert. Further alerts will be sent on the configured intervals until the leak is fixed.";

    sendEmail("🚨 WATER ALARM DETECTED - Alert #" + String(wetEmailCounter), body);
}

void sendWetAlertEmail()
{
    wetEmailCounter++;
    lastWetEmailTime = (unsigned long)time(nullptr);
    if (currentAlertIntervalIndex < NUM_ALERT_INTERVALS - 1)
        currentAlertIntervalIndex++;

    String body = "WATER STILL DETECTED - ALERT #" + String(wetEmailCounter) + "\n\n";

    body += "Time: " + getCurrentTime() + "\n";
    body += "Status: WET (ongoing)\n";

    body += "Sensor 1: ";
    body += sensor1Detected ? "WET" : "DRY";
    body += "\n";

    body += "Sensor 2: ";
    body += sensor2Detected ? "WET" : "DRY";
    body += "\n";

    body += "Uptime: " + getUptime() + "\n";
    body += "Total Alerts This Session: " + String(wetEmailCounter) + "\n\n";

    body += "The water sensor is still detecting moisture. Please check for leaks immediately.";

    sendEmail("🚨 WATER ALERT #" + String(wetEmailCounter) + " - Still Wet", body);
    addEvent("Wet alert email #" + String(wetEmailCounter) + " sent");
}

void sendTestEmail()
{
    lastTestEmailTime = getCurrentTime();
    sendEmail("ESP32 Water Sensor Test", "This is a test email sent at " + lastTestEmailTime);
}

void sendStatusEmail()
{
    String status = waterDetected ? "WET - ALARM ACTIVE" : "DRY - NORMAL";

    String body = "=== ESP32 Water Sensor Status Report ===\n\n";
    body += "Firmware: v" + String(FIRMWARE_VERSION) + "\n";
    body += "Current Status: " + status + "\n";
    body += "Sensor 1: ";
body += sensor1Detected ? "WET" : "DRY";
body += "\n";

body += "Sensor 2: ";
body += sensor2Detected ? "WET" : "DRY";
body += "\n";
    body += "Report Time: " + getCurrentTime() + "\n";
    body += "Uptime: " + getUptime() + "\n";
    body += "\n--- Counters ---\n";
    body += "Alarms Triggered: " + String(alarmCounter) + "\n";
    body += "Emails Sent: " + String(emailCounter) + "\n";
    body += "WiFi Signal: " + String(WiFi.RSSI()) + " dBm\n";
    body += "\n--- Last Events ---\n";
    body += "Last Alarm: " + lastAlarmTime + "\n";
    body += "Last Test Email: " + lastTestEmailTime + "\n";
    body += "\nSystem is operational and monitoring for water leaks.";

    lastStatusEmailDate = getCurrentDate();
    sendEmail("ESP32 Water Sensor - Status Report", body);
    addEvent("Status email sent");
}
void sendStartupEmail()
{
    String body;

    body += "ESP32 Startup Report\n\n";

    body += "===== System Information =====\n\n";

    body += "Time: " + getCurrentTime() + "\n";
    body += "Firmware: v" + String(FIRMWARE_VERSION) + "\n";
    body += "IP Address: " + WiFi.localIP().toString() + "\n";
    body += "WiFi Signal: " + String(WiFi.RSSI()) + " dBm\n\n";

    body += "===== Configuration =====\n\n";

    body += "Email Notifications: ";
    body += getEmailEnabled() ? "ENABLED" : "DISABLED";
    body += "\n\n";

    body += "Recipients:\n";
    body += getRecipientEmails();
    body += "\n\n";

    body += "Heartbeat Interval: ";
    body += String(getHeartbeatIntervalDays());
    body += " day(s)\n";

    body += "Motion Email Cooldown: ";
    body += String(getMotionEmailCooldownMinutes());
    body += " minute(s)\n\n";

    body += "===== Current Status =====\n\n";

    body += "Overall Water Status: ";
    body += waterDetected ? "WET" : "DRY";
    body += "\n";

    body += "Sensor 1: ";
    body += sensor1Detected ? "WET" : "DRY";
    body += "\n";

    body += "Sensor 2: ";
    body += sensor2Detected ? "WET" : "DRY";
    body += "\n";

    body += "Motion Status: ";
    body += isMotionDetected() ? "ACTIVE" : "IDLE";
    body += "\n\n";

    body += "===== Counters =====\n\n";

    body += "Water Alarms: ";
    body += String(alarmCounter);
    body += "\n";

    body += "Motion Events: ";
    body += String(motionCount);
    body += "\n";

    body += "Emails Sent: ";
    body += String(emailCounter);
    body += "\n\n";

    body += "===== Last Activity =====\n\n";

    body += "Last Alarm: " + lastAlarmTime + "\n";
    body += "Last Motion: " + lastMotionTime + "\n";
    body += "Last Motion Email: " + lastMotionEmailTimestamp + "\n";
    body += "Last Test Email: " + lastTestEmailTime + "\n";
    body += "Last Status Email: " + lastStatusEmailDate + "\n\n";

    body += "System initialization completed successfully.";

    sendEmail("ESP32 Started Successfully", body);

    addEvent("Startup email sent");
}