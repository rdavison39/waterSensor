#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <ESP_Mail_Client.h>
#include <time.h>

#include "secrets.h"

WebServer server(80);

bool waterDetected = false;
bool alarmEmailSent = false;

// Sensor pin definitions
#define WATER_PIN 27
#define MOTION_PIN 22

// Motion sensor state tracking
bool motionState = false;

unsigned long bootMillis;

#define SMTP_HOST "smtp.gmail.com"
#define SMTP_PORT 465

const bool EMAIL_ENABLED = false;
#define FIRMWARE_VERSION "2.0.0"

// Time and tracking variables
String lastAlarmTime = "Never";
String lastTestEmailTime = "Never";
String lastStatusEmailDate = "Never";
int alarmCounter = 0;
int emailCounter = 0;

// Wet alert variables (4-hour repeating alerts)
unsigned long lastWetEmailTime = 0;
int wetEmailCounter = 0;
#define WET_EMAIL_INTERVAL 14400  // 4 hours in seconds

// Status email days (1st, 8th, 15th, 22nd, 29th)
const int STATUS_EMAIL_DAYS[] = {1, 8, 15, 22, 29};
#define NUM_STATUS_DAYS 5

// Event logging (last 10 events)
#define MAX_EVENTS 10
String events[MAX_EVENTS];
int eventIndex = 0;

// Function to add event to log
void addEvent(String eventText)
{
    events[eventIndex] = eventText;
    eventIndex = (eventIndex + 1) % MAX_EVENTS;
}

// Function to get current time as string
String getCurrentTime()
{
    time_t now = time(nullptr);
    struct tm* timeinfo = localtime(&now);
    
    char buffer[64];
    strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", timeinfo);
    
    return String(buffer);
}

// Function to get current date as YYYY-MM-DD string
String getCurrentDate()
{
    time_t now = time(nullptr);
    struct tm* timeinfo = localtime(&now);
    
    char buffer[16];
    strftime(buffer, sizeof(buffer), "%Y-%m-%d", timeinfo);
    
    return String(buffer);
}

// Function to get day of month
int getDayOfMonth()
{
    time_t now = time(nullptr);
    struct tm* timeinfo = localtime(&now);
    return timeinfo->tm_mday;
}

// Function to check if today is a status email day
bool isStatusEmailDay()
{
    int dayOfMonth = getDayOfMonth();
    for (int i = 0; i < NUM_STATUS_DAYS; i++)
    {
        if (dayOfMonth == STATUS_EMAIL_DAYS[i])
        {
            return true;
        }
    }
    return false;
}

// Function to get WiFi signal color coding
String getWiFiColor()
{
    int rssi = WiFi.RSSI();
    if (rssi >= -50) return "#27ae60";      // Excellent - Green
    if (rssi >= -60) return "#2ecc71";      // Very Good - Light Green
    if (rssi >= -70) return "#f39c12";      // Good - Orange
    if (rssi >= -80) return "#e67e22";      // Fair - Dark Orange
    return "#e74c3c";                       // Poor - Red
}

// Function to get seconds until next wet email
unsigned long getSecondsUntilNextWetEmail()
{
    if (!waterDetected)
    {
        return 0;
    }
    
    if (lastWetEmailTime == 0)
    {
        return 0;  // First email should be sent immediately
    }
    
    time_t now = time(nullptr);
    unsigned long secondsSinceLast = (unsigned long)now - lastWetEmailTime;
    
    if (secondsSinceLast >= WET_EMAIL_INTERVAL)
    {
        return 0;  // Ready to send now
    }
    
    return WET_EMAIL_INTERVAL - secondsSinceLast;
}

// Function to format seconds as readable string (H:MM:SS)
String formatSecondsToTime(unsigned long seconds)
{
    unsigned long hours = seconds / 3600;
    unsigned long minutes = (seconds % 3600) / 60;
    unsigned long secs = seconds % 60;
    
    char buffer[16];
    sprintf(buffer, "%lu:%02lu:%02lu", hours, minutes, secs);
    return String(buffer);
}


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
    if (!EMAIL_ENABLED)
    {
        Serial.println("EMAIL DISABLED");
        return;
    }

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
        
        emailCounter++;
        addEvent("Email sent: " + subject);
    }

    smtp.closeSession();
}


