#include "settings.h"

#include <Preferences.h>

Preferences preferences;

static const char* DEFAULT_EMAILS =
    "ronaldjdavison@hotmail.com,sonya_davison@hotmail.com";

static const int DEFAULT_HEARTBEAT_DAYS = 7;

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
    return preferences.getInt(
        "heartbeat",
        DEFAULT_HEARTBEAT_DAYS
    );
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

void setMotionEmailCooldownMinutes(int minutes)
{
    if (minutes < 1)
        minutes = 1;

    if (minutes > 1440)
        minutes = 1440;

    preferences.putInt("motionCooldown", minutes);
}