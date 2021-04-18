/*
 The MIT License (MIT)

 Copyright (c) 2019-2020 Dirk Ohme

 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in all
 copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 SOFTWARE.
*/

#pragma once
#include <RtcDateTime.h>

class CClock {
public:
	// events
	enum EEvent
	{
		eEventNone  = 0,
		eEventMinute,
		eEventHour,
		eEventDay
	};
	
	// snooze time (minutes)
	static const uint8_t SnoozeMinutes = 5;
  
protected:
	uint8_t		au8Alarm[2];
	uint8_t		au8AlarmNext[2];
	bool		boAlarm;
	bool		boAlarmEnable;
	bool		boAlarmValid;
	bool		boHWClock;
	char		szAlarmTime[16];
	uint32_t	u32Next;
	uint32_t	u32Start;
  
	// save alarm time
	// @return true on success, false on error
	bool SaveAlarm();

public:
	// current date and time
	RtcDateTime	dtNow;
	
	// constructor
	CClock();

	// check for event
	// @return event type
	EEvent CheckEvent();

	// enable or disable alarm
	// @param boEnable true to enable, false to disable
	// @return true if enabled, false if disabled
	bool EnableAlarm(const bool boEnable = true);
	
	// get alarm time as string
	// @return string
	const char* GetAlarmTime();
	
	// check if hard- or software clock
	// @return true on hardware clock, false on software emulation
	inline bool HasHWClock() { return boHWClock; }
	
	// initialize class
	// @return true on success, false on error
	bool Init();
	
	// check if alarm is pending
	// @return true on alarm, false if no alarm
	inline bool IsAlarm() { return boAlarm; }
	
	// check if alarm is enabled
	// @return true if enabled, false if disabled
	inline bool IsAlarmEnabled() { return boAlarmEnable; }
	
	// check if alarm time is valid
	// @return true if valid, false if invalid
	inline bool IsAlarmValid() { return boAlarmValid; }
	
	// check if day or night mode
	// @return true on day, false on night
	inline bool IsDay() { return ((dtNow.Hour() >= 6) && (dtNow.Hour() <= 18)); }
	
	// set alarm time
	// @param iHour hour (0..23)
	// @param iMin minute (0..59)
	// @return true on success, false on error
	bool SetAlarm(const int iHour, const int iMin);
	
	// set alarm time (relative)
	// @param iHourRel relative hour (-1, 0, +1)
	// @param iMinRel relative minute (-59..0..+59)
	// @return true on success, false on error
	bool SetAlarmRelative(const int iHourRel = 0, const int iMinRel = 1);
	
	// set (hardware) clock
	// @return true on success, false on error
	bool SetClock();
	
	// set snooze
	// @param bo24Hours true if snooze for 24 hours, false for snooze time
	void Snooze(bool bo24Hours = false);
	
	// toogle alarm
	// @return true if enabled, false if disabled
	inline bool ToggleAlarm() { return EnableAlarm(!boAlarmEnable); }
};

extern CClock Clock;

//===| eof - end of file |====================================================
