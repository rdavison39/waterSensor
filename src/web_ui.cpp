#include "web_ui.h"
#include "config.h"
#include "utils.h"
#include "event_log.h"
#include "email_manager.h"
#include "water_sensor.h"
#include "secrets.h"
#include "motion_sensor.h"
#include "settings.h"

#include <WiFi.h>

static int parseHHMMToMinutes(String value, int fallbackMinutes)
{
    value.trim();

    int colonIndex = value.indexOf(':');

    if (colonIndex < 1)
        return fallbackMinutes;

    String hoursText = value.substring(0, colonIndex);
    String minutesText = value.substring(colonIndex + 1);

    hoursText.trim();
    minutesText.trim();

    if (hoursText.length() == 0 || minutesText.length() != 2)
        return fallbackMinutes;

    for (unsigned int i = 0; i < hoursText.length(); i++)
    {
        if (!isDigit(hoursText.charAt(i)))
            return fallbackMinutes;
    }

    for (unsigned int i = 0; i < minutesText.length(); i++)
    {
        if (!isDigit(minutesText.charAt(i)))
            return fallbackMinutes;
    }

    int hours = hoursText.toInt();
    int minutes = minutesText.toInt();

    if (hours < 0 || hours > 24)
        return fallbackMinutes;

    if (minutes < 0 || minutes > 59)
        return fallbackMinutes;

    if (hours == 24 && minutes != 0)
        return fallbackMinutes;

    int totalMinutes = (hours * 60) + minutes;

    if (totalMinutes < 1 || totalMinutes > 1440)
        return fallbackMinutes;

    return totalMinutes;
}

static unsigned long parseDDHHMMToMinutes(
    String value,
    unsigned long fallbackMinutes)
{
    value.trim();

    int firstColon = value.indexOf(':');
    int secondColon = value.indexOf(':', firstColon + 1);

    if (firstColon < 1 || secondColon <= firstColon + 1)
        return fallbackMinutes;

    String daysText = value.substring(0, firstColon);
    String hoursText = value.substring(firstColon + 1, secondColon);
    String minutesText = value.substring(secondColon + 1);

    daysText.trim();
    hoursText.trim();
    minutesText.trim();

    if (daysText.length() == 0 ||
        hoursText.length() != 2 ||
        minutesText.length() != 2)
        return fallbackMinutes;

    for (unsigned int i = 0; i < daysText.length(); i++)
    {
        if (!isDigit(daysText.charAt(i)))
            return fallbackMinutes;
    }

    for (unsigned int i = 0; i < hoursText.length(); i++)
    {
        if (!isDigit(hoursText.charAt(i)))
            return fallbackMinutes;
    }

    for (unsigned int i = 0; i < minutesText.length(); i++)
    {
        if (!isDigit(minutesText.charAt(i)))
            return fallbackMinutes;
    }

    unsigned long days = (unsigned long)daysText.toInt();
    unsigned long hours = (unsigned long)hoursText.toInt();
    unsigned long minutes = (unsigned long)minutesText.toInt();

    if (days > 365 || hours > 23 || minutes > 59)
        return fallbackMinutes;

    unsigned long totalMinutes =
        (days * 24UL * 60UL) + (hours * 60UL) + minutes;

    if (totalMinutes < 1 || totalMinutes > 365UL * 24UL * 60UL)
        return fallbackMinutes;

    return totalMinutes;
}

