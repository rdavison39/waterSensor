#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <ESP_Mail_Client.h>

#include "secrets.h"

WebServer server(80);

bool waterDetected = false;
bool alarmEmailSent = false;

unsigned long bootMillis;

String lastAlarmTime = "Never";

#define SMTP_HOST "smtp.gmail.com"
#define SMTP_PORT 465


String getUptime()
{
    unsigned long seconds =
        (millis() - bootMillis) / 1000;

    unsigned long days =
        seconds / 86400;

    seconds %= 86400;

    unsigned long hours =
        seconds / 3600;

    seconds %= 3600;

    unsigned long minutes =
        seconds / 60;

    seconds %= 60;

    char buffer[64];

    sprintf(
        buffer,
        "%lu d %lu h %lu m %lu s",
        days,
        hours,
        minutes,
        seconds);

    return String(buffer);
}


void sendEmail(
    String subject,
    String body)
{
    SMTPSession smtp;

    ESP_Mail_Session session;

    session.server.host_name =
        SMTP_HOST;

    session.server.port =
        SMTP_PORT;

    session.login.email =
        AUTHOR_EMAIL;

    session.login.password =
        AUTHOR_PASSWORD;

    SMTP_Message message;

    message.sender.name =
        "ESP32 Water Sensor";

    message.sender.email =
        AUTHOR_EMAIL;

    message.subject =
        subject;

    message.addRecipient(
        "Ron",
        RECIPIENT_EMAIL);

    message.text.content =
        body.c_str();

    Serial.println("Connecting to Gmail...");

    if (!smtp.connect(&session))
    {
        Serial.println("SMTP Connect Failed");
        return;
    }

    if (!MailClient.sendMail(
            &smtp,
            &message))
    {
        Serial.print("Send Error: ");
        Serial.println(
            smtp.errorReason());
    }
    else
    {
        Serial.println(
            "Email Sent Successfully");
    }

    smtp.closeSession();
}


void sendAlarmEmail()
{
    sendEmail(
        "WATER ALARM DETECTED",
        "ESP32 detected water.");
}


void sendTestEmail()
{
    sendEmail(
        "ESP32 Water Sensor Test",
        "This is a test email.");
}


void triggerAlarm()
{
    waterDetected = true;

    lastAlarmTime =
        getUptime();

    if (!alarmEmailSent)
    {
        sendAlarmEmail();

        alarmEmailSent = true;
    }
}


void handleRoot()
{
    String statusColor =
        waterDetected
            ? "#dc3545"
            : "#28a745";

    String statusText =
        waterDetected
            ? "WET"
            : "DRY";

    String html;

    html += "<!DOCTYPE html>";
    html += "<html>";
    html += "<head>";

    html += "<meta charset='utf-8'>";
    html += "<meta name='viewport' content='width=device-width,initial-scale=1'>";

    html += "<style>";

    html += "body{background:#f3f5f7;font-family:Arial;margin:0;padding:0;}";

    html += ".container{max-width:900px;margin:auto;padding:20px;}";

    html += ".card{background:white;border-radius:12px;padding:25px;box-shadow:0 3px 12px rgba(0,0,0,.15);}";

    html += ".title{text-align:center;}";

    html += ".status{font-size:42px;font-weight:bold;text-align:center;color:";
    html += statusColor;
    html += ";}";

    html += ".grid{display:grid;grid-template-columns:1fr 1fr;gap:12px;}";

    html += ".tile{background:#f7f7f7;padding:15px;border-radius:8px;}";

    html += ".btn{display:inline-block;padding:12px 18px;margin:5px;border-radius:6px;color:white;text-decoration:none;font-weight:bold;}";

    html += ".red{background:#dc3545;}";
    html += ".green{background:#28a745;}";
    html += ".blue{background:#007bff;}";

    html += "</style>";

    html += "</head>";
    html += "<body>";

    html += "<div class='container'>";
    html += "<div class='card'>";

    html += "<h1 class='title'>💧 ESP32 Water Sensor</h1>";

    html += "<div class='status'>";
    html += statusText;
    html += "</div>";

    html += "<br>";

    html += "<div class='grid'>";

    html += "<div class='tile'><b>Uptime</b><br>";
    html += getUptime();
    html += "</div>";

    html += "<div class='tile'><b>WiFi Signal</b><br>";
    html += String(WiFi.RSSI());
    html += " dBm</div>";

    html += "<div class='tile'><b>Last Alarm</b><br>";
    html += lastAlarmTime;
    html += "</div>";

    html += "<div class='tile'><b>Email Sent</b><br>";
    html += alarmEmailSent ? "YES" : "NO";
    html += "</div>";

    html += "</div>";

    html += "<br>";

    html += "<a class='btn red' href='/alarm'>Simulate Alarm</a>";
    html += "<a class='btn green' href='/reset'>Reset Alarm</a>";
    html += "<a class='btn blue' href='/testemail'>Send Test Email</a>";

    html += "</div>";
    html += "</div>";

    html += "</body>";
    html += "</html>";

    server.send(
        200,
        "text/html",
        html);
}


void handleAlarm()
{
    Serial.println(
        "SIMULATED WATER DETECTED");

    triggerAlarm();

    server.sendHeader(
        "Location",
        "/");

    server.send(
        302,
        "text/plain",
        "");
}


void handleReset()
{
    waterDetected = false;

    alarmEmailSent = false;

    Serial.println(
        "ALARM RESET");

    server.sendHeader(
        "Location",
        "/");

    server.send(
        302,
        "text/plain",
        "");
}


void handleTestEmail()
{
    Serial.println(
        "Sending Test Email");

    sendTestEmail();

    server.sendHeader(
        "Location",
        "/");

    server.send(
        302,
        "text/plain",
        "");
}


void setup()
{
    bootMillis =
        millis();

    Serial.begin(115200);

    WiFi.begin(
        WIFI_SSID,
        WIFI_PASSWORD);

    Serial.println();
    Serial.println(
        "Connecting to WiFi...");

    while (
        WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.print(".");
    }

    Serial.println();
    Serial.println(
        "WiFi Connected");

    Serial.print(
        "IP Address: ");

    Serial.println(
        WiFi.localIP());

    server.on(
        "/",
        handleRoot);

    server.on(
        "/alarm",
        handleAlarm);

    server.on(
        "/reset",
        handleReset);

    server.on(
        "/testemail",
        handleTestEmail);

    server.begin();

    Serial.println(
        "Web Server Started");
}


void loop()
{
    server.handleClient();
}