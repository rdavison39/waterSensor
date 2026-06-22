#ifndef EMAIL_MANAGER_H
#define EMAIL_MANAGER_H

#include <Arduino.h>

void sendEmail(const String &subject, const String &body);
void sendAlarmEmail();
void sendWetAlertEmail();
void sendTestEmail();
void sendStatusEmail();
void sendStartupEmail();

#endif // EMAIL_MANAGER_H