void handleStatus()
{
    String json = "{";

    json += "\"currentTime\":\"" + getCurrentTime() + "\",";

    json += "\"waterStatus\":\"";
    json += (waterDetected ? "WET" : "DRY");
    json += "\",";

    json += "\"waterColor\":\"";
    json += (waterDetected ? "#e74c3c" : "#27ae60");
    json += "\",";

    json += "\"motionStatus\":\"";
    json += (isMotionDetected() ? "ACTIVE" : "IDLE");
    json += "\",";

    json += "\"motionCount\":\"" + String(getMotionCount()) + "\",";
    json += "\"lastMotion\":\"" + getLastMotionTime() + "\",";
    json += "\"lastMotionEmail\":\"" + getLastMotionEmailTime() + "\",";
    json += "\"motionEmailsToday\":\"" + String(getMotionEmailsToday()) + "\",";
    json += "\"motionCooldownMinutes\":\"" + String(getMotionEmailCooldownMinutes()) + "\",";
    json += "\"motionCooldownHHMM\":\"" + getMotionEmailCooldownHHMM() + "\",";
    json += "\"motionCooldownRemaining\":\"" + formatSecondsToTime(getMotionCooldownSecondsRemaining()) + "\",";
    json += "\"motionCooldownDetections\":\"" + String(getMotionDetectionsDuringCooldown()) + "\",";
    json += "\"suppressedMotionCount\":\"" + String(getSuppressedMotionCount()) + "\"";

    json += "}";

    server.send(200, "application/json", json);
}

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

    html += "<script>";
    html += "function updateDashboard(){";
    html += "fetch('/status')";
    html += ".then(r=>r.json())";
    html += ".then(d=>{";

    html += "var e=document.getElementById('waterStatus');";
    html += "if(e)e.innerHTML=d.waterStatus;";

    html += "e=document.getElementById('statusBox');";
    html += "if(e)e.style.background=d.waterColor;";

    html += "e=document.getElementById('currentTime');";
    html += "if(e)e.innerHTML=d.currentTime;";

    html += "e=document.getElementById('motionStatus');";
    html += "if(e)e.innerHTML=d.motionStatus;";

    html += "e=document.getElementById('motionCount');";
    html += "if(e)e.innerHTML=d.motionCount;";

    html += "e=document.getElementById('lastMotion');";
    html += "if(e)e.innerHTML=d.lastMotion;";

    html += "e=document.getElementById('lastMotionEmail');";
    html += "if(e)e.innerHTML=d.lastMotionEmail;";

    html += "e=document.getElementById('motionEmailsToday');";
    html += "if(e)e.innerHTML=d.motionEmailsToday;";

    html += "e=document.getElementById('motionCooldownMinutes');";
    html += "if(e)e.innerHTML=d.motionCooldownHHMM;";

    html += "e=document.getElementById('motionCooldownRemaining');";
    html += "if(e)e.innerHTML=d.motionCooldownRemaining;";

    html += "e=document.getElementById('motionCooldownDetections');";
    html += "if(e)e.innerHTML=d.motionCooldownDetections;";

    html += "e=document.getElementById('suppressedMotionCount');";
    html += "if(e)e.innerHTML=d.suppressedMotionCount;";

    html += "console.log('Dashboard updated', d);";

    html += "})";
    html += ".catch(err=>console.log('AJAX Error',err));";

    html += "}";

    html += "window.onload=function(){";
    html += "updateDashboard();";
    html += "setInterval(updateDashboard,2000);";
    html += "};";

    html += "</script>";

    html += "</head>";
    html += "<body>";
    html += "<div class='container'>";

    html += "<div class='header'>";
    html += "<h1 class='title'>💧 Water Sensor Dashboard</h1>";
    html += "<p class='version'>Firmware v" + String(FIRMWARE_VERSION) + "</p>";
    html += "</div>";

    html += "<div class='status-container'>";
    html += "<div class='status-box' id='statusBox'>";
    html += "<svg class='water-droplet' viewBox='0 0 24 24' fill='white'>";
    html += "<path d='M12 2.69l5.66 5.66a8 8 0 1 1-11.32 0z'/>";
    html += "</svg>";

    html += "<p class='status-text' id='waterStatus'>";
    html += statusText;
    html += "</p>";

    html += "</div>";
    html += "</div>";

    html += "<div class='grid'>";

    html += "<div class='card'><div class='card-title'>Sensor 1</div><div class='card-value'>";
    html += sensor1Detected ? "WET" : "DRY";
    html += "</div></div>";

    html += "<div class='card'><div class='card-title'>Sensor 2</div><div class='card-value'>";
    html += sensor2Detected ? "WET" : "DRY";
    html += "</div></div>";
    html += "<div class='card'><div class='card-title'>Current Time</div><div class='card-value' id='currentTime'>" + getCurrentTime() + "</div></div>";
    html += "<div class='card'><div class='card-title'>Uptime</div><div class='card-value'>" + getUptime() + "</div></div>";
    html += "<div class='card'><div class='card-title'>WiFi Signal</div><div class='card-value'>" + String(WiFi.RSSI()) + " dBm</div><div class='wifi-indicator'>" + getWiFiQuality() + "</div></div>";
    html += "<div class='card'><div class='card-title'>Current Interval (WET)</div><div class='card-value'>" + String(ALERT_INTERVALS[currentAlertIntervalIndex] / 60) + " min</div></div>";
    html += "<div class='card'><div class='card-title'>Alerts Sent</div><div class='card-value'>" + String(emailCounter) + "</div></div>";
    html += "<div class='card'><div class='card-title'>Last Alarm</div><div class='card-value'>" + lastAlarmTime + "</div></div>";
    html += "<div class='card'><div class='card-title'>Last Test Email</div><div class='card-value'>" + lastTestEmailTime + "</div></div>";
    html += "<div class='card'><div class='card-title'>Alert Email To</div><div class='card-value'>" + getRecipientEmails() + "</div></div>";
    html += "<div class='card'><div class='card-title'>Last Status Email</div><div class='card-value'>" + lastStatusEmailDate + "</div></div>";

    html += "<div class='card'><div class='card-title'>Email Notifications</div><div class='card-value'>";
    html += getEmailEnabled() ? "ENABLED" : "DISABLED";
    html += "</div></div>";
    html += "<div class='card'><div class='card-title'>Heartbeat Interval</div><div class='card-value'>";
    html += getHeartbeatIntervalDDHHMM();
    html += "</div></div>";
    html += "<div class='card'><div class='card-title'>Motion Status</div><div class='card-value' id='motionStatus'>";
    html += isMotionDetected() ? "ACTIVE" : "IDLE";
    html += "</div></div>";

    html += "<div class='card'><div class='card-title'>Motion Count</div><div class='card-value' id='motionCount'>";
    html += String(getMotionCount());
    html += "</div></div>";

    html += "<div class='card'><div class='card-title'>Last Motion</div><div class='card-value' id='lastMotion'>";
    html += getLastMotionTime();
    html += "</div></div>";

    html += "<div class='card'><div class='card-title'>Last Motion Email</div><div class='card-value' id='lastMotionEmail'>";
    html += getLastMotionEmailTime();
    html += "</div></div>";

    html += "<div class='card'><div class='card-title'>Motion Emails Today</div><div class='card-value' id='motionEmailsToday'>";
    html += String(getMotionEmailsToday());
    html += "</div></div>";

    html += "<div class='card'><div class='card-title'>Motion Cooldown</div><div class='card-value' id='motionCooldownMinutes'>";
    html += getMotionEmailCooldownHHMM();
    html += "</div></div>";

    html += "<div class='card'><div class='card-title'>Cooldown Remaining</div><div class='card-value' id='motionCooldownRemaining'>";
    html += formatSecondsToTime(getMotionCooldownSecondsRemaining());
    html += "</div></div>";

    html += "<div class='card'><div class='card-title'>Motion During Cooldown</div><div class='card-value' id='motionCooldownDetections'>";
    html += String(getMotionDetectionsDuringCooldown());
    html += "</div></div>";

    html += "<div class='card'><div class='card-title'>Suppressed Motion Total</div><div class='card-value' id='suppressedMotionCount'>";
    html += String(getSuppressedMotionCount());
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

    html += "<div class='card'><div class='card-title'>Email Status</div><div class='card-value'>";
    html += getEmailEnabled() ? "ENABLED" : "DISABLED";
    html += "</div></div>";

    html += "</div>";

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
    html += "<a class='btn btn-test' href='/settings'>⚙ Settings</a>";
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
void handleSettings()
{
    String html;

    html += "<!DOCTYPE html><html><head>";
    html += "<meta charset='utf-8'>";
    html += "<meta name='viewport' content='width=device-width,initial-scale=1'>";
    html += "<title>Settings</title>";

    html += "<style>";
    html += "body{background:#1a1a1a;color:#ecf0f1;font-family:Arial;padding:20px;}";
    html += ".container{max-width:800px;margin:auto;}";
    html += "input{width:100%;padding:12px;margin-top:5px;margin-bottom:20px;font-size:16px;}";
    html += ".btn{display:inline-block;padding:12px 20px;background:#3498db;color:white;text-decoration:none;border:none;border-radius:6px;cursor:pointer;margin-right:10px;}";
    html += "</style>";
    html += "</head><body>";

    html += "<div class='container'>";
    html += "<h1>⚙ Settings</h1>";
    if (server.hasArg("saved"))
    {
        html += "<div style='background:#27ae60;";
        html += "padding:15px;";
        html += "margin-bottom:20px;";
        html += "border-radius:8px;";
        html += "font-weight:bold;'>";
        html += "✓ Settings saved successfully";
        html += "</div>";
    }

    html += "<form method='POST' action='/savesettings'>";

    html += "<label>Email Recipients (comma-separated)</label>";
    html += "<input type='text' name='emails' value='" + getRecipientEmails() + "'>";

    html += "<label>Motion Email Cooldown (hh:mm)</label>";
    html += "<input type='text' ";
    html += "pattern='([0-9]{1,2}):[0-5][0-9]' ";
    html += "placeholder='00:15' ";
    html += "name='motionCooldown' value='";
    html += getMotionEmailCooldownHHMM();
    html += "'>";

    html += "<label>Heartbeat Interval (dd:hh:mm)</label>";
    html += "<input type='text' ";
    html += "pattern='([0-9]{1,3}):[0-2][0-9]:[0-5][0-9]' ";
    html += "placeholder='07:00:00' ";
    html += "name='heartbeat' value='";
    html += getHeartbeatIntervalDDHHMM();
    html += "'>";

    html += "<label style='display:flex;";
    html += "align-items:center;";
    html += "gap:15px;";
    html += "font-size:24px;";
    html += "margin-bottom:25px;'>";

    html += "<input type='checkbox' ";
    html += "name='emailEnabled' ";

    if (getEmailEnabled())
    {
        html += "checked ";
    }

    html += "style='width:28px;";
    html += "height:28px;";
    html += "margin:0;'>";

    html += "<span>Enable Email Notifications</span>";

    html += "</label>";

    html += "<br><br>";

    html += "<button class='btn' type='submit'>Save Settings</button>";
    html += "<a class='btn' href='/'>Back To Dashboard</a>";

    html += "</form>";
    html += "</div>";
    html += "</body></html>";

    server.send(200, "text/html", html);
}

void handleSaveSettings()
{
    if (server.hasArg("emails"))
    {
        setRecipientEmails(server.arg("emails"));
    }

    if (server.hasArg("heartbeat"))
    {
        unsigned long heartbeatMinutes = parseDDHHMMToMinutes(
            server.arg("heartbeat"),
            getHeartbeatIntervalMinutes()
        );

        setHeartbeatIntervalMinutes(heartbeatMinutes);
    }
    if (server.hasArg("motionCooldown"))
    {
        int cooldownMinutes = parseHHMMToMinutes(
            server.arg("motionCooldown"),
            getMotionEmailCooldownMinutes()
        );

        setMotionEmailCooldownMinutes(cooldownMinutes);
    }

    bool emailEnabled = server.hasArg("emailEnabled");
    setEmailEnabled(emailEnabled);
    addEvent("Settings updated");

    server.sendHeader("Location", "/settings?saved=1");
    server.send(302, "text/plain", "");
}
void setupWebUI()
{
    server.on("/", handleRoot);

    server.on("/wet", handleSimulateWet);
    server.on("/dry", handleSimulateDry);
    server.on("/testemail", handleTestEmail);

    server.on("/status", handleStatus);

    server.on("/settings", HTTP_GET, handleSettings);
    server.on("/savesettings", HTTP_POST, handleSaveSettings);
}
