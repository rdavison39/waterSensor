#include "utils.h"
#include "config.h"
#include <time.h>

String getCurrentTime()
{
    time_t now = time(nullptr);
    struct tm* timeinfo = localtime(&now);

    char buffer[64];
    strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", timeinfo);
    return String(buffer);
}

String getCurrentDate()
{
    time_t now = time(nullptr);
    struct tm* timeinfo = localtime(&now);

    char buffer[16];
    strftime(buffer, sizeof(buffer), "%Y-%m-%d", timeinfo);
    return String(buffer);
}

int getDayOfMonth()
{
    time_t now = time(nullptr);
    struct tm* timeinfo = localtime(&now);
    return timeinfo->tm_mday;
}

bool isStatusEmailDay()
{
    int dayOfMonth = getDayOfMonth();
    for (int i = 0; i < NUM_STATUS_DAYS; i++)
    {
        if (dayOfMonth == STATUS_EMAIL_DAYS[i])
            return true;
    }
    return false;
}

String getWiFiColor()
{
    int rssi = WiFi.RSSI();
    if (rssi >= -50) return "#27ae60";
    if (rssi >= -60) return "#2ecc71";
    if (rssi >= -70) return "#f39c12";
    if (rssi >= -80) return "#e67e22";
    return "#e74c3c";
}

unsigned long getSecondsUntilNextWetEmail()
{
    if (!waterDetected)
        return 0;

    if (lastWetEmailTime == 0)
        return 0;

    time_t now = time(nullptr);
    unsigned long secondsSinceLast = (unsigned long)now - lastWetEmailTime;

    unsigned long interval = ALERT_INTERVALS[currentAlertIntervalIndex];
    if (secondsSinceLast >= interval)
        return 0;
    return interval - secondsSinceLast;
}

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
    unsigned long seconds = (millis() - bootMillis) / 1000;

    unsigned long days = seconds / 86400;
    seconds %= 86400;

    unsigned long hours = seconds / 3600;
    seconds %= 3600;

    unsigned long minutes = seconds / 60;
    seconds %= 60;

    char buffer[64];
    sprintf(buffer, "%lu d %lu h %lu m %lu s", days, hours, minutes, seconds);
    return String(buffer);
}
