# SalatTime-Arduino-oled
A clock displayed on a small 0.98" OLED SSD1306 and also calculate and displays Salat Time for the town

This code has a mathematical routines to compute sun position and salat time for a given town (latitude / longitude), DST and convention.
The hardware part is composed of an Arduino Nano, an RTC and the SSD1306 OLED display to dicplay the time, the date and prayer times.

## Unified Sensor Drivers ##

The following drivers are based on the Adafruit Unified Sensor Driver:

**More info**
  - https://baghli.blogspot.com/2016/07/rtc-salattime-et-arduino.html
  - https://baghli.blogspot.com/2016/07/position-du-soleil-et-salattime-pour.html
  - http://salat-mosque.com/programme.php#Sources
	- check the conventions on http://salat-mosque.com/salattime.php  (in expert mode)


```
/** change your town */
	const char CountryName[] PROGMEM = "Algeria";
	const char TownName[] PROGMEM = "Tlemcen";
	const double latitude = 34.88*deg2rd;
	const double longitude = -1.31*deg2rd;
	const int TimeZoneTown = 	1;
	const int Convention = 1;
	const int DST = 0;
```
