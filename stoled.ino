
// SalatTime + RTC + Clock + OLED
// 24/01/2017

#include <Wire.h>
#include <RTClib.h>
#include <Streaming.h>
#include <SoftwareSerial.h>
#include <U8g2lib.h>

// https://github.com/olikraus/u8g2/wiki/u8g2setupcpp
const unsigned int text1_y0=30, text2_y0=60, text2_x1=0, text2_x2=58;
char text1[8], text2[8];  //
int displaypage=0;

// pas de u8x8 text mode mais du u8g2 graphic 2 lines scrolling
//U8G2_SSD1306_128X64_NONAME_F_SW_I2C u8g2(U8G2_R0, /* clock=*/ SCL, /* data=*/ SDA, /* reset=*/ U8X8_PIN_NONE);   // All Boards without Reset of the Display
U8G2_SSD1306_128X64_NONAME_2_HW_I2C u8g2(U8G2_R0, /* clock=*/ SCL, /* data=*/ SDA);   // (rotation, [reset [, clock, data]])
////U8X8_SSD1306_128X64_NONAME_SW_I2C u8x8(  /* clock=*/ SCL,   /* data=*/ SDA, /* reset=*/ U8X8_PIN_NONE);   // OLEDs without Reset of the Display
//U8X8_SSD1306_128X64_NONAME_HW_I2C  u8x8(U8X8_PIN_NONE);
// OLED sur I2C 0x3C
// RTC sur I2C  0x68  R/W D0/D1
// AT24C32 sur I2C 0x50  R/W A0/A1

#include "mainroutines.h"

const uint8_t BTTX = 3;  // RX sur BC04  avec diviseur de tension 5V*2/3
const uint8_t BTRX = 2;  // TX sur BC04
SoftwareSerial BTSerial(BTRX, BTTX); // RX, TX

//RTC_DS1307 rtc;
// int Oldih, Oldim, Oldis;
int quartsec, OldDay, OldMinute, dayMinutes, NextSalat;

int i;
DateTime now;
char time[30];

struct FLAGS {
      unsigned Recalcule  :   1;
      unsigned CheckDate  :   1;
      unsigned seconde     :  1;
      unsigned SeqAthan   :   1;
      unsigned Toggles    :   1;
      unsigned eteint     :   1;
      unsigned heures     :   1;
      unsigned displayPageTog     :   1;
    };
struct FLAGS Flags;
    
