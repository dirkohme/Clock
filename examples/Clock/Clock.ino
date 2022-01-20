//==============================================================================
// Clock - sample for Arduino Clock class
// (c) 2019-2022 by Dirk Ohme
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
	delay(50);
	Serial.println("");
	
	// initialize clock
	boResult = MyClock.Init();
	Serial.printf("Initialize clock: %s\n", (boResult) ? "ok" : "FAILED");
}

//------------------------------------------------------------------------------
// main loop
//------------------------------------------------------------------------------
void loop()
{
	Clock::EEvent eEvent;
	
	// check clock event
	eEvent = MyClock.CheckEvent();
	
	if (eEvent == Clock::eEventDay)
	{
		Serial.println("New day starts");
	}
	else
	if (eEvent == Clock::eEventHour)
	{
		Serial.println("New hour start");
		MyClock.SaveClock();
	}
	else
	if (eEvent == Clock::eEventMinute)
	{
		Serial.printf("%s [%2u:%02u] (is %s)\n",
		              MyClock.GetDateStr(),
			      MyClock.Hour(),
			      MyClock.Minute(),
			      MyClock.IsDay() ? "day" : "night");
	}

	// wait
	delay(500);
}

//===| eof - end of file |======================================================