void sendAlarmEmail()
{
    lastAlarmTime = getCurrentTime();
    wetEmailCounter = 1;  // First wet alert
    lastWetEmailTime = (unsigned long)time(nullptr);
    
    String body = "WATER DETECTED - ALERT #" + String(wetEmailCounter) + "\n\n";
    body += "Time: " + lastAlarmTime + "\n";
    body += "Status: WET\n";
    body += "This is the first alert. Further alerts will be sent every 4 hours until the leak is fixed.";
    
    sendEmail(
        "🚨 WATER ALARM DETECTED - Alert #" + String(wetEmailCounter),
        body);
}


void sendWetAlertEmail()
{
    wetEmailCounter++;
    lastWetEmailTime = (unsigned long)time(nullptr);
    
    String body = "WATER STILL DETECTED - ALERT #" + String(wetEmailCounter) + "\n\n";
    body += "Time: " + getCurrentTime() + "\n";
    body += "Status: WET (ongoing)\n";
    body += "Uptime: " + getUptime() + "\n";
    body += "Total Alarms This Session: " + String(wetEmailCounter) + "\n\n";
    body += "The water sensor is still detecting moisture.\n";
    body += "Please check for leaks immediately.";
    
    sendEmail(
        "🚨 WATER ALERT #" + String(wetEmailCounter) + " - Still Wet",
        body);
    addEvent("Wet alert email #" + String(wetEmailCounter) + " sent");
}


void sendTestEmail()
{
    lastTestEmailTime = getCurrentTime();
    sendEmail(
        "ESP32 Water Sensor Test",
        "This is a test email sent at " + lastTestEmailTime);
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
    sendEmail(
        "ESP32 Water Sensor - Weekly Status Report",
        body);
    addEvent("Weekly status email sent");
}


void triggerAlarm()
{
    waterDetected = true;

    alarmCounter++;

    if (!alarmEmailSent)
    {
        sendAlarmEmail();
        addEvent("Water alarm triggered!");

        alarmEmailSent = true;
    }
}


void triggerDry()
{
    waterDetected = false;
    alarmEmailSent = false;
    wetEmailCounter = 0;
    lastWetEmailTime = 0;

    Serial.println("SENSOR DRY - ALARM CLEARED");
    addEvent("Sensor returned to DRY");
}


