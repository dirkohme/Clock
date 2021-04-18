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

//---| definitions |----------------------------------------------------------
#undef TEST
//#define TEST

//---|debugging |---------------------------------------------------------------
#if defined(TEST)
#  define DebugOut(s)         Serial.println(s)
#else
#  define DebugOut(s)
#endif

//---| definitions |------------------------------------------------------------
#define       ADDR_ALARM_MSB    0x00
#define       ADDR_ALARM_LSB    0x01
#define       ADDR_ALARM_ENABLE 0x02

//---| includes |---------------------------------------------------------------
#if defined(ESP8266)
#  include <pgmspace.h>
#else
#  include <avr/pgmspace.h>
#endif
#include <Arduino.h>
#include <EEPROM.h>
#include <Wire.h>
#include <RtcDateTime.h>
#include <RtcDS1307.h>
#include "Clock.h"

//---| globals |----------------------------------------------------------------
static RtcDS1307<TwoWire> s_clsHWClock(Wire);
static const RtcDateTime  sc_dtCompiled = RtcDateTime(__DATE__, __TIME__);

//----------------------------------------------------------------------------
// constructor
//----------------------------------------------------------------------------
CClock::CClock()
{
	au8Alarm[0]     = au8Alarm[1]     = 0xFFu;
	au8AlarmNext[0] = au8AlarmNext[1] = 0xFFu;
	boAlarm         =
	boAlarmEnable   = false;
	boHWClock       = false;
	szAlarmTime[0]  = '\0';
	u32Next         = u32Start        = 0ul;
}

//----------------------------------------------------------------------------
// check for next event
//----------------------------------------------------------------------------
CClock::EEvent CClock::CheckEvent()
{
	CClock::EEvent eEvent  =  CClock::eEventNone;
	uint32_t       u32Diff;
	uint8_t        u8Day, u8Hour;

	// check for next minute
	if (millis() > u32Next)
	{
		// next minute
		u8Day   = dtNow.Day();
		u8Hour  = dtNow.Hour();

		if (boHWClock)
		{
			dtNow = s_clsHWClock.GetDateTime();
#if defined(TEST)
			Serial.printf("[clock] clock %02u:%02u:%02u (RTC)\n",
			              dtNow.Hour(), dtNow.Minute(), dtNow.Second());
#endif
		}
		else
		{
			u32Diff = ((u32Next - u32Start) / 1000);
			dtNow  += u32Diff;
#if defined(TEST)
			Serial.printf("[clock] clock %02u:%02u:%02u (%lu sec)\n",
			              dtNow.Hour(), dtNow.Minute(), dtNow.Second(), u32Diff);
#endif
		}

		// check for overrun
		if (u8Day != dtNow.Day())
		{
			eEvent = CClock::eEventDay;
			DebugOut("[clock] new day starts");
		}
		else
		if (u8Hour != dtNow.Hour())
		{
			eEvent = CClock::eEventHour;
			DebugOut("[clock] new hour starts");
		}
		else
		{
			eEvent = CClock::eEventMinute;
			DebugOut("[clock] new minute starts");
		}

		// check for alarm
		if (boAlarmEnable)
		{
			// check alarm
			boAlarm = (((au8Alarm[0]     == dtNow.Hour()) && (au8Alarm[1]     == dtNow.Minute())) ||
			           ((au8AlarmNext[0] == dtNow.Hour()) && (au8AlarmNext[1] == dtNow.Minute())));
			if (boAlarm)
			{       
				DebugOut("[clock] alarm event");
			}
		}

		// read hardware clock?
		if (boHWClock && (eEvent >= CClock::eEventHour))
		{
			DebugOut("[clock] read hardware clock");
			dtNow = s_clsHWClock.GetDateTime();
		}

		// set new update cycle
		u32Start = millis();
		u32Next  = u32Start + ((60 - dtNow.Second()) * 1000);
#if defined(TEST)
		Serial.printf("[clock] clock %02u:%02u:%02u (%lu -> %lu)\n",
		              dtNow.Hour(), dtNow.Minute(), dtNow.Second(),
			      u32Start, u32Next);
#endif
	}

	// return event
	return eEvent;
}

//----------------------------------------------------------------------------
// enable or disable alarm
//----------------------------------------------------------------------------
bool CClock::EnableAlarm(const bool boEnable /*= true*/)
{
	bool boReturnValue = boAlarmEnable;

	boAlarm         = false;
	boAlarmEnable   = boEnable;
	au8AlarmNext[0] = 0xFF;
	au8AlarmNext[1] = 0xFF;
	DebugOut(boEnable ? "[clock] enable alarm" : "[clock] disable alarm");
	SaveAlarm();
	return boReturnValue;
}

