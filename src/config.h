#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>
#include <WebServer.h>

//====================================================
// Hardware Configuration
//====================================================

#define WATER_SENSOR_1_PIN 27
#define WATER_SENSOR_2_PIN 26
#define MOTION_PIN         22

#define FIRMWARE_VERSION "3.1.0"

//====================================================
// SMTP
//====================================================

#define SMTP_HOST "smtp.gmail.com"
#define SMTP_PORT 465

// Toggle all email sending
static const bool EMAIL_ENABLED = true;

//====================================================
// Web Server
//====================================================

extern WebServer server;

//====================================================
// Water Sensor
//====================================================

extern bool waterDetected;
extern bool alarmEmailSent;

extern String lastAlarmTime;

extern unsigned long lastWetEmailTime;

extern int alarmCounter;
extern int wetEmailCounter;

extern int currentAlertIntervalIndex;

extern const unsigned long ALERT_INTERVALS[];
extern const int NUM_ALERT_INTERVALS;

//====================================================
// Motion Sensor
//====================================================

extern bool motionState;

extern unsigned long motionCount;

extern unsigned long motionEmailsToday;
extern unsigned long suppressedMotionCount;

extern unsigned long lastMotionEmailMillis;

extern int motionEmailCooldownMinutes;

extern String lastMotionTime;
extern String lastMotionEmailTimestamp;
extern String lastMotionEmailDate;

//====================================================
// Heartbeat
//====================================================

extern const int STATUS_EMAIL_DAYS[];
extern const int NUM_STATUS_DAYS;

//====================================================
// General
//====================================================

extern unsigned long bootMillis;

extern String lastTestEmailTime;
extern String lastStatusEmailDate;

extern int emailCounter;

#define MAX_EVENTS 10

#endif