void setup()
{
  Serial.begin(115200); // ne sert qu'en TX pour debug
  BTSerial.begin(9600);
  BTSerial.listen(); // listen only on BT not on HW Serial
//    while (!Serial); // wait for serial attach
  u8g2.begin();
//  u8g2.setFont(u8g2_font_ncenB14_tr);  // choose a suitable font
//  u8g2.setFont(u8g2_font_inb30_mr);  // set the target font to calculate the pixel width
  u8g2.setFont(u8g2_font_logisoso24_tr);  // set the target font to calculate the pixel width
  u8g2.setFontMode(0);    // enable transparent mode, which is faster

  u8g2.firstPage();
  do {
		u8g2.drawUTF8(0,text1_y0,"SalatTime");  // write something to the internal memory
		u8g2.drawUTF8(0,text2_y0,"L. Baghli");  // write something to the internal memory
  } while ( u8g2.nextPage() );

  Wire.begin();
  rtc.begin();
  if (! rtc.isrunning()) {
    Serial.println(F("RTC NOT running!"));
    // following line sets the RTC to the date & time this sketch was compiled
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    // This line sets the RTC with an explicit date & time, for example to set
    // January 21, 2014 at 3am you would call:
    //rtc.adjust(DateTime(2016, 6, 29, 4, 7, 0));
    }
//rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
//rtc.adjust(DateTime(2016, 6, 29, 16, 55, 0));
//rtc.adjust(DateTime(2017, 3, 19, 23, 59, 0));
 Serial.flush();

  // Init SalatTime
  STinit();
  Flags.Recalcule=1;
  Flags.heures=0;
  Flags.eteint=0;
  Flags.displayPageTog=0;
  
  cli(); // Désactive l'interruption globale
  TCCR1A = 0;	// pas de PWM ou OCR
  TCCR1B = (0<<WGM13) | (1<<WGM12) | 4;	// Clear Timer on Compare match (CTC) mode, OCR1A= PRD
																				// CS12 CS11 CS10 = 0b100 = 4 =>256 prescaler
  OCR1A = 15625; // 62.5 ns * 15625 * 256 prescaler = 0.250 s
  // pas de compteur logiciel
  TIMSK1 = 1<<OCIE1A; // ISR on Output Compare1 (TCNT1==OCR1)
	TIFR1 = 0;	// clear T1 IF
  TCNT1 = 0; // RAZ T1
  sei(); // Active l'interruption globale
}
//----------------------------------------------------------------------------
void CheckTime()
{
  Flags.seconde = 0;
  now = rtc.now();
/*
  if (old.second() == now.second())  quartsec++;
    else quartsec = 0;
  old = now; 

    u8x8.drawString(0,1,now.hour());
    u8x8.drawString(2,1,":");  
    u8x8.drawString(3,1,now.minute()); 
    u8x8.drawString(5,1,":");  
    u8x8.drawString(6,1,now.second());
*/
  
Flags.Toggles = !Flags.Toggles;
//sprintf( time, "%8s %02hhu:%02hhu", "Maghrib", SalatT.h[3],SalatT.m[3] );
/*
u8x8.setInverseFont(Flags.Toggles);
u8x8.drawString(0,5,time);
u8x8.setInverseFont(0);
*/
  Serial << now.day() << F("/")<< now.month() << F("/")<<now.year()
          << F(" ")<< now.hour() << F(":")<<now.minute() << F(":")<<now.second() << endl;
          
  if (OldDay != now.day()) Flags.Recalcule = 1;  // recalcule ST
  if ((OldMinute != now.minute()) && (Flags.Recalcule == 0))
    {
    // une minute d'est écoulée
    if ((SalatT.m[NextSalat]==now.minute()) && (SalatT.h[NextSalat]==now.hour()))
      {// Athan
      Flags.SeqAthan = 1;  
      }
      else Flags.SeqAthan = 0;
    OldMinute = now.minute();
    dayMinutes = now.hour()*60+now.minute();
    for (i=4; i>=0; i--)
    if (SalatT.h[i]*60 + SalatT.m[i] >= dayMinutes)
      {
      NextSalat = i;
      Serial << F("SalatT.m[")<< i <<F("] =") << SalatT.m[i] << endl; 
//debug affiche heure next salat      
      }
    }
/*
  // BT Check INput
  if (BTSerial.available()) {
    String RxStr = BTSerial.readStringUntil('\n');
    if (RxStr[0]=='$') {
      Serial << RxStr;
      RxStr[0]='0';
      // $20,09,15,14,25,31
       char RxStrBuf[22], * pch;
       int dt[6], Stridx=0;
       RxStr.toCharArray(RxStrBuf, 22);
        pch = strtok (RxStrBuf,",");
        while ((pch != NULL) && (Stridx<6))
        {
          dt[Stridx++] = atoi(pch);
          pch = strtok (NULL, ",");
        }
      // int dt[] = int(split(RxStr.substring(1), ','));
      dt[2] += 2000;  // 2000+16
      Serial << "Rx:" << dt[0]<< F("/")<< dt[1] << F("/")<< dt[2]
          << F(" ")<< dt[3] << F(":")<< dt[4] << F(":")<< dt[5] << endl;
      MaJRTC(dt);
      
      }
 
    }
  // BT time output
  BTSerial << now.day() << F("/")<< now.month() << F("/")<<now.year()
          << F(" ")<< now.hour() << F(":")<<now.minute() << F(":")<<now.second() << endl;  */ 
}
//----------------------------------------------------------------------------
void MaJRTC(int * dt)
{
// verifie la validité
  if ((dt[0]<1) || (dt[0]>31)) return;  
  if ((dt[1]<1) || (dt[1]>12)) return;  
  if ((dt[2]<2000) || (dt[2]>2099)) return;  
  if ((dt[3]<0) || (dt[3]>23)) return;  
  if ((dt[4]<0) || (dt[4]>59)) return;  
  if ((dt[5]<0) || (dt[5]>59)) return; 
  // set RTC 
  rtc.adjust(DateTime(dt[2], dt[1], dt[0], dt[3], dt[4], dt[5]));
}
//----------------------------------------------------------------------------
 
