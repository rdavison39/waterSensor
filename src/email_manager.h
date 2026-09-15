#ifndef EMAIL_MANAGER_H
#define EMAIL_MANAGER_H

#include <Arduino.h>

//====================================================
// Generic Email
//====================================================

void sendEmail(
    const String &subject,
    const String &body
);

//====================================================
// Water Alarm Emails
//====================================================

void sendAlarmEmail();

void sendWetAlertEmail();

void sendWaterClearedEmail();

//====================================================
// Other Emails
//====================================================

void sendTestEmail();

void sendStatusEmail();

void sendStartupEmail();

#endif