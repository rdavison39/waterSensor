#include "web_ui.h"
#include "config.h"
#include "utils.h"
#include "event_log.h"
#include "email_manager.h"
#include "water_sensor.h"
#include "secrets.h"
#include "motion_sensor.h"

#include <WiFi.h>

void handleRoot()
{
    String statusColor = waterDetected ? "#e74c3c" : "#27ae60";
    String statusText = waterDetected ? "WET" : "DRY";
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
    html += "body{background:#1a1a1a;color:#ecf0f1;font-family:'Segoe UI',Arial,sans-serif;margin:0;padding:0;}";
    html += ".container{max-width:1200px;margin:auto;padding:20px;}";
    html += ".header{text-align:center;margin-bottom:30px;border-bottom:2px solid #2c3e50;padding-bottom:20px;}";
    html += ".title{font-size:2.5em;margin:0;color:#3498db;}";
    html += ".version{font-size:0.9em;color:#95a5a6;margin-top:5px;}";
    html += ".status-container{display:flex;flex-direction:column;align-items:center;margin-bottom:30px;}";
    html += ".status-box{background:" + statusColor + ";padding:40px 60px;border-radius:15px;box-shadow:0 8px 20px rgba(0,0,0,.5);text-align:center;}";
    html += ".status-text{font-size:3em;font-weight:bold;color:white;margin:0;}";
    html += ".water-droplet{width:80px;height:80px;margin:20px auto;}";
    html += ".grid{display:grid;grid-template-columns:repeat(auto-fit,minmax(280px,1fr));gap:20px;margin-bottom:30px;}";
    html += ".card{background:#2c3e50;border-radius:12px;padding:25px;box-shadow:0 4px 15px rgba(0,0,0,.3);border-left:5px solid #3498db;}";
    html += ".card-title{font-size:0.95em;color:#95a5a6;text-transform:uppercase;letter-spacing:1px;margin-bottom:10px;}";
    html += ".card-value{font-size:1.8em;font-weight:bold;color:#ecf0f1;word-break:break-all;}";
    html += ".wifi-indicator{display:inline-block;padding:8px 12px;border-radius:6px;background:" + wifiColor + ";color:white;font-weight:bold;margin-top:8px;}";
    html += ".events-container{background:#2c3e50;border-radius:12px;padding:25px;box-shadow:0 4px 15px rgba(0,0,0,.3);margin-bottom:30px;}";
    html += ".events-title{font-size:1.2em;color:#3498db;margin-top:0;margin-bottom:15px;text-transform:uppercase;}";
    html += ".event-list{background:#1a1a1a;border-radius:8px;padding:15px;max-height:250px;overflow-y:auto;}";
    html += ".event-item{padding:10px;border-bottom:1px solid #34495e;font-size:0.95em;color:#bdc3c7;}";
    html += ".event-item:last-child{border-bottom:none;}";
    html += ".button-container{display:flex;flex-wrap:wrap;gap:15px;justify-content:center;margin-bottom:20px;}";
    html += ".btn{display:inline-block;padding:15px 25px;border-radius:8px;color:white;text-decoration:none;font-weight:bold;border:none;cursor:pointer;transition:all 0.3s;}";
    html += ".btn-alarm{background:#e74c3c;}";
    html += ".btn-reset{background:#27ae60;}";
    html += ".btn-test{background:#3498db;}";
    html += "@media(max-width:768px){.grid{grid-template-columns:1fr;}.title{font-size:1.8em;}.status-box{padding:30px 40px;}.btn{padding:12px 20px;font-size:0.9em;}}";
    html += "</style>";
    html += "</head>";
    html += "<body>";
    html += "<div class='container'>";
    html += "<div class='header'>";
    html += "<h1 class='title'>💧 Water Sensor Dashboard</h1>";
    html += "<p class='version'>Firmware v" + String(FIRMWARE_VERSION) + "</p>";
    html += "</div>";
    html += "<div class='status-container'>";
    html += "<div class='status-box'>";
    html += "<svg class='water-droplet' viewBox='0 0 24 24' fill='white'>";
    html += "<path d='M12 2.69l5.66 5.66a8 8 0 1 1-11.32 0z'/>";
    html += "</svg>";
    html += "<p class='status-text'>" + statusText + "</p>";
    html += "</div>";
    html += "</div>";
    html += "<div class='grid'>";

    html += "<div class='card'><div class='card-title'>Current Time</div><div class='card-value'>" + getCurrentTime() + "</div></div>";
    html += "<div class='card'><div class='card-title'>Uptime</div><div class='card-value'>" + getUptime() + "</div></div>";
    html += "<div class='card'><div class='card-title'>WiFi Signal</div><div class='card-value'>" + String(WiFi.RSSI()) + " dBm</div><div class='wifi-indicator'>" + getWiFiQuality() + "</div></div>";
    html += "<div class='card'><div class='card-title'>Current Interval</div><div class='card-value'>" + String(ALERT_INTERVALS[currentAlertIntervalIndex] / 60) + " min</div></div>";
    html += "<div class='card'><div class='card-title'>Alerts Sent</div><div class='card-value'>" + String(emailCounter) + "</div></div>";
    html += "<div class='card'><div class='card-title'>Last Alarm</div><div class='card-value'>" + lastAlarmTime + "</div></div>";
    html += "<div class='card'><div class='card-title'>Last Test Email</div><div class='card-value'>" + lastTestEmailTime + "</div></div>";
    html += "<div class='card'><div class='card-title'>Last Status Email</div><div class='card-value'>" + lastStatusEmailDate + "</div></div>";
    html += "<div class='card'><div class='card-title'>Alert Email To</div><div class='card-value'>" + String(RECIPIENT_EMAIL) + "</div></div>";
    html += "<div class='card'><div class='card-title'>Motion Status</div><div class='card-value'>";
html += isMotionDetected() ? "ACTIVE" : "IDLE";
html += "</div></div>";

html += "<div class='card'><div class='card-title'>Motion Count</div><div class='card-value'>";
html += String(getMotionCount());
html += "</div></div>";

html += "<div class='card'><div class='card-title'>Last Motion</div><div class='card-value'>";
html += getLastMotionTime();
html += "</div></div>";

    if (waterDetected)
    {
        unsigned long secondsUntilNext = getSecondsUntilNextWetEmail();
        html += "<div class='card'><div class='card-title'>Wet Alerts Sent</div><div class='card-value'>" + String(wetEmailCounter) + "</div></div>";
        html += "<div class='card'><div class='card-title'>Next Alert In</div>";
        if (secondsUntilNext == 0)
            html += "<div class='card-value' style='color:#f39c12;'>READY NOW</div>";
        else
            html += "<div class='card-value'>" + formatSecondsToTime(secondsUntilNext) + "</div>";
        html += "</div>";
    }

    html += "<div class='card'><div class='card-title'>Email Status</div><div class='card-value'>" + String(EMAIL_ENABLED ? (alarmEmailSent ? "SENT" : "READY") : "DISABLED") + "</div></div>";

    html += "</div>"; // grid

    html += "<div class='events-container'><h2 class='events-title'>Last 10 Events</h2><div class='event-list'>";
    for (int i = 0; i < getEventCapacity(); i++)
    {
        String e = getEvent(i);
        if (e.length() > 0)
            html += "<div class='event-item'>• " + e + "</div>";
    }
    html += "</div></div>";

    html += "<div class='button-container'>";
    html += "<a class='btn btn-alarm' href='/wet'>Simulate Sensor Wet</a>";
    html += "<a class='btn btn-reset' href='/dry'>Simulate Sensor Dry</a>";
    html += "<a class='btn btn-test' href='/testemail'>Send Test Email</a>";
    html += "</div>";

    html += "</div></body></html>";

    server.send(200, "text/html", html);
}

void handleSimulateWet()
{
    Serial.println("SIMULATED WATER DETECTED");
    triggerAlarm();
    addEvent("Sensor wet simulated");
    server.sendHeader("Location", "/");
    server.send(302, "text/plain", "");
}

void handleSimulateDry()
{
    Serial.println("SIMULATED SENSOR DRY");
    triggerDry();
    server.sendHeader("Location", "/");
    server.send(302, "text/plain", "");
}

void handleTestEmail()
{
    Serial.println("Sending Test Email");
    sendTestEmail();
    addEvent("Test email sent");
    server.sendHeader("Location", "/");
    server.send(302, "text/plain", "");
}

void setupWebUI()
{
    server.on("/", handleRoot);
    server.on("/wet", handleSimulateWet);
    server.on("/dry", handleSimulateDry);
    server.on("/testemail", handleTestEmail);
}
