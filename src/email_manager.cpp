#include "email_manager.h"
#include "config.h"
#include "event_log.h"
#include "utils.h"
#include "secrets.h"

#include <ESP_Mail_Client.h>

void sendEmail(const String &subject, const String &body)
{
        Serial.println("================================");
    Serial.println("[EMAIL] SEND REQUEST");
    Serial.print("[EMAIL] Subject: ");
    Serial.println(subject);
    Serial.println("================================");
    
    if (!EMAIL_ENABLED)
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
    message.sender.name = "ESP32 Water Sensor";
    message.sender.email = AUTHOR_EMAIL;
    message.subject = subject;
    String recipients = RECIPIENT_EMAILS;

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
    body += "Status: WET\n";
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
