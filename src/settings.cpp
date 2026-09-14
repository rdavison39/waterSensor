#include "settings.h"
#include <Arduino.h>
#include <Preferences.h>

Preferences preferences;

static const char* DEFAULT_EMAILS =
    "ronaldjdavison@hotmail.com,sonya_davison@hotmail.com";

static const int DEFAULT_HEARTBEAT_DAYS = 7;
static const unsigned long MIN_HEARTBEAT_MINUTES = 1;
static const unsigned long MAX_HEARTBEAT_MINUTES = 365UL * 24UL * 60UL;

static const char* DEFAULT_DEVICE_NAME =
    "ESP32 Water Sensor";

void setupSettings()
{
    preferences.begin("waterSensor", false);
}

String getRecipientEmails()
{
    return preferences.getString(
        "emails",
        DEFAULT_EMAILS
    );
}

void setRecipientEmails(const String& emails)
{
    preferences.putString("emails", emails);
}

int getHeartbeatIntervalDays()
{
    unsigned long minutes = getHeartbeatIntervalMinutes();
    int days = (int)(minutes / 1440UL);

    if ((minutes % 1440UL) > 0)
        days++;

    if (days < 1)
        days = 1;

    return days;
}

void setHeartbeatIntervalDays(int days)
{
    if (days < 1)
    {
        days = 1;
    }

    if (days > 365)
    {
        days = 365;
    }

    preferences.putInt("heartbeat", days);
    setHeartbeatIntervalMinutes((unsigned long)days * 24UL * 60UL);
}

unsigned long getHeartbeatIntervalMinutes()
{
    if (preferences.isKey("heartbeatMin"))
    {
        return preferences.getUInt(
            "heartbeatMin",
            DEFAULT_HEARTBEAT_DAYS * 24UL * 60UL
        );
    }

    return (unsigned long)preferences.getInt(
        "heartbeat",
        DEFAULT_HEARTBEAT_DAYS
    ) * 24UL * 60UL;
}

String getHeartbeatIntervalDDHHMM()
{
    unsigned long totalMinutes = getHeartbeatIntervalMinutes();
    unsigned long days = totalMinutes / 1440UL;
    unsigned long remainingMinutes = totalMinutes % 1440UL;
    unsigned long hours = remainingMinutes / 60UL;
    unsigned long minutes = remainingMinutes % 60UL;

    char buffer[16];
    snprintf(
        buffer,
        sizeof(buffer),
        "%02lu:%02lu:%02lu",
        days,
        hours,
        minutes
    );

    return String(buffer);
}

void setHeartbeatIntervalMinutes(unsigned long minutes)
{
    if (minutes < MIN_HEARTBEAT_MINUTES)
        minutes = MIN_HEARTBEAT_MINUTES;

    if (minutes > MAX_HEARTBEAT_MINUTES)
        minutes = MAX_HEARTBEAT_MINUTES;

    preferences.putUInt("heartbeatMin", minutes);

    unsigned long wholeDays = minutes / 1440UL;
    if ((minutes % 1440UL) > 0)
        wholeDays++;

    if (wholeDays < 1)
        wholeDays = 1;

    preferences.putInt("heartbeat", (int)wholeDays);

    Serial.print("[SETTINGS] Heartbeat interval set to ");
    Serial.print(minutes);
    Serial.println(" minutes");
}

String getDeviceName()
{
    return preferences.getString(
        "deviceName",
        DEFAULT_DEVICE_NAME
    );
}

void setDeviceName(const String& name)
{
    preferences.putString("deviceName", name);
}

String getLastHeartbeatDate()
{
    return preferences.getString(
        "lastHB",
        ""
    );
}

void setLastHeartbeatDate(const String& date)
{
    preferences.putString(
        "lastHB",
        date
    );
}

unsigned long getLastHeartbeatTimestamp()
{
    return preferences.getUInt("lastHBTS", 0);
}

void setLastHeartbeatTimestamp(unsigned long timestamp)
{
    preferences.putUInt("lastHBTS", timestamp);
}

bool getEmailEnabled()
{
    return preferences.getBool("emailEnabled", true);
}

void setEmailEnabled(bool enabled)
{
    preferences.putBool("emailEnabled", enabled);
}

int getMotionEmailCooldownMinutes()
{
    return preferences.getInt("motionCooldown", 15);
}

String getMotionEmailCooldownHHMM()
{
    int minutes = getMotionEmailCooldownMinutes();
    int hours = minutes / 60;
    int remainingMinutes = minutes % 60;

    char buffer[8];
    snprintf(
        buffer,
        sizeof(buffer),
        "%02d:%02d",
        hours,
        remainingMinutes
    );

    return String(buffer);
}

void setMotionEmailCooldownMinutes(int minutes)
{
    if (minutes < 1)
        minutes = 1;

    if (minutes > 1440)
        minutes = 1440;

    preferences.putInt("motionCooldown", minutes);

    Serial.print("[SETTINGS] Motion cooldown set to ");
    Serial.print(minutes);
    Serial.println(" minutes");
}


String getLastWiFiSSID()
{
    return preferences.getString("lastSSID", "");
}

void setLastWiFiSSID(const String& ssid)
{
    preferences.putString("lastSSID", ssid);
}