//----------------------------------------------------------------------------
// get alarm time
//----------------------------------------------------------------------------
const char* CClock::GetAlarmTime()
{
	if (boAlarmEnable)
	{
		if ((au8AlarmNext[0] < 24) && (au8AlarmNext[1] < 60))
		{
			snprintf(szAlarmTime, sizeof(szAlarmTime),
			         "*%2u:%02u", au8AlarmNext[0], au8AlarmNext[1]);
		}
		else
		{
			snprintf(szAlarmTime, sizeof(szAlarmTime),
			         " %2u:%02u", au8Alarm[0], au8Alarm[1]);
		}
	}
	else
	{
		strncpy(szAlarmTime, "      ", sizeof(szAlarmTime));
	}

	return (const char*)&szAlarmTime;
}

//----------------------------------------------------------------------------
// initialize clock
//----------------------------------------------------------------------------
bool CClock::Init()
{
	// initialize clock
	s_clsHWClock.Begin();
	delay(100);

	// initialize internal
	boAlarm         = false;
	boAlarmEnable   = false;
	boHWClock       = false;
	dtNow           = s_clsHWClock.GetDateTime();
	au8Alarm[0]     = 0;
	au8Alarm[1]     = 0;
	au8AlarmNext[0] = 0xFF;
	au8AlarmNext[1] = 0xFF;

	if (dtNow < sc_dtCompiled)
	{
		s_clsHWClock.SetDateTime(sc_dtCompiled);
		DebugOut("[clock] RTC with invalid date/time");
	}

	if (!s_clsHWClock.GetIsRunning())
	{
		s_clsHWClock.SetIsRunning(true);
		DebugOut("[clock] RTC stopped - starting");
		delay(500);
	}

	if (s_clsHWClock.GetIsRunning())
	{
		if ((s_clsHWClock.GetDateTime() > sc_dtCompiled) &&
		    (dtNow.Hour() < 24) && (dtNow.Minute() < 60))
		{
			DebugOut("[clock] RTC up and running");
			boAlarmEnable   = false;
			boAlarmValid    = true;
			au8Alarm[0]     = s_clsHWClock.GetMemory(ADDR_ALARM_MSB);
			au8Alarm[1]     = s_clsHWClock.GetMemory(ADDR_ALARM_LSB);

			if ((au8Alarm[0] < 24) && (au8Alarm[1] < 60))
			{
				boAlarmEnable = (s_clsHWClock.GetMemory(ADDR_ALARM_ENABLE) == 0xEA);
				boAlarmValid  = true;
#if defined(TEST)
				Serial.printf("[clock] alarm %s (%2u:%02u)\n",
					      (boAlarmEnable ? "on" : "off"),
					      au8Alarm[0], au8Alarm[1]);
			}
			else
			{
				DebugOut("[clock] alarm invalid");
#endif
			}


			if ((s_clsHWClock.GetMemory(ADDR_ALARM_ENABLE) != 0) &&
			    (au8Alarm[0] < 24) && (au8Alarm[1] < 60))
			{
				boAlarmEnable = true;
			}
			else
			{
				au8Alarm[0]   = 0;
				au8Alarm[1]   = 0;
			}

			au8AlarmNext[0] = 0xFF;
			au8AlarmNext[1] = 0xFF;
			boHWClock       = true;
		}
	}

	if (!boHWClock)
	{
		dtNow = sc_dtCompiled;
		DebugOut("[clock] no RTC available - emulation");
		EEPROM.begin(512);
		boAlarmEnable = false;
		boAlarmValid  = false;
		au8Alarm[0]   =  EEPROM.read(1);
		au8Alarm[1]   =  EEPROM.read(2);

		if ((au8Alarm[0] < 24) && (au8Alarm[1] < 60))
		{
			boAlarmEnable = (EEPROM.read(0) == 0xEA);
			boAlarmValid  = true;
#if defined(TEST)
			Serial.printf("[clock] alarm %s (%2u:%02u)\n",
			              (boAlarmEnable ? "on" : "off"),
				      au8Alarm[0], au8Alarm[1]);
		}
		else
		{
			DebugOut("[clock] alarm invalid");
#endif
		}

		EEPROM.end();				
	}

	// set new update cycle
	u32Start = millis();
	u32Next  = u32Start + ((60 - dtNow.Second()) * 1000);
#if defined(TEST)
	Serial.printf("[clock] init clock %02u:%02u:%02u (%lu -> %lu)\n",
	              dtNow.Hour(), dtNow.Minute(), dtNow.Second(),
		      u32Start, u32Next);
#endif
	// return success
	return true;
}

//----------------------------------------------------------------------------
// save alarm time
//----------------------------------------------------------------------------
bool CClock::SaveAlarm()
{

	if (boHWClock)
	{
		DebugOut("[clock] store alarm settings (RTC)");
		s_clsHWClock.SetMemory(ADDR_ALARM_MSB, au8Alarm[0]);
		s_clsHWClock.SetMemory(ADDR_ALARM_LSB, au8Alarm[1]);
		s_clsHWClock.SetMemory(ADDR_ALARM_ENABLE, boAlarmEnable ? 0xEA : 0x00);
		boAlarmValid = true;
	}
	else
	{
		DebugOut("[clock] store alarm settings (EEPROM)");
		EEPROM.begin(512);
		EEPROM.write(0, boAlarmEnable ? 0xEA : 0x00);
		EEPROM.write(1, au8Alarm[0]);
		EEPROM.write(2, au8Alarm[1]);
		delay(200);
		EEPROM.commit();
		EEPROM.end();
		boAlarmValid = true;
	}
}