void handleRoot()
{
    String statusColor =
        waterDetected
            ? "#e74c3c"
            : "#27ae60";

    String statusText =
        waterDetected
            ? "WET"
            : "DRY";

    String wifiColor = getWiFiColor();

    String html;

    html += "<!DOCTYPE html>";
    html += "<html>";
    html += "<head>";

    html += "<meta charset='utf-8'>";
    html += "<meta name='viewport' content='width=device-width,initial-scale=1'>";
    html += "<meta http-equiv='refresh' content='30'>";

    html += "<title>ESP32 Water Sensor</title>";

    html += "<style>";

    // Dark theme styles
    html += "body{background:#1a1a1a;color:#ecf0f1;font-family:'Segoe UI',Arial,sans-serif;margin:0;padding:0;}";
    html += ".container{max-width:1200px;margin:auto;padding:20px;}";
    html += ".header{text-align:center;margin-bottom:30px;border-bottom:2px solid #2c3e50;padding-bottom:20px;}";
    html += ".title{font-size:2.5em;margin:0;color:#3498db;}";
    html += ".version{font-size:0.9em;color:#95a5a6;margin-top:5px;}";

    html += ".status-container{display:flex;flex-direction:column;align-items:center;margin-bottom:30px;}";
    html += ".status-box{background:";
    html += statusColor;
    html += ";padding:40px 60px;border-radius:15px;box-shadow:0 8px 20px rgba(0,0,0,.5);text-align:center;}";
    html += ".status-text{font-size:3em;font-weight:bold;color:white;margin:0;}";
    html += ".water-droplet{width:80px;height:80px;margin:20px auto;}";

    html += ".grid{display:grid;grid-template-columns:repeat(auto-fit,minmax(280px,1fr));gap:20px;margin-bottom:30px;}";
    html += ".card{background:#2c3e50;border-radius:12px;padding:25px;box-shadow:0 4px 15px rgba(0,0,0,.3);border-left:5px solid #3498db;}";
    html += ".card-title{font-size:0.95em;color:#95a5a6;text-transform:uppercase;letter-spacing:1px;margin-bottom:10px;}";
    html += ".card-value{font-size:1.8em;font-weight:bold;color:#ecf0f1;word-break:break-all;}";

    html += ".wifi-indicator{display:inline-block;padding:8px 12px;border-radius:6px;background:";
    html += wifiColor;
    html += ";color:white;font-weight:bold;margin-top:8px;}";

    html += ".events-container{background:#2c3e50;border-radius:12px;padding:25px;box-shadow:0 4px 15px rgba(0,0,0,.3);margin-bottom:30px;}";
    html += ".events-title{font-size:1.2em;color:#3498db;margin-top:0;margin-bottom:15px;text-transform:uppercase;}";
    html += ".event-list{background:#1a1a1a;border-radius:8px;padding:15px;max-height:250px;overflow-y:auto;}";
    html += ".event-item{padding:10px;border-bottom:1px solid #34495e;font-size:0.95em;color:#bdc3c7;}";
    html += ".event-item:last-child{border-bottom:none;}";

    html += ".button-container{display:flex;flex-wrap:wrap;gap:15px;justify-content:center;margin-bottom:20px;}";
    html += ".btn{display:inline-block;padding:15px 25px;border-radius:8px;color:white;text-decoration:none;font-weight:bold;border:none;cursor:pointer;transition:all 0.3s;}";
    html += ".btn-alarm{background:#e74c3c;}";
    html += ".btn-alarm:hover{background:#c0392b;transform:scale(1.05);}";
    html += ".btn-reset{background:#27ae60;}";
    html += ".btn-reset:hover{background:#229954;transform:scale(1.05);}";
    html += ".btn-test{background:#3498db;}";
    html += ".btn-test:hover{background:#2980b9;transform:scale(1.05);}";

    html += "@media(max-width:768px){.grid{grid-template-columns:1fr;}.title{font-size:1.8em;}.status-box{padding:30px 40px;}.btn{padding:12px 20px;font-size:0.9em;}}";

    html += "</style>";

    html += "</head>";
    html += "<body>";

    html += "<div class='container'>";

    // Header
    html += "<div class='header'>";
    html += "<h1 class='title'>💧 Water Sensor Dashboard</h1>";
    html += "<p class='version'>Firmware v" + String(FIRMWARE_VERSION) + "</p>";
    html += "</div>";

    // Status Box
    html += "<div class='status-container'>";
    html += "<div class='status-box'>";
    html += "<svg class='water-droplet' viewBox='0 0 24 24' fill='white'>";
    html += "<path d='M12 2.69l5.66 5.66a8 8 0 1 1-11.32 0z'/>";
    html += "</svg>";
    html += "<p class='status-text'>" + statusText + "</p>";
    html += "</div>";
    html += "</div>";

    // Info Grid
    html += "<div class='grid'>";

    // Current Time
    html += "<div class='card'>";
    html += "<div class='card-title'>Current Time</div>";
    html += "<div class='card-value'>" + getCurrentTime() + "</div>";
    html += "</div>";

    // Uptime
    html += "<div class='card'>";
    html += "<div class='card-title'>Uptime</div>";
    html += "<div class='card-value'>" + getUptime() + "</div>";
    html += "</div>";

    // WiFi Signal
    html += "<div class='card'>";
    html += "<div class='card-title'>WiFi Signal</div>";
    html += "<div class='card-value'>" + String(WiFi.RSSI()) + " dBm</div>";
    html += "<div class='wifi-indicator'>" + String(WiFi.RSSI()) + " dBm</div>";
    html += "</div>";

    // Alarm Counter
    html += "<div class='card'>";
    html += "<div class='card-title'>Alarm Counter</div>";
    html += "<div class='card-value'>" + String(alarmCounter) + "</div>";
    html += "</div>";

    // Email Counter
    html += "<div class='card'>";
    html += "<div class='card-title'>Email Counter</div>";
    html += "<div class='card-value'>" + String(emailCounter) + "</div>";
    html += "</div>";

    // Last Alarm
    html += "<div class='card'>";
    html += "<div class='card-title'>Last Alarm</div>";
    html += "<div class='card-value'>" + lastAlarmTime + "</div>";
    html += "</div>";

    // Last Test Email
    html += "<div class='card'>";
    html += "<div class='card-title'>Last Test Email</div>";
    html += "<div class='card-value'>" + lastTestEmailTime + "</div>";
    html += "</div>";

    // Last Status Email
    html += "<div class='card'>";
    html += "<div class='card-title'>Last Status Email</div>";
    html += "<div class='card-value'>" + lastStatusEmailDate + "</div>";
    html += "</div>";

    // Alert Recipient
    html += "<div class='card'>";
    html += "<div class='card-title'>Alert Email To</div>";
    html += "<div class='card-value'>" + String(RECIPIENT_EMAIL) + "</div>";
    html += "</div>";

    // Wet Alert Status (if sensor is wet)
    if (waterDetected)
    {
        unsigned long secondsUntilNext = getSecondsUntilNextWetEmail();
        html += "<div class='card'>";
        html += "<div class='card-title'>Wet Alerts Sent</div>";
        html += "<div class='card-value'>" + String(wetEmailCounter) + "</div>";
        html += "</div>";

        html += "<div class='card'>";
        html += "<div class='card-title'>Next Alert In</div>";
        if (secondsUntilNext == 0)
        {
            html += "<div class='card-value' style='color:#f39c12;'>READY NOW</div>";
        }
        else
        {
            html += "<div class='card-value'>" + formatSecondsToTime(secondsUntilNext) + "</div>";
        }
        html += "</div>";
    }

    // Email Status
    html += "<div class='card'>";
    html += "<div class='card-title'>Email Status</div>";
    html += "<div class='card-value'>" + String(alarmEmailSent ? "SENT" : "READY") + "</div>";
    html += "</div>";

    html += "</div>";

    // Events Log
    html += "<div class='events-container'>";
    html += "<h2 class='events-title'>Last 10 Events</h2>";
    html += "<div class='event-list'>";

    // Display events in reverse order (newest first)
    for (int i = 0; i < MAX_EVENTS; i++)
    {
        int index = (eventIndex - 1 - i + MAX_EVENTS) % MAX_EVENTS;
        if (events[index].length() > 0)
        {
            html += "<div class='event-item'>• " + events[index] + "</div>";
        }
    }

    html += "</div>";
    html += "</div>";

    // Buttons
    html += "<div class='button-container'>";
    html += "<a class='btn btn-alarm' href='/wet'>Simulate Sensor Wet</a>";
    html += "<a class='btn btn-reset' href='/dry'>Simulate Sensor Dry</a>";
    html += "<a class='btn btn-test' href='/testemail'>Send Test Email</a>";
    html += "</div>";

    html += "</div>";
    html += "</body>";
    html += "</html>";

    server.send(
        200,
        "text/html",
        html);
}


