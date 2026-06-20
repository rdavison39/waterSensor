#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>
#include <WebServer.h>

// Pin and firmware configuration
#define WATER_PIN 27
#define MOTION_PIN 22
#define FIRMWARE_VERSION "2.1.0"

// Email / SMTP
#define SMTP_HOST "smtp.gmail.com"
#define SMTP_PORT 465

// Toggle email sending while testing
static const bool EMAIL_ENABLED = false;

// Forward-declare the web server instance
extern WebServer server;

// Global runtime state (defined in main.cpp)
extern bool waterDetected;
extern bool alarmEmailSent;
extern bool motionState;
extern unsigned long bootMillis;

extern String lastAlarmTime;
extern String lastTestEmailTime;
extern String lastStatusEmailDate;
extern int alarmCounter;
extern int emailCounter;

extern unsigned long lastWetEmailTime;
extern int wetEmailCounter;
extern int currentAlertIntervalIndex;
extern const unsigned long ALERT_INTERVALS[];
extern const int NUM_ALERT_INTERVALS;

extern const int STATUS_EMAIL_DAYS[];
extern const int NUM_STATUS_DAYS;

#define MAX_EVENTS 10

#endif // CONFIG_H
