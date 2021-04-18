//==============================================================================
// Beep - example for beep
// (c) 2019-2020 by Dirk Ohme
//==============================================================================

//---| definitions |------------------------------------------------------------
#undef TEST
//#define TEST

//---| includes |---------------------------------------------------------------
#if defined(ESP8266)
#  include <pgmspace.h>
#else
#  include <avr/pgmspace.h>
#endif
#include <Arduino.h>
#include <Clock.h>


//------------------------------------------------------------------------------
// setup routine
//------------------------------------------------------------------------------
void setup()
{
	bool boResult;
	
	// start serial communication
	Serial.begin(9600);
	Serial.println("");
	
	// initialize clock
	boResult = Clock.Init();
	Serial.println((boResult == false)  ? "Cannot initialize clock!" :
	               (Clock.HasHWClock()) ? "Clock uses hardware RTC" :
		                              "Clock uses software emulation");
}

//------------------------------------------------------------------------------
// main loop
//------------------------------------------------------------------------------
void loop()
{
	CClock::EEvent eEvent;
	
	// check clock event
	eEvent = Clock.CheckEvent();
	
	if (eEvent == CClock::eEventDay)
	{
		Serial.println("New day starts");
	}
	else
	if (eEvent == CClock::eEventHour)
	{
		Serial.println("New hour start");
	}
	else
	if (eEvent == CClock::eEventMinute)
	{
		Serial.printf("[%2u:%02u] (is %s)\n",
		              Clock.dtNow.Hour(),
			      Clock.dtNow.Minute(),
			      Clock.IsDay() ? "day" : "night");
	}

	// wait
	delay(500);
}

//===| eof - end of file |======================================================
