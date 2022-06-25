# Clock-Library

Arduino software emulation of a real-time clock based on the milliseconds
timer of the CPU. The clock works with a resolution of one minute.
- 24h alarm with snooze option
- events for new minute, hour, day
- backup of clock and alarm info to EEPROM

## Clock

Software clock. A variable *MyClock* with a default clock object is available.

### Constants
- Months
  - array with abbreviations of the month names
- SnoozeMinutes
  - default snooze time in minutes (5)


### Constructor
- Clock()
  - no special parameters
  - clock initialization is done by Init() and SetClock()

### Methods
- MyClock.CheckEvent()
  - should be called periodically in order to keep the clock running, at
    least every couple of seconds or more often
  - returns one of the following values:
    - eEventNone = no clock update
    - eEventMinute = new minute starts
    - eEventHour = new hour starts
    - eEventDay = new day starts
    
- MyClock.Day()
  - returns the current day of month (1..31)
  
- MyClock.EnableAlarm(boEnable)
  - enable (true) or disable (false) alarm
  
- MyClock.GetAlarmTime()
  - returns current alarm time:
    - empty string, if no alarm is set
    - string ' HH:MM' for alarm time
    - string '*HH:MM' for next alarm time (if snoozed)
    
- MyClock.GetDateStr()
  - returns current date as 'dd. mon yyyy'
  
- MyClock.Hour()
  - returns current hour (0..23)
  
- MyClock.Init()
  - initialize clock, has to be called first to any other method
  - tries to read stored information from EEPROM about clock and alarm
  
- MyClock.IsAlarm()
  - returns *true* if alarm is active

- MyClock.IsAlarmEnabled()
  - returns *true* if alarm is valid and enabled
  
- MyClock.IsAlarmValid()
  - returns *true* if a valid alarm is stored
  - valid means that the alarm time is valid, the alarm may be enabled or not

- MyClock.IsDay()
  - returns *true* on day, *false* on night
  
- MyClock.Loop()
  - may be called instead of CheckEvent() in order to keep the clock running
  
- MyClock.Minute()
  - returns the current minute (0..59)
  
- MyClock.Month()
  - returns the current month (1..12)
  
- MyClock.SaveClock()
  - save current clock data (date and time) to EEPROM if available
  - returns *true* on success
  
- MyClock.SetAlarm(hour, min)
  - sets a new alarm time and enable alarm
  - returns *true* on success
  
- MyClock.SetAlarm(string)
  - sets a new alarm time given as string 'HH:MM' and enable alarm
  - returns *true* on success

- MyClock.SetAlarmRelative(dHour, dMin)
  - changes alarm time relative the current alarm time:
    - dHour +1 means plus one hour, -1 means minus one hour, 0 keeps hour
    - dMin +1 means plus one minute, -1 means minus one minute, 0 keeps minute
  - returns *true* on success
  
- MyClock.SetClock(struct)
  - set clock by a 'struct tm'
  - returns *true* on success
	
- MyClock.SetClock(time_t, utc)
  - set clock by a time_t value, utc *true* means usage of UTC else local time
  - returns *true* on success

- MyClock.SetClock()
  - activate clock
  - returns *true* on success
	
- MyClock.Snooze(bo24Hours)
  - snooze an active alarm either by 24 hours (*true*) or 'SnoozeMinutes' (*false*)
  - returns *true* on success
	
- MyClock.ToggleAlarm()
  - toggle alarm state (enabled/disabled)
  
- MyClock.Year()
  - returns current year (1900..2099)

## Releases

- 1.0.0 Initial release.
- 1.0.1 Minor bug-fixes
- 1.0.2 Minor bug-fixes
- 1.0.3 Minor bug-fixes
- 1.0.4 missing return values on SaveAlarm() and SaveClock, changed behaviour on boAlarm_m (line 127)

## How to install

[Installing Additional Arduino Libraries](https://www.arduino.cc/en/Guide/Libraries)
