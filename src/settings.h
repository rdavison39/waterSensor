#ifndef SETTINGS_H
#define SETTINGS_H

#include <Arduino.h>

void setupSettings();

String getRecipientEmails();
void setRecipientEmails(const String& emails);

int getHeartbeatIntervalDays();
void setHeartbeatIntervalDays(int days);

String getDeviceName();
void setDeviceName(const String& name);

String getLastHeartbeatDate();
void setLastHeartbeatDate(const String& date);

bool getEmailEnabled();
void setEmailEnabled(bool enabled);

int getMotionEmailCooldownMinutes();
void setMotionEmailCooldownMinutes(int minutes);

#endif