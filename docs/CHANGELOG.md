# Changelog

## 3.2.0 - Motion Email Settings and Stats

- Exposed motion email cooldown values on the dashboard and settings UI.
- Added daily motion email numbering in motion email subjects.
- Added midnight reset logic for the daily motion email counter.
- Added motion cooldown suppression counters.
- Expanded motion email body with heartbeat-style system, network, sensor, configuration, counter, and last-activity details.
- Changed motion cooldown settings entry from raw minutes to `hh:mm` format while keeping Preferences storage in minutes.
- Added timestamps to event log entries shown in the dashboard.
- Changed heartbeat interval settings entry from days to `dd:hh:mm` and schedules heartbeat checks by minutes.
