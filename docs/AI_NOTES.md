# ESP32 Water Sensor Project - AI Notes

## Project Purpose

This project is an ESP32-based remote monitoring system designed for unattended operation at a cottage. The primary purpose is to detect water leaks and motion, send email notifications, provide a web dashboard, and periodically report system health.

The firmware is intended to operate for months without user intervention.

---

# Source of Truth

are these files

---
Project Rules

1. These files are the source of truth (i will commit to github later)

2. Always preserve existing functionality.

3. Never remove features unless requested.

4. Prefer modifying existing code over rewriting modules.

5. Provide complete compileable changes.

6. All settings belong in Preferences.

7. New features should appear in the dashboard.

8. Update CHANGELOG.md after completing a feature.

9. Use section separators in source files.

10. Firmware versions follow semantic versioning.

# Development Rules

These rules are mandatory.

1. Never remove existing functionality unless explicitly requested.

2. Always provide COMPLETE replacement source files whenever practical.

3. Do not provide partial snippets for large source files.

4. Preserve existing UI unless changes are specifically requested.

5. Backward compatibility is preferred.

6. All configurable values should eventually be stored in Preferences rather than hardcoded.

7. Every new feature should include appropriate Serial debug output.

8. Every significant event should be added to the event log.

9. Keep the web dashboard responsive.

10. Favor reliability over cleverness.

---

# Current Hardware

ESP32 Dev Module

Water Sensor #1
GPIO 27

Water Sensor #2 (planned)
GPIO 26

PIR Motion Sensor
GPIO 22

---

# Current Features

✓ Water detection

✓ Motion detection

✓ Startup email

✓ Water alarm email

✓ Repeating wet reminder emails

✓ Motion emails with cooldown

✓ Heartbeat email

✓ AJAX dashboard

✓ Event log

✓ WiFi failover

✓ Persistent settings using Preferences

---

# Email

SMTP Provider

Gmail

Email Library

ESP Mail Client

Email Types

Startup

Heartbeat

Water Alarm

Water Reminder

Motion

Test Email

Future

Per-sensor email recipients

---

# Web Dashboard

Displays

Current sensor status

Current time

Uptime

WiFi RSSI

Water alarm status

Motion status

Motion count

Event log

Email controls

Future

Editable settings

Motion cooldown

Heartbeat interval

Recipient emails

Second water sensor

---

# Heartbeat

Heartbeat is sent once every odd-numbered day.

Only one heartbeat may be sent per day.

The date of the last heartbeat is stored in Preferences.

---

# Motion Sensor

Current functionality

Motion count

Cooldown timer

Email notification

Debug logging

Future improvements

Configurable cooldown

Emails today

Suppressed motion counter

Cooldown countdown

Motion duration

---

# Water Sensors

Current

One sensor implemented.

Future

Two independent water sensors.

Dashboard should display both separately.

Email should identify which sensor triggered.

---

# WiFi

Primary SSID

Secondary SSID

Automatically reconnect

Remember last successful SSID using Preferences.

---

# Coding Style

Prefer readable code over compact code.

Use clear section headers.

Avoid magic numbers.

Log important operations to Serial.

---

# Current Firmware

Version

3.1.0

Next Planned Version

3.2.0

Cleanup release

---

# Long-Term Roadmap

Version 3.2

Project cleanup

AI documentation

Dashboard improvements

Motion statistics

Configurable settings

Version 3.3

Second water sensor

Per-sensor email recipients

Configuration pages

Version 4.0

Fully configurable appliance

LittleFS configuration

REST API

OTA firmware updates

Multiple sensor types

---

# Notes for Future AI Sessions

This project has evolved over many months.

Before modifying code:

Understand existing architecture.

Preserve functionality.

Do not simplify working code.

When changing one module, consider dependencies in:

config.h

settings.*

email_manager.*

motion_sensor.*

water_sensor.*

heartbeat.*

web_ui.*

main.cpp

Reliability is more important than minimizing code.

## Coding Standards

Header files should use logical section separators.

All modules should expose setupXXX() and loopXXX() where appropriate.

Persistent settings belong in Preferences via settings.cpp.

New configurable values should never be hardcoded.

All modules should log significant events.

Every feature should have corresponding dashboard support where appropriate.