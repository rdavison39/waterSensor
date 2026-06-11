#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <ESP_Mail_Client.h>

const char* ssid = "Eldorado";
const char* password = "Eldorado!";

WebServer server(80);

bool waterDetected = false;

#define SMTP_HOST "smtp.gmail.com"
#define SMTP_PORT 465

#define AUTHOR_EMAIL "ronaldjdavison@gmail.com"
#define AUTHOR_PASSWORD "abdk khnf rdbu ywjf"

#define RECIPIENT_EMAIL "ronaldjdavison@hotmail.com"

void sendTestEmail()
{
    SMTPSession smtp;

    ESP_Mail_Session session;

    session.server.host_name = SMTP_HOST;
    session.server.port = SMTP_PORT;

    session.login.email = AUTHOR_EMAIL;
    session.login.password = AUTHOR_PASSWORD;

    SMTP_Message message;

    message.sender.name = "ESP32 Water Sensor";
    message.sender.email = AUTHOR_EMAIL;

    message.subject = "ESP32 Water Sensor Test";

    message.addRecipient("Ron", RECIPIENT_EMAIL);

    message.text.content =
        "This is a test email from your ESP32 water sensor.";

    Serial.println("Connecting to Gmail...");

    if (!smtp.connect(&session))
    {
        Serial.println("SMTP Connect Failed");
        return;
    }

    Serial.println("Sending email...");

    if (!MailClient.sendMail(&smtp, &message))
    {
        Serial.print("Send Error: ");
        Serial.println(smtp.errorReason());
    }
    else
    {
        Serial.println("Email Sent Successfully");
    }

    smtp.closeSession();
}

void handleRoot()
{
    String html;

    html += "<html><body>";
    html += "<h1>Water Sensor Controller</h1>";

    html += "<p>Status: ";

    if (waterDetected)
        html += "WET";
    else
        html += "DRY";

    html += "</p>";

    html += "<p><a href='/alarm'>Simulate Alarm</a></p>";
    html += "<p><a href='/reset'>Reset Alarm</a></p>";
    html += "<p><a href='/testemail'>Send Test Email</a></p>";

    html += "</body></html>";

    server.send(200, "text/html", html);
}

void handleAlarm()
{
    waterDetected = true;

    Serial.println("SIMULATED WATER DETECTED");

    server.sendHeader("Location", "/");
    server.send(302, "text/plain", "");
}

void handleReset()
{
    waterDetected = false;

    Serial.println("ALARM RESET");

    server.sendHeader("Location", "/");
    server.send(302, "text/plain", "");
}

void handleTestEmail()
{
    Serial.println("Sending Test Email");

    sendTestEmail();

    server.sendHeader("Location", "/");
    server.send(302, "text/plain", "");
}

void setup()
{
    Serial.begin(115200);

    WiFi.begin(ssid, password);

    Serial.println();
    Serial.println("Connecting to WiFi...");

    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.print(".");
    }

    Serial.println();
    Serial.println("WiFi Connected");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());

    server.on("/", handleRoot);
    server.on("/alarm", handleAlarm);
    server.on("/reset", handleReset);
    server.on("/testemail", handleTestEmail);

    server.begin();

    Serial.println("Web Server Started");
}

void loop()
{
    server.handleClient();
}