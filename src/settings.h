#ifndef SETTINGS_H
#define SETTINGS_H

#include <Arduino.h>

//----------------------------------------------------
// Initialization
//----------------------------------------------------
void setupSettings();

//----------------------------------------------------
// Heartbeat Settings
//----------------------------------------------------
int getHeartbeatIntervalDays();
void setHeartbeatIntervalDays(int days);
unsigned long getHeartbeatIntervalMinutes();
String getHeartbeatIntervalDDHHMM();
void setHeartbeatIntervalMinutes(unsigned long minutes);
String getLastHeartbeatDate();
void setLastHeartbeatDate(const String &date);
unsigned long getLastHeartbeatTimestamp();
void setLastHeartbeatTimestamp(unsigned long timestamp);

//----------------------------------------------------
// Email Settings
//----------------------------------------------------
String getRecipientEmails();
void setRecipientEmails(const String &emails);
bool getEmailEnabled();
void setEmailEnabled(bool enabled);

//----------------------------------------------------
// WiFi Settings
//----------------------------------------------------
String getLastWiFiSSID();
void setLastWiFiSSID(const String &ssid);

//----------------------------------------------------
// Motion Settings
//----------------------------------------------------
int getMotionEmailCooldownMinutes();
String getMotionEmailCooldownHHMM();
void setMotionEmailCooldownMinutes(int minutes);

//----------------------------------------------------
// Future Settings (v2.5+)
//----------------------------------------------------
// bool getMotionEmailEnabled();
// void setMotionEmailEnabled(bool enabled);
//
// bool getWaterEmailEnabled();
// void setWaterEmailEnabled(bool enabled);
//
// bool getHeartbeatEmailEnabled();
// void setHeartbeatEmailEnabled(bool enabled);

#endif
