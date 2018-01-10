# SalatTime-Arduino-oled
An Arduino code that displays on a small 0.98" OLED SSD1306 Date/Time and also calculates and displays Salat Time for a given town

This code has math routines to compute sun position and muslims salat (prayer) time for a given town (latitude / longitude), DST and convention.
The hardware part is composed of an Arduino Nano, a RTC DS1307 and the SSD1306 OLED display to display the time, the date and prayer times.

**More info**
  - https://baghli.blogspot.com/2016/07/rtc-salattime-et-arduino.html
  - https://baghli.blogspot.com/2016/07/position-du-soleil-et-salattime-pour.html
  - http://salat-mosque.com/programme.php#Sources
  - check the conventions on http://salat-mosque.com/salattime.php  (in expert mode)

Here is the part of code you have to adapt to change for your town:

in **mainroutines.cpp**
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
in **stoled.ino**
```
//rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
```
uncomment to force the RTC to take the compilation date and time
