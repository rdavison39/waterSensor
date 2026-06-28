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
String getLastHeartbeatDate();
void setLastHeartbeatDate(const String &date);

//----------------------------------------------------
// WiFi Settings
//----------------------------------------------------
String getLastWiFiSSID();
void setLastWiFiSSID(const String &ssid);

//----------------------------------------------------
// Motion Settings
//----------------------------------------------------
int getMotionEmailCooldownMinutes();
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