void handleSimulateWet()
{
    Serial.println(
        "SIMULATED WATER DETECTED");

    triggerAlarm();
    addEvent("Sensor wet simulated");

    server.sendHeader(
        "Location",
        "/");

    server.send(
        302,
        "text/plain",
        "");
}


void handleSimulateDry()
{
    Serial.println("SIMULATED SENSOR DRY");

    triggerDry();

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
    addEvent("Test email sent");

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

    // Configure NTP time with Eastern timezone
    // EST/EDT = UTC-5 (standard) or UTC-4 (daylight)
    configTime(-5 * 3600, 3600, "pool.ntp.org", "time.nist.gov");
    
    Serial.println("Waiting for NTP time sync...");
    delay(2000);

    // Add initial boot event
    addEvent("System started - v" + String(FIRMWARE_VERSION));

    server.on(
        "/",
        handleRoot);

    server.on(
        "/wet",
        handleSimulateWet);

    server.on(
        "/dry",
        handleSimulateDry);

    server.on(
        "/testemail",
        handleTestEmail);

    server.begin();

    Serial.println(
        "Web Server Started");

    // Configure sensor pins
    pinMode(WATER_PIN, INPUT_PULLUP);
    pinMode(MOTION_PIN, INPUT);
    Serial.println("Sensor pins configured: WATER_PIN=27, MOTION_PIN=22");
}


void loop()
{
    server.handleClient();
    
    // Check if it's a status email day and we haven't sent today
    if (isStatusEmailDay() && lastStatusEmailDate != getCurrentDate())
    {
        sendStatusEmail();
    }
    
    // Check if sensor is wet and it's time to send another alert
    if (waterDetected && lastWetEmailTime > 0)
    {
        time_t now = time(nullptr);
        unsigned long secondsSinceLast = (unsigned long)now - lastWetEmailTime;
        
        if (secondsSinceLast >= WET_EMAIL_INTERVAL)
        {
            sendWetAlertEmail();
        }
    }

    // Read hardware sensors
    int waterStateRaw = digitalRead(WATER_PIN);
    // Treat HIGH as wet; flip logic here if your sensor is active-low
    if (waterStateRaw == LOW && !waterDetected)
    {
        triggerAlarm();
        addEvent("Water sensor: WET (hardware)");
    }
    else if (waterStateRaw == HIGH && waterDetected)
    {
        triggerDry();
        addEvent("Water sensor: DRY (hardware)");
    }

    int motionRaw = digitalRead(MOTION_PIN);
    if (motionRaw == HIGH && !motionState)
    {
        motionState = true;
        addEvent("Motion detected");
    }
    else if (motionRaw == LOW && motionState)
    {
        motionState = false;
        addEvent("Motion ended");
    }
}