//----------------------------------------------------------------------------
// set alarm
//----------------------------------------------------------------------------
bool CClock::SetAlarm(const int iHour, const int iMin)
{
	// disable alarm next
	au8AlarmNext[0] = 0xFF;
	au8AlarmNext[1] = 0xFF;

	// set alarm
	if ((iHour >= 0) && (iHour < 24) && (iMin >= 0) && (iMin < 60))
	{
		au8Alarm[0] = iHour;
		au8Alarm[1] = iMin;

		if (!boAlarmEnable)
		{
			EnableAlarm(true);
		}

		// return success
		SaveAlarm();
		return true;
	}

	// return failure
	return false;
}

//----------------------------------------------------------------------------
// set alarm relative
//----------------------------------------------------------------------------
bool CClock::SetAlarmRelative(const int iHourRel /*= 0*/, const int iMinRel /*= 1*/)
{
	// enable alarm?
	if (!boAlarmEnable)
	{
		EnableAlarm(true);
	}

	// disable alarm next
	au8AlarmNext[0] = 0xFF;
	au8AlarmNext[1] = 0xFF;

	// set alarm relative (minutes)
	if (iMinRel > 0)
	{
		au8Alarm[1]++;

		if (au8Alarm[1] >= 60)
		{
			au8Alarm[1] = 0;
			au8Alarm[0] = (au8Alarm[0] < 23) ? (au8Alarm[0] + 1) : 0;
		}
	}
	else
	if (iMinRel < 0)
	{
		au8Alarm[1]--;

		if (au8Alarm[1] >= 60)
		{
			au8Alarm[1] = 59;
			au8Alarm[0] = (au8Alarm[0] > 0) ? (au8Alarm[0] - 1) : 23;
		}
	}

	// set alarm relative (hours)
	if (iHourRel > 0)
	{
		au8Alarm[0] = (au8Alarm[0] < 23) ? (au8Alarm[0] + 1) : 0;
	}
	else
	if (iHourRel < 0)
	{
		au8Alarm[0] = (au8Alarm[0] > 0) ? (au8Alarm[0] - 1) : 23;
	}

	// return success
	SaveAlarm();
	return true;
}

//----------------------------------------------------------------------------
// set (hardware) clock
//----------------------------------------------------------------------------
bool CClock::SetClock()
{
	// set hardware clock
	if (boHWClock)
	{
		DebugOut("[clock] set hardware clock");
		s_clsHWClock.SetDateTime(dtNow);
	}

	// set new update cycle
	u32Start = millis();
	u32Next  = u32Start + ((60 - dtNow.Second()) * 1000);
#if defined(TEST)
	Serial.printf("[clock] set clock %02u:%02u:%02u (%lu -> %lu)\n",
	              dtNow.Hour(), dtNow.Minute(), dtNow.Second(),
		      u32Start, u32Next);
#endif

	// return success
	return true;
}

//----------------------------------------------------------------------------
// snooze alarm
//----------------------------------------------------------------------------
void CClock::Snooze(bool bo24Hours /*= false*/)
{
	boAlarm = false;

	if (boAlarmEnable)
	{
		if (bo24Hours)
		{
			DebugOut("[clock] snooze for 24h");
			au8AlarmNext[0] = 0xFF;
			au8AlarmNext[1] = 0xFF;
		}
		else
		if ((au8AlarmNext[0] < 24) && (au8AlarmNext[1] < 60))
		{
			DebugOut("[clock] snooze (next)");
			au8AlarmNext[1] += SnoozeMinutes;
			
			if (au8AlarmNext[1] >= 60)
			{
				au8AlarmNext[1] -= 60;
				au8AlarmNext[0]++;
				
				if (au8AlarmNext[0] >= 24)
				{
					au8AlarmNext[0] -= 24;
				}
			}
		}
		else
		{
			DebugOut("[clock] snooze (first)");
			au8AlarmNext[0] = au8Alarm[0];
			au8AlarmNext[1] = au8Alarm[1] + SnoozeMinutes;
			
			if (au8AlarmNext[1] >= 60)
			{
				au8AlarmNext[1] -= 60;
				au8AlarmNext[0]++;
				
				if (au8AlarmNext[0] >= 24)
				{
					au8AlarmNext[0] -= 24;
				}
			}
		}
	}
}

//----------------------------------------------------------------------------
// global variable for access
//----------------------------------------------------------------------------
CClock Clock;

//===| eof - end of file |====================================================