// Routine d'interruption
ISR(TIMER1_COMPA_vect) {  // 250 ms sont passés, IF RAZ automatiquement
/*  	
	if (! Flags.eteint) {
		// vert pour les ST
		for (i=0; i<5; i++)    strip.SetPixelColor(iSalatT[i], green);
		strip.SetPixelColor(iSalatTminute, ltgreen);  
												 
		// temps
    strip.SetPixelColor(id, orange);
		strip.SetPixelColor(ih, red);
		if (im != ih) strip.SetPixelColor(im, violet);
			else        strip.SetPixelColor(im, violetred);
		if (is != ih) strip.SetPixelColor(is, blue);
			else        strip.SetPixelColor(is, violetred);
		if (is != im) strip.SetPixelColor(is, blue);
			else        strip.SetPixelColor(is, violetred);
		// Athan
		
		if (Flags.heures)  for (i=0; i<24; i++)    strip.SetPixelColor(10*i, pink);
		}


	Serial << quartsec << F("  is=") << is << endl;
*/
  if (Flags.SeqAthan)
      {
      Flags.Toggles = !Flags.Toggles;
//      if (Flags.Toggles)  strip.SetPixelColor(iSalatT[NextSalat], yellow);  
//        else   strip.SetPixelColor(iSalatT[NextSalat], green);
      }
  if (++quartsec == 4)  {
  		quartsec = 0;
  		Flags.seconde = 1;
  	}
//  if ((quartsec == 0)||(quartsec == 2))  {
  if ((quartsec == 0) )  {
		Flags.displayPageTog = 1;
  	}
}
//----------------------------------------------------------------------------
void DisplayPages()
{
	if (++displaypage>5)	displaypage=0;
	switch(displaypage)
		{
		case 0: sprintf( text1, "Fajr");
						sprintf( text2, "%02hhu:%02hhu", SalatT.h[0],SalatT.m[0] );
				break;
		case 1: sprintf( text1, "Zuhr");
						sprintf( text2, "%02hhu:%02hhu", SalatT.h[1],SalatT.m[1] );
				break;
		case 2: sprintf( text1, "Asr");
						sprintf( text2, "%02hhu:%02hhu", SalatT.h[2],SalatT.m[2] );
				break;
		case 3: sprintf( text1, "Magr");
						sprintf( text2, "%02hhu:%02hhu", SalatT.h[3],SalatT.m[3] );
				break;
		case 4: sprintf( text1, "Isha");
						sprintf( text2, "%02hhu:%02hhu", SalatT.h[4],SalatT.m[4] );
				break;
		case 5: sprintf( text1, "%02hhu/%02hhu/%02hhu", now.day(),now.month(),now.year()%100 );
						sprintf( text2, "");
				break;
		}
  sprintf( time, "%02hhu:%02hhu:%02hhu", now.hour(), now.minute(), now.second() );

	u8g2.firstPage();
	do {
		u8g2.drawUTF8(0,text1_y0,time);  // write something to the internal memory
		u8g2.drawUTF8(text2_x1,text2_y0, text1); 
		u8g2.drawUTF8(text2_x2,text2_y0, text2);
	} while ( u8g2.nextPage() );
	Flags.displayPageTog = 0;
}
//----------------------------------------------------------------------------
void loop()
{
  if (Flags.seconde)  CheckTime();
  if (Flags.Recalcule)  {
                        ComputeSalatTime();
                        /*
                        Serial <<  << SalatT.h[0] << F(":") <<SalatT.m[0] <<endl;
                        Serial << F("Chourouk=") << h2hmiarrondie(SalatTimeC.Chourouk) <<endl;
                        Serial << F("Zuhr    =") << SalatT.h[1] << F(":") <<SalatT.m[1] <<endl;
                        Serial << F("Asr     =") << SalatT.h[2] << F(":") <<SalatT.m[2] <<endl;
                        Serial << F("Maghrib =") << SalatT.h[3] << F(":") <<SalatT.m[3] <<endl;
                        Serial << F("Isha    =") << SalatT.h[4] << F(":") <<SalatT.m[4] <<endl;
                        */  
												/*
                        sprintf( time, "%02hhu/%02hhu/%04hhu", now.day(),now.month(),now.year() );
                        u8x8.drawString(4,7,time);
                        sprintf( time, "%8s %02hhu:%02hhu", "Fajr", SalatT.h[0],SalatT.m[0] );
                        u8x8.drawString(0,2,time);
                        sprintf( time, "%8s %02hhu:%02hhu", "Zuhr", SalatT.h[1],SalatT.m[1] );
                        u8x8.drawString(0,3,time);
                        sprintf( time, "%8s %02hhu:%02hhu", "Asr", SalatT.h[2],SalatT.m[2] );
                        u8x8.drawString(0,4,time);
                        sprintf( time, "%8s %02hhu:%02hhu", "Maghrib", SalatT.h[3],SalatT.m[3] );
                        u8x8.drawString(0,5,time);
                        sprintf( time, "%8s %02hhu:%02hhu", "Isha", SalatT.h[4],SalatT.m[4] );
width2 = u8g2.getUTF8Width(icystreamtitle.c_str());    // calculate the pixel width of the text
strncpy ( text2, icystreamtitle.c_str(), text2_len) ;                         // Make a local copy of the string

                        u8x8.drawString(0,6,time);
  */
                        OldDay = now.day();
                        OldMinute = 61;
                        
                        Flags.Recalcule = 0;
                        }
  if (Flags.displayPageTog)  DisplayPages();
}

