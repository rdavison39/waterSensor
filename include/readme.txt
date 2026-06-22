to build press Ctrl + Alt + B
to upload pres ctrl +alt + u  (remember to press the button on the esp32)

URL

For motion sensor with 3 pins facing you in up it goes

grey - ground
white - p22
black -  3v3

uses GMAIL SMTP to send email alerts, so you will need to set up an app password for your gmail account and put that in secrets.h


ESP32 Cottage Monitor Project – Current State (June 2026)

Project Overview

* ESP32 Dev Module
* Web dashboard with AJAX updates
* Water leak detection
* Motion detection
* Gmail SMTP email notifications
* Event logging
* NTP time synchronization
* Heartbeat/status emails

Important User Rules

1. Always provide complete code files when modifying code.
2. Never remove existing functionality without asking first.
3. Never change the UI without asking first.

Current Hardware

* ESP32 Dev Module
* Water sensor on GPIO27
* Motion sensor on GPIO22
* WiFi connected
* Gmail SMTP working

Current Software Status

Working:

* WiFi connection
* NTP time sync
* Web dashboard
* Motion detection
* Motion email alerts
* Water detection
* Water alarm emails
* Test email button
* Status/heartbeat emails
* Event logging
* AJAX updates on dashboard
* Wet alert scheduling

Dashboard Shows:

* Water status (WET/DRY)
* Motion status
* Motion count
* Last motion
* Last motion email
* Current time
* Uptime
* WiFi signal
* Last alarm
* Last test email
* Last status email
* Event log
* Simulate Wet button
* Simulate Dry button
* Send Test Email button

Current Water Sensor Logic

water_sensor.cpp currently contains debounce logic:

* WET must be continuously LOW for 2 seconds before alarming
* DRY must be continuously HIGH for 60 seconds before clearing

Current code:

void setupWaterSensor()
{
pinMode(WATER_PIN, INPUT_PULLUP);
}

Water Sensor Findings

* GPIO27 shorted directly to GND works perfectly
* Sensor works in salt water
* Sensor sometimes works in tap water
* Sensor expected deployment: under hot water tank relief valve
* Likely conductivity issue rather than software issue
* No resistor changes implemented yet
* No hardware changes planned until tested at cottage with actual well water

NTP / Heartbeat Issue Resolved
Problem:

* Heartbeat emails were not sending
* Device was reporting 1969-12-31

Cause:

* NTP had not synchronized

Current Status:

* NTP now synchronizes correctly
* Status emails now send successfully
* Dashboard now shows correct Last Status Email date

Current Heartbeat Logic

main.cpp:

if (isStatusEmailDay() &&
lastStatusEmailDate != getCurrentDate())
{
sendStatusEmail();
}

utils.cpp:

bool isStatusEmailDay()
{
int dayOfMonth = getDayOfMonth();

```
for (int i = 0; i < NUM_STATUS_DAYS; i++)
{
    if (dayOfMonth == STATUS_EMAIL_DAYS[i])
        return true;
}

return false;
```

}

config in main.cpp:

const int STATUS_EMAIL_DAYS[] =
{
1,
8,
15,
22,
29
};

Current Email System

* Gmail SMTP works
* Motion emails work
* Water alarm emails work
* Status emails work
* Test email works

Current sender name:

message.sender.name = "ESP32 Water Sensor";

User may later rename to:
"Eldorado Cottage Monitor"

Next Feature To Build

Goal:
Make email recipients and heartbeat interval configurable from the web UI.

Desired UI

Add Settings page:

⚙ Settings

## Email Recipients:

[ronaldjdavison@hotmail.com](mailto:ronaldjdavison@hotmail.com),[sonya_davison@hotmail.com](mailto:sonya_davison@hotmail.com)

## Heartbeat Interval (Days):

2

[ Save Settings ]

Requirements

* Store settings in ESP32 Preferences
* Survive reboot
* Survive power failure
* No code changes required after deployment

Implementation Plan

Step 1
Create:

settings.h
settings.cpp

Store:

* recipientEmails
* heartbeatIntervalDays

Using:
#include <Preferences.h>

Step 2
Load settings during startup.

Step 3
Modify email_manager.cpp

Replace:

message.addRecipient("Ron", RECIPIENT_EMAIL);

With:

* Parse comma-separated email list
* Add each recipient automatically

Example:

[ron@gmail.com](mailto:ron@gmail.com)

or

[ron@gmail.com](mailto:ron@gmail.com),[sonya@gmail.com](mailto:sonya@gmail.com)

or

[ron@gmail.com](mailto:ron@gmail.com),[sonya@gmail.com](mailto:sonya@gmail.com),[sydney@gmail.com](mailto:sydney@gmail.com)

Step 4
Add /settings page to web UI.

Step 5
Add Save button and POST handler.

Step 6
Replace STATUS_EMAIL_DAYS logic with configurable heartbeatIntervalDays logic.

Desired Result

* User can change recipients from phone/browser
* User can change heartbeat frequency from phone/browser
* No firmware upload required for future changes

Known Good State Before Starting Settings Work

* Water alarms working
* Motion alerts working
* Status emails working
* Gmail working
* Dashboard working
* NTP working
* AJAX updates working

==================
recipientEmails
heartbeatIntervalDays

Using:
#include <Preferences.h>

Step 2
Load settings during startup.

Step 3
Modify email_manager.cpp

Replace:

message.addRecipient("Ron", RECIPIENT_EMAIL);

With:

Parse comma-separated email list
Add each recipient automatically

Example:

ron@gmail.com

or

ron@gmail.com,sonya@gmail.com

or

ron@gmail.com,sonya@gmail.com,sydney@gmail.com

Step 4
Add /settings page to web UI.

Step 5
Add Save button and POST handler.

Step 6
Replace STATUS_EMAIL_DAYS logic with configurable heartbeatIntervalDays logic.

Desired Result

User can change recipients from phone/browser
User can change heartbeat frequency from phone/browser
No firmware upload required for future changes

Known Good State Before Starting Settings Work

Water alarms working
Motion alerts working
Status emails working
Gmail working
Dashboard working
NTP working
AJAX updates working