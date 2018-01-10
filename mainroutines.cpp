//---------------------------------------------------------------------------
/*
Compute sun position and Muslims Salat times for particular date
Copyright (C) 2006  Lotfi BAGHLI

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

*********************************************************
*** IF YOU ARE MUSLIM, PLEASE PRAY FOR MY FORGIVENESS ***
*********************************************************

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

// for debug purpose
//#define Verbose

//---------------------------------------------------------------------------
//#include <math.h>
#include <Streaming.h>

#include "mainroutines.h"

//---------------------------------------------------------------------------
//const double M_PI=3.1415926535897932384626433832795;
const PROGMEM double deg2rd = 0.017453292519943295769236907684886;
const PROGMEM double rd2deg = 57.295779513082320876798154814105;
const PROGMEM double heure2rd = 0.26179938779914943653855361527329;
const PROGMEM double rd2heure = 3.8197186342054880584532103209403;
const PROGMEM double SolarSiderealCorrection = 1.002738;

//---------------------------------------------------------------------------
	//ChangeVille();
	// 0x00055278, 348792 , 0xffffcc9f, -13153  		// { 1, "Tlemcen        ", 34.879200 , -1.315300  },
	const char CountryName[] PROGMEM = "Alg�rie";
	const char TownName[] PROGMEM = "Tlemcen";
	const double latitude = 34.88*deg2rd;
	const double longitude = -1.31*deg2rd;
	const int TimeZoneTown = 	1;
	const int Convention = 1;
	const int DST = 0;

	struct Astre Soleil;
	struct SalatTimeStruct SalatTime, SalatTimeC;
	struct SalatTimeDetStruct SalatT;
	double JD, T,  // Jour Julien, Julien Century : T,
				theta0, theta;  // theta0 : temps sideral � Greenwich
	struct DateUT date; // date en jours
	struct Heure heure;
	int TimeZone;
	static String SSbuf;

	const char day_0[] PROGMEM = "Lundi";
	const char day_1[] PROGMEM = "Mardi";
	const char day_2[] PROGMEM = "Mercredi";
	const char day_3[] PROGMEM = "Jeudi";
	const char day_4[] PROGMEM = "Vendredi";
	const char day_5[] PROGMEM = "Samedi";
	const char day_6[] PROGMEM = "Dimanche";
	const char* const Days[] PROGMEM = {day_0, day_1, day_2, day_3, day_4, day_5};

// extern struct SalatTimeConfigStruct SalatTimeConfig[NConvention];

//const struct SalatTimeConfigStruct SalatTimeConfig[] =
// {
//	//8 Convention Name
//	//8 Convention AsrSahfi/Hanafi FajrTwilightAngle IshaTwilightAngle Sunrise/SetAngle Isha_is_Maghrib_plus xx mn ZuhrFixe ZuhrHour(h)
//	//8 Convention Fajr  Chourouk  Zuhr  Asr  Maghrib  Isha
//     {"User defined",																1, 18  , 17  , 0.61, 0, 75, 0, 13.3333334,	{-5.0,-5.0, 5.0, 0.0, 5.0, 0.0} },
//     {"France : UOIF",															1, 12  , 12  , 0.61, 0,  0, 0, 0,						{-5.0, 0.0, 5.0, 0.0, 5.0, 5.0} },
//     {"Algerie : (Isha 90 mn after Magrib)",				1, 18  , 18  , 0.61, 1, 90, 0, 13.3333334,	{ 0.0, 0.0, 0.0, 0.0, 5.0, 0.0} },
//     {"Maroc : (18� et Isha 17 mn apr�s Magrib)",		1, 18  , 17  , 0.61, 0, 75, 0, 13.3333334,	{-5.0,-5.0, 5.0, 0.0, 5.0, 0.0} },
//     {"Egyptian General Organisation of Surveying", 1, 19.5, 17.5, 0.61, 0,  0, 0, 0,						{ 0.0, 0.0, 0.0, 0.0, 0.0, 0.0} }
// };
const PROGMEM struct SalatTimeConfigStruct SalatTimeConfig[] =
 {
  //8 Convention Name
  //8 Convention AsrSahfi/Hanafi FajrTwilightAngle IshaTwilightAngle Sunrise/SetAngle Isha_is_Maghrib_plus xx mn ZuhrFixe ZuhrHour(h)
  //8 Convention Fajr  Chourouk  Zuhr  Asr  Maghrib  Isha
	{ 1, 12.0, 12.0, 0.61,  0,  0,   0, 0.000000, 		{-5.0, 0.0, 5.0, 0.0, 5.0, 5.0} },	//"Fr UOIF", 
	{ 1, 18.0, 17.0, 0.61,  0, 90,   0, 13.333333, 		{0.0, 0.0, 0.0, 0.0, 5.0, 0.0} },		//"Alg�rie", 
	{ 1, 18.0, 18.0, 0.61,  0,  0,   0, 0.000000, 		{0.0, 0.0, 0.0, 0.0, 5.0, 5.0} },		//"Karachi", 
	{ 1, 15.0, 15.0, 0.61,  0,  0,   0, 0.000000, 		{0.0, 0.0, 0.0, 0.0, 0.0, 5.0} },		//"ISNA", 
	{ 1, 18.0, 17.0, 0.61,  0,  0,   0, 0.000000, 		{0.0, 0.0, 0.0, 0.0, 5.0, 5.0} },		//"WIL", 
	{ 1, 19.0, 19.0, 0.61,  1, 90,   0, 0.000000, 		{-1.0, 0.0, 0.0, 0.0, 1.0, 0.0} },	//"Um Ul-Qura", 
	{ 1, 19.5, 17.5, 0.61,  0,  0,   0, 0.000000, 		{0.0, 0.0, 0.0, 0.0, 0.0, 0.0} },		//"Egyptian", 
	{ 1, 18.0, 17.0, 0.61,  0, 75,   0, 13.333333, 		{-5.0, -5.0, 5.0, 0.0, 5.0, 0.0} },	//"Userdef", 
	{ 1, 18.0, 17.0, 0.61,  0, 75,   0, 13.333333, 		{-5.0, -5.0, 5.0, 0.0, 5.0, 0.0} },	//"Maroc", 
 };
//Sketch uses 16,630 bytes (54%) of program storage space. Maximum is 30,720 bytes.
//Global variables use 584 bytes (28%) of dynamic memory, leaving 1,464 bytes for local variables. Maximum is 2,048 bytes.
// avec les constantes
//Sketch uses 14,728 bytes (47%) of program storage space. Maximum is 30,720 bytes.
//Global variables use 558 bytes (27%) of dynamic memory, leaving 1,490 bytes for local variables. Maximum is 2,048 bytes.

//---------------------------------------------------------------------------
// Algorithms from 	http://www.jgiesen.de/SME/details/basics/meeus.htm#solar
//								 	http://www.jgiesen.de/SME/details/basics/index.htm
//  								http://stjarnhimlen.se/comp/riset.html#3
//									http://www.ummah.net/astronomy/saltime/
// modified by Lotfi BAGHLI on 15/01/2006 et 10/06/2006
//---------------------------------------------------------------------------

// Julian day: 86400 s, Julian year: 365.25 d, Julian Century: 36525 d
// Julian Day (valid from 1900/3/1 to 2100/2/28)
/*
double JulianDay (int date, int month, int year, double UT)
{
	if (month<=2) {month=month+12; year=year-1;}
	return floor(365.25*year) + floor(30.6001*(month+1)) - 15 + 1720996.5 + date + UT/24.0;
}
*/
//---------------------------------------------------------------------------
// Julian century depuis 01/01/2000 � 12h UT
double JulianCentury (int date, int month, int year, double UT)
{
	if (month<=2) {month=month+12; year=year-1;}
	double JC;
	long int liAux1, liAux2;
	liAux1 = 365.25*year;
	liAux2 = 30.6001*(month+1);
	liAux1 += liAux2 - 15 + date - 2451545;
	JC = (liAux1 + 1720996.5 + UT/24.0)/36525.0;
	return JC;
	//return(floor(365.25*year) + floor(30.6001*(month+1)) - 15 + 1720996.5 + date + UT/24.0 - 2451545.0)/36525.0;
} 
//---------------------------------------------------------------------------
//Solar Coordinates (according to: Jean Meeus: Astronomical Algorithms), accuracy of 0.01 degree 
double SoleilLongitude( double T)   // L
{
double M, L0, DL, L, Lm;
	M = 357.52910 + 35999.05030*T - 0.0001559*T*T - 0.00000048*T*T*T; // mean anomaly, degree
	L0 = 280.46645 + 36000.76983*T + 0.0003032*T*T; // mean longitude, degree 
	DL = (1.914600 - 0.004817*T - 0.000014*T*T)*sin(deg2rd*M)
				+ (0.019993 - 0.000101*T)*sin(deg2rd*2*M) + 0.000290*sin(deg2rd*3*M); 
	L=L0 + DL; // true longitude, deg sans modulo
	Lm=L-360*floor(L/360);
	if (Lm<0)	Lm+=360;
	return Lm*deg2rd; // true longitude, rd 
}
//---------------------------------------------------------------------------
// convert ecliptic longitude L (rd) to right ascension RA and declination delta
void eclipticL2RA_delta( double L, double * RA, double * delta)
{
double eps, X, Y, Z, R;
  eps = deg2rd*23.43999; // obliquity of ecliptic
	X = cos(L); Y = cos(eps)*sin(L); Z = sin(eps)*sin(L); R = sqrt(1.0-Z*Z); 
	*delta = atan2(Z, R); // in rd 
	*RA = 2*atan2(Y, X+R); //  in rd 
}
//---------------------------------------------------------------------------
//compute sidereal time at Greenwich (according to: Jean Meeus: Astronomical Algorithms ) 
double TempsSideral( double T)   // theta0
{
	double theta0, theta0m;
//	T siecle julien depuis 2000
//	theta0=280.46061837 + 360.98564736629*T*36525 + 0.000387933*T*T - T*T*T/38710000.0;
	theta0=- T*T*T/38710000.0;
	theta0+= 0.000387933*T*T;
	theta0+= 360.98564736629*T*36525;
	theta0+= 280.46061837;
	theta0m=theta0-360*floor(theta0/360);
	if (theta0m<0)	theta0m+=360;
	return theta0m*deg2rd;    // TempsSideralen rd
}
//---------------------------------------------------------------------------
//convert tau, delta to horizon coordinates of the observer (altitude h, azimuth az) , needs latitude : beta
void equatorial2horizontal( double beta, double tau, double delta, double * h, double * az)
{
	*h=asin( sin(beta)*sin(delta) + cos(beta)*cos(delta)*cos(tau));	// h en rd
	*az =atan2( -sin(tau), cos(beta)*tan(delta) - sin(beta)*cos(tau))-M_PI;	// az en rd du SUD
	if (*az<0)	*az+=2*M_PI;
}
//---------------------------------------------------------------------------
// check DST applies ou non
int CalcTimeZone( int a, int m, int j, int DST, int TimeZoneTown)
{      // ici on approxime TimeZoneTown avec TimeZone puisque �a n'a pas d'importance
int jjdebut, jjfin, mmdebut, mmfin, ji, jour, JourToSwitch;
double T;
	if (DST==2) {JourToSwitch=4; mmdebut=4; mmfin=9;} // dernier vendredi d'avril / septembre
			else		{JourToSwitch=6; mmdebut=3; mmfin=10;}// dernier dimanche de mars / octobre
// recherchedernier dimanche de mars et dernier dimanche d'octobre
// DST applys 26-Mar, 02:00h  �  29-Oct, 03:00h   pour 2006
	for (ji=31; ji>24; ji--)
		{
		T=JulianCentury (ji, mmdebut, a, 12);    // pour 12h
		jour=CalculeJour(T+(TimeZoneTown+12)/24.0/36525.0);
		if (jour==JourToSwitch) 	{	jjdebut=ji; ji=0; // dimanche  et on sort de la for
															}
		}
	for (ji=31; ji>24; ji--)
		{
		T=JulianCentury (ji, mmfin, a, 12);    // pour 12h
		jour=CalculeJour(T+(TimeZoneTown+12)/24.0/36525.0);
		if (jour==JourToSwitch) 	{	jjfin=ji; ji=0; // dimanche  et on sort de la for
															}
		}
	if (m>mmdebut && m<mmfin)	return true;
	if (m==mmdebut && j>=jjdebut)	return true;
	if (m==mmfin && j<jjfin)	return true;
	return false;
}
//---------------------------------------------------------------------------
//convert rad en deg min sec
String rd2degminsec( double angle)
{
double h, m, s;
int hi, mi;
	h=angle*rd2deg;
	hi=h;
	m=(h-hi)*60;
	mi=(h-hi)*60;
	s=(m-mi)*60;
	SSbuf = String(hi,	DEC)+F("�")+String(mi,	DEC)+F("'")+String(s,	DEC)+F("\""); //(SSbuf,"%3d�%02d'%02.0f\"", hi, mi,s);
	return SSbuf;
}
//---------------------------------------------------------------------------
#ifdef Verbose
//convert rad en h min sec
String rd2hminsec( double angle)
{
double h, m, s;
int hi, mi;
	h=rd2heure*angle;
	hi=h;
	m=(h-hi)*60;
	mi=(h-hi)*60;
	s=(m-mi)*60;
	SSbuf = String(hi,	DEC)+F("h")+String(mi,	DEC)+F("'")+String(s,	DEC)+F("\""); // SSbuf,"%02dh%02d'%02.0f\"", hi, mi,s);
	return SSbuf;
}
//---------------------------------------------------------------------------
//convert h en h min sec
String h2hminsec( double h)
{
double m, s;
int hi, mi;
	hi=h;
	m=(h-hi)*60;
	mi=(h-hi)*60;
	s=(m-mi)*60;
	SSbuf = String(hi,	DEC)+F("h")+String(mi,	DEC)+F("'")+String(s,	DEC)+F("\""); // SSbuf,"%02dh%02d'%02.0f\"", hi, mi,s);
	return SSbuf;
}
#endif
//---------------------------------------------------------------------------
//convert h en h min arondie
String h2hmiarrondie( double h)
{
double m, s;
int hi, mi;
	hi=h;
	m=(h-hi)*60;
	mi=(h-hi)*60;
	s=(m-mi)*60;
	if (s>=30)	{
							mi++;
							if (mi==60)	{
													mi=0; hi++;
													if (hi==24) { //alrs pb . on passe au jour suivant.. qui n'arrive jamais
																			return F("erreur : arrondi au jour suivant");
						                          }	
													}
							}
	SSbuf = String(hi,	DEC)+F(":")+String(mi,	DEC); //SSbuf, "%02d:%02d",hi,mi);
	return SSbuf;
}
//---------------------------------------------------------------------------
//convert h en h min arondie
void CalcSTHeureMinute( double h, byte & hi, byte & mi)
{
double m, s;
	hi=h;
	m=(h-hi)*60;
	mi=(h-hi)*60;
	s=(m-mi)*60;
	if (s>=30)	{
							mi++;
							if (mi==60)	{
													mi=0; hi++;
													if (hi==24) { //alrs pb . on passe au jour suivant.. qui n'arrive jamais
//																			return F("erreur : arrondi au jour suivant");
						                          }	
													}
							}
}
//---------------------------------------------------------------------------
//convert h en h min arondie
void CalcSalatT()
{
	CalcSTHeureMinute( SalatTimeC.Fajr,		SalatT.h[0], SalatT.m[0]);
	CalcSTHeureMinute( SalatTimeC.Zuhr,		SalatT.h[1], SalatT.m[1]);
	CalcSTHeureMinute( SalatTimeC.Asr,		SalatT.h[2], SalatT.m[2]);
	CalcSTHeureMinute( SalatTimeC.Maghrib,SalatT.h[3], SalatT.m[3]);
	CalcSTHeureMinute( SalatTimeC.Isha,		SalatT.h[4], SalatT.m[4]);
}
//---------------------------------------------------------------------------
//saisi la date
void GetDate()
{
//	scanf("%d/%d/%d", & date.j, & date.m, & date.a);
//	date.j=26; date.m=02; date.a=2008;
// debug
DateTime	now = rtc.now();
  date.a = now.year();
  date.m = now.month();
  date.j = now.day();
	#ifdef Verbose
	Serial << date.j <<F ("/") << date.m << F("/") << date.a <<endl;
	#endif
}
//---------------------------------------------------------------------------
//saisi l'heure
void GetHeure()
{
DateTime	now = rtc.now();
  heure.h = now.hour();
  heure.m = now.minute();
  heure.s = now.second();
	heure.ms=0;
//	heure.h=12; heure.m=00; heure.s=00;
	#ifdef Verbose
	Serial << heure.h << F(":") << heure.m << F(":") << heure.s <<endl;
	#endif
}
//---------------------------------------------------------------------------
void STinit()
{
	// initialisation...
	ReadConfigFile(); //l'attribution de la ville
// debug
	GetDate(); // lu de la RTCC
	#ifdef Verbose
	//Serial.println(date.j+String("/")+date.m+String("/")+date.a);
	//Serial.println(heure.h + String(":") + heure.m + String(":") +heure.s);
	#endif
}
//---------------------------------------------------------------------------
void ComputeZuhrHour()
{
double Z_GMT, GMST0;
	T=JulianCentury (date.j, date.m, date.a, 12);    // pour 12h
	#ifdef Verbose
	Serial << F("T=") << _FLOAT(T, 10) << F("  JC � 12h GMT") << endl;
	#endif
	Soleil.L=SoleilLongitude(T);
	eclipticL2RA_delta( Soleil.L, & Soleil.RA, & Soleil.delta);
	theta0=TempsSideral( T);
	GMST0=theta0-12.0*heure2rd;
	Z_GMT=rd2heure*(Soleil.RA-GMST0-longitude);
// iteration bis
	if (Z_GMT<0) 	{// si n�gatif, on se place � j-1
								Z_GMT+=24;
								T=JulianCentury (date.j-1, date.m, date.a, Z_GMT+24);    // pour LA vraie heure � GWT
								}
		else				T=JulianCentury (date.j, date.m, date.a, Z_GMT);    // pour LA vraie heure � GWT
	#ifdef Verbose
	Serial << F("T=") << _FLOAT(T, 10) << F("  JC � ") << h2hminsec(Z_GMT) << endl;
	#endif
	Soleil.L=SoleilLongitude(T);
	eclipticL2RA_delta( Soleil.L, & Soleil.RA, & Soleil.delta);
	theta0=TempsSideral( T);
	GMST0=theta0-Z_GMT*heure2rd;
	SalatTime.Zuhr=TimeZone+rd2heure*(Soleil.RA-GMST0-longitude);    // on le prend avec TimeZone
	if (SalatTime.Zuhr<0) SalatTime.Zuhr+=24;
	//Serial << "#Zuhr="<< _FLOAT(SalatTime.Zuhr,10) <<endl;
}
//---------------------------------------------------------------------------
//compute pour une des 5 prieres
int ComputeTwilightHour(int TypeSalat, double * SalatHour)
{
double costau;
int Offset;
	* SalatHour=0;
	switch (TypeSalat)
		{
		case idFajr : Soleil.h=-SalatTimeConfig[Convention].FajrTwilightAngle*deg2rd;
									Offset=-1;
									break;
		case idIsha : Soleil.h=-SalatTimeConfig[Convention].IshaTwilightAngle*deg2rd;
									Offset=+1;
									break;
		case idChourouk : Soleil.h=-SalatTimeConfig[Convention].Sunrise_setAngle*deg2rd;
									Offset=-1;
									break;
		case idMaghrib : Soleil.h=-SalatTimeConfig[Convention].Sunrise_setAngle*deg2rd;
									Offset=+1;
									break;
		}
	// commence par calculer au milieur de la journ�e le T et declinaison du soleil
	// iteration 1
	#ifdef Verbose
//	printf(" 1st iter %s\n", h2hminsec(SalatTime.Zuhr-TimeZone));
	#endif
	T=JulianCentury (date.j, date.m, date.a, SalatTime.Zuhr-TimeZone);    // pour LA vraie heure � GWT
	#ifdef Verbose
//	printf("T=%g JC � %s\n", T, h2hminsec(SalatTime.Zuhr-TimeZone));
	#endif
	Soleil.L=SoleilLongitude(T);
	eclipticL2RA_delta( Soleil.L, & Soleil.RA, & Soleil.delta);
	if (TypeSalat==idAsr)		{
													if (SalatTimeConfig[Convention].AsrShafi)		Soleil.h=atan(1.0/(1.0+tan(latitude-Soleil.delta)));   // Asr Shafi : length of shadow = length of object + noon shadow
																else			Soleil.h=atan(1.0/(2.0+tan(latitude-Soleil.delta)));   // Asr Hanafi : length of shadow =2x length of object + noon shadow
													Offset=+1;
													}
	costau =(sin(Soleil.h)- sin(latitude)*sin(Soleil.delta))/cos(latitude)/cos(Soleil.delta);
	#ifdef Verbose
//	printf(" costau=%g\n",costau);
	#endif
	if (costau>1.0)		{
										#ifdef Verbose
//										printf("ERROR : then the Sun is always above our altitude limit. If we were computing rise/set times, the Sun is then aboute the horizon continuously; we have Midnight Sun.\n");
										#endif
										return 1;}
	if (costau<-1.0)	{
										#ifdef Verbose
//										printf("ERROR : then the Sun is always below our altitude limit. One example is midwinter in the arctics, when the Sun never gets above the horizon.\n");
										#endif
										return -1;}
	if (costau>=-1.0 && costau<=1.0)
			{
			Soleil.tau=acos(costau)/SolarSiderealCorrection;
			* SalatHour=SalatTime.Zuhr + Offset*rd2heure*(Soleil.tau);
			#ifdef Verbose
//			printf(" SalatHour=%gh ie : %s\n", * SalatHour, h2hminsec(* SalatHour));
			#endif
			}
// corrected values, avec iterations
	#ifdef Verbose
//	printf("** corrected values ** :\n");
	#endif
	// iteration 2
	#ifdef Verbose
//	printf(" 2nd iter � %s\n", h2hminsec(* SalatHour-TimeZone));
	#endif
	T=JulianCentury (date.j, date.m, date.a, * SalatHour-TimeZone);    // pour LA vraie heure � GWT
	#ifdef Verbose
//	printf("T=%g JC � %s\n", T, h2hminsec(* SalatHour-TimeZone, SSbuf));
	#endif
	Soleil.L=SoleilLongitude(T);
	eclipticL2RA_delta( Soleil.L, & Soleil.RA, & Soleil.delta);
	if (TypeSalat==idAsr)		{
													if (SalatTimeConfig[Convention].AsrShafi)		Soleil.h=atan(1.0/(1.0+tan(latitude-Soleil.delta)));   // Asr Shafi : length of shadow = length of object + noon shadow
																else			Soleil.h=atan(1.0/(2.0+tan(latitude-Soleil.delta)));   // Asr Hanafi : length of shadow =2x length of object + noon shadow
													}
//	theta0=TempsSideral( T);
//	GMST0=theta0-heure2rd*(* SalatHour-TimeZone);
 	costau =(sin(Soleil.h)- sin(latitude)*sin(Soleil.delta))/cos(latitude)/cos(Soleil.delta);
	#ifdef Verbose
//	printf(" costau=%g\n",costau);
	#endif
	if (costau>1.0)		{
										#ifdef Verbose
//										printf("ERROR : then the Sun is always above our altitude limit. If we were computing rise/set times, the Sun is then aboute the horizon continuously; we have Midnight Sun.\n");
										#endif
										return 1;}
	if (costau<-1.0)	{
										#ifdef Verbose
//										printf("ERROR : then the Sun is always below our altitude limit. One example is midwinter in the arctics, when the Sun never gets above the horizon.\n");
										#endif
										return -1;}
	if (costau>=-1.0 && costau<=1.0)
			{
			Soleil.tau=acos(costau); ///SolarSiderealCorrection; pas pour les suivants
			* SalatHour=SalatTime.Zuhr + Offset*rd2heure*(Soleil.tau);
			#ifdef Verbose
//			printf(" SalatHour=%gh ie : %s\n", * SalatHour, h2hminsec(* SalatHour, SSbuf));
			#endif
			}
	// iteration 3
	#ifdef Verbose
//	printf(" 3rd iter � %s\n", h2hminsec(* SalatHour-TimeZone, SSbuf));
	#endif
	T=JulianCentury (date.j, date.m, date.a, * SalatHour-TimeZone);    // pour LA vraie heure � GWT
	#ifdef Verbose
//	printf("T=%g JC � %s\n", T, h2hminsec(* SalatHour-TimeZone, SSbuf));
	#endif
	Soleil.L=SoleilLongitude(T);
	eclipticL2RA_delta( Soleil.L, & Soleil.RA, & Soleil.delta);
	if (TypeSalat==idAsr)		{
													if (SalatTimeConfig[Convention].AsrShafi)		Soleil.h=atan(1.0/(1.0+tan(latitude-Soleil.delta)));   // Asr Shafi : length of shadow = length of object + noon shadow
																else			Soleil.h=atan(1.0/(2.0+tan(latitude-Soleil.delta)));   // Asr Hanafi : length of shadow =2x length of object + noon shadow
													#ifdef Verbose
//													printf("h=%g�  ie altitude %s\n", Soleil.h*rd2deg, rd2degminsec(Soleil.h, SSbuf));
													#endif
													}
//	theta0=TempsSideral( T);
//	GMST0=theta0-heure2rd*(* SalatHour-TimeZone);
	costau =(sin(Soleil.h)- sin(latitude)*sin(Soleil.delta))/cos(latitude)/cos(Soleil.delta);
	#ifdef Verbose
//	printf(" costau=%g\n",costau);
	#endif
	if (costau>1.0)		{
										#ifdef Verbose
//										printf("ERROR : then the Sun is always above our altitude limit. If we were computing rise/set times, the Sun is then aboute the horizon continuously; we have Midnight Sun.\n");
										#endif
										return 1;}
	if (costau<-1.0)	{
										#ifdef Verbose
//										printf("ERROR : then the Sun is always below our altitude limit. One example is midwinter in the arctics, when the Sun never gets above the horizon.\n");
										#endif
										return -1;}
	if (costau>=-1.0 && costau<=1.0)
			{
			Soleil.tau=acos(costau); ///SolarSiderealCorrection; pas pour les suivants
			* SalatHour=SalatTime.Zuhr + Offset*rd2heure*(Soleil.tau);
			#ifdef Verbose
//			printf(" SalatHour=%gh ie : %s\n", * SalatHour, h2hminsec(* SalatHour, SSbuf));
			#endif
			}
	return 0;
}
//---------------------------------------------------------------------------
//Appelle les 2 methodes pr�c�dentes pour calculer ttes les prieres
void ComputeSalatTime()
{
int erreur, DSTApplies;
	GetDate();

	DSTApplies=(DST !=0) && CalcTimeZone(date.a, date.m, date.j, DST, TimeZoneTown);
	if (DSTApplies) TimeZone=1+TimeZoneTown;
						else  TimeZone=TimeZoneTown;
	#ifdef Verbose
//	printf("_____________________________________________________________\n");
//	printf("Lieu : %s, %s\n", CountryName, TownName);
//	printf("latitude=%s   longitude=%s\n", rd2degminsec(latitude, SSbuf),rd2degminsec(longitude, SS2));
//	printf((DSTApplies ? "DST Applies": "DST does not apply\n"));
//	printf("Convention %d\n", Convention);
//	printf("____ Mawakit Salat _____\n");
//	printf(" at Date=: %d/%d/%d   TZ=%d\n", date.j, date.m, date.a, TimeZone);
	#endif
	// Calcul Zuhr
	ComputeZuhrHour();
	#ifdef Verbose
	Serial << F("meridien/Zuhr=") << _FLOAT(SalatTime.Zuhr,10) << F(" ie : ") << h2hminsec(SalatTime.Zuhr) <<endl;
	// lever / coucher
	//Serial << F("Sunrise / Sunset at -") << SalatTimeConfig[Convention].Sunrise_setAngle <<endl;
	#endif
	// Calcul Chourouk (lever)
	erreur=ComputeTwilightHour( idChourouk, & SalatTime.Chourouk);
//return; //debug low mem
	#ifdef Verbose
//	printf("Chourouk (Sunrise) at %gh ie : %s\n", SalatTime.Chourouk, h2hminsec(SalatTime.Chourouk, SSbuf));
//	printf("\n");
	#endif
	// Calcul Maghrib (coucher)
	erreur=ComputeTwilightHour( idMaghrib, & SalatTime.Maghrib);
	#ifdef Verbose
//	printf("Maghrib (Sunset) at %gh ie : %s\n", SalatTime.Maghrib, h2hminsec(SalatTime.Maghrib, SSbuf));
//	printf("\n");
	#endif
	// Calcul Fajr (Twilight)
	erreur=ComputeTwilightHour( idFajr, & SalatTime.Fajr);
	#ifdef Verbose
//	printf("Fajr (Twilight) at %gh ie : %s\n", SalatTime.Fajr, h2hminsec(SalatTime.Fajr, SSbuf));
//	printf("\n");
	#endif
	// Calcul Isha (Twilight) or Maghrib+
	if (SalatTimeConfig[Convention].Isha_is_Maghrib_plus)
						SalatTime.Isha=SalatTime.Maghrib + SalatTimeConfig[Convention].Isha_is_Maghrib_plus_min/60.0;
			else	erreur=ComputeTwilightHour( idIsha, & SalatTime.Isha);
	#ifdef Verbose
//	if (SalatTimeConfig[Convention].Isha_is_Maghrib_plus)
//											printf("Isha (Maghrib_plus %dmn) at %gh ie : %s\n", SalatTimeConfig[Convention].Isha_is_Maghrib_plus_min, SalatTime.Isha, h2hminsec(SalatTime.Isha, SSbuf));
//											else printf("Isha (Twilight) at %gh ie : %s\n", SalatTime.Isha, h2hminsec(SalatTime.Isha, SSbuf));
//	printf("\n");
	#endif
	// Calcul Asr (arccot(1+tan(B-D)))
	erreur=ComputeTwilightHour( idAsr, & SalatTime.Asr);
	#ifdef Verbose
//	printf("Asr (%s) at %gh ie : %s\n", SalatTimeConfig[Convention].AsrShafi>=0 ? "Shafi":"Hanafi", SalatTime.Asr, h2hminsec(SalatTime.Asr, SSbuf));
//	printf("\n");
	#endif

	// calcule les vraies heures de pri�re
	SalatTimeC.Fajr=SalatTime.Fajr +SalatTimeConfig[Convention].SalatTimeAdjust.Fajr/60.0;
	SalatTimeC.Chourouk=SalatTime.Chourouk +SalatTimeConfig[Convention].SalatTimeAdjust.Chourouk/60.0;
	if (SalatTimeConfig[Convention].ZuhrFixe)	SalatTimeC.Zuhr=SalatTimeConfig[Convention].ZuhrFixeHour;
			else SalatTimeC.Zuhr=SalatTime.Zuhr +SalatTimeConfig[Convention].SalatTimeAdjust.Zuhr/60.0;
	SalatTimeC.Asr=SalatTime.Asr +SalatTimeConfig[Convention].SalatTimeAdjust.Asr/60.0;
	SalatTimeC.Maghrib=SalatTime.Maghrib +SalatTimeConfig[Convention].SalatTimeAdjust.Maghrib/60.0;
	SalatTimeC.Isha=SalatTime.Isha  +SalatTimeConfig[Convention].SalatTimeAdjust.Isha/60.0;
	#ifdef Verbose
//	printf("_______ Mawakit Salat Exactes _______\n");
//	printf("Fajr    =%s%s%gmn  %s\n", h2hminsec(SalatTime.Fajr, SSbuf), SalatTimeConfig[Convention].SalatTimeAdjust.Fajr>=0 ? " +":" ", SalatTimeConfig[Convention].SalatTimeAdjust.Fajr, h2hminsec(SalatTimeC.Fajr, SS2));
//	printf("Chourouk=%s%s%gmn  %s\n", h2hminsec(SalatTime.Chourouk, SSbuf), SalatTimeConfig[Convention].SalatTimeAdjust.Chourouk>=0 ? " +":" ", SalatTimeConfig[Convention].SalatTimeAdjust.Chourouk, h2hminsec(SalatTimeC.Chourouk, SS2));
//	{
//	if (SalatTimeConfig[Convention].ZuhrFixe)	printf("Zuhr Fixed = %s\n", h2hminsec(SalatTimeC.Zuhr, SSbuf));
//		else printf("Zuhr    =%s%s%gmn  %s\n", h2hminsec(SalatTime.Zuhr, SSbuf), SalatTimeConfig[Convention].SalatTimeAdjust.Zuhr>=0 ? " +":" ", SalatTimeConfig[Convention].SalatTimeAdjust.Zuhr, h2hminsec(SalatTimeC.Zuhr, SS2));
//	}
//	printf("Asr     =%s%s%gmn  %s\n", h2hminsec(SalatTime.Asr, SSbuf), SalatTimeConfig[Convention].SalatTimeAdjust.Asr>=0 ? " +":" ", SalatTimeConfig[Convention].SalatTimeAdjust.Asr, h2hminsec(SalatTimeC.Asr, SS2));
//	printf("Maghrib =%s%s%gmn  %s\n", h2hminsec(SalatTime.Maghrib, SSbuf), SalatTimeConfig[Convention].SalatTimeAdjust.Maghrib>=0 ? " +":" ", SalatTimeConfig[Convention].SalatTimeAdjust.Maghrib, h2hminsec(SalatTimeC.Maghrib, SS2));
//	printf("Isha    =%s%s%gmn  %s\n", h2hminsec(SalatTime.Isha, SSbuf), SalatTimeConfig[Convention].SalatTimeAdjust.Isha>=0 ? " +":" ", SalatTimeConfig[Convention].SalatTimeAdjust.Isha, h2hminsec(SalatTimeC.Isha, SS2));
//	printf("_______ Mawakit Salat arrondi _______\n");
//
//	printf("Lieu : %s, %s\n", CountryName, TownName);
//	printf("latitude=%s   longitude=%s Convention %d\n", rd2degminsec(latitude, SSbuf), rd2degminsec(longitude, SS2), Convention);
//	printf("__ Date : %s %d/%d/%d   %s  TZ (include DST)=%d\n", Days[CalculeJour(T+(TimeZone+12)/24.0/36525.0)], date.j, date.m, date.a,
//																									DSTApplies ? "DST Applies": "DST does not apply", TimeZone);
//	printf("Fajr    =%s\n", h2hmiarrondie(SalatTimeC.Fajr, SSbuf));
//	printf("Chourouk=%s\n", h2hmiarrondie(SalatTimeC.Chourouk, SSbuf));
//	printf("Zuhr    =%s\n", h2hmiarrondie(SalatTimeC.Zuhr, SSbuf));
//	printf("Asr     =%s\n", h2hmiarrondie(SalatTimeC.Asr, SSbuf));
//	printf("Maghrib =%s\n", h2hmiarrondie(SalatTimeC.Maghrib, SSbuf));
//	printf("Isha    =%s\n", h2hmiarrondie(SalatTimeC.Isha, SSbuf));
//	printf("_____________________________________\n");
	#endif
	CalcSalatT();
//	Serial << F("Fajr    =") << h2hmiarrondie(SalatTimeC.Fajr) <<endl;
//	Serial << F("Chourouk=") << h2hmiarrondie(SalatTimeC.Chourouk) <<endl;
//	Serial << F("Zuhr    =") << h2hmiarrondie(SalatTimeC.Zuhr) <<endl;
//	Serial << F("Asr     =") << h2hmiarrondie(SalatTimeC.Asr) <<endl;
//	Serial << F("Maghrib =") << h2hmiarrondie(SalatTimeC.Maghrib) <<endl;
//	Serial << F("Isha    =") << h2hmiarrondie(SalatTimeC.Isha) <<endl;
	Serial << F("Fajr    =") << SalatT.h[0] << F(":") <<SalatT.m[0] <<endl;
	Serial << F("Chourouk=") << h2hmiarrondie(SalatTimeC.Chourouk) <<endl;
	Serial << F("Zuhr    =") << SalatT.h[1] << F(":") <<SalatT.m[1] <<endl;
	Serial << F("Asr     =") << SalatT.h[2] << F(":") <<SalatT.m[2] <<endl;
	Serial << F("Maghrib =") << SalatT.h[3] << F(":") <<SalatT.m[3] <<endl;
	Serial << F("Isha    =") << SalatT.h[4] << F(":") <<SalatT.m[4] <<endl;
}
//---------------------------------------------------------------------------
/*
void Calc_hazClick()
{
int DSTApplies;
	GetDate();
	GetHeure();
 // debug
	DSTApplies=(DST !=0) && CalcTimeZone(date.a, date.m, date.j, DST, TimeZoneTown);
	if (DSTApplies) TimeZone=1+TimeZoneTown;
						else  TimeZone=TimeZoneTown;
	#ifdef Verbose
//	printf("_____________________________________________________________\n");
//	printf("Lieu : %s, %s\n", CountryName, TownName);
//	printf("latitude=%g�   longitude=%g�\n", latitude*rd2deg, longitude*rd2deg);
//	printf("latitude=%s   longitude=%s\n", rd2degminsec(latitude, SSbuf), rd2degminsec(longitude, SS2));
	#endif
//???  if (date.j<0) {date.j+=???; date.m--}
	date.h=heure.h-TimeZone+heure.m/60.0+heure.s/3600.0;
	if (date.h<0) {date.h+=24; date.j--;}
// debug  fin
	#ifdef Verbose
	Serial << F("Date=") << date.j <<F ("/") << date.m << F("/") << date.a;
	Serial << F("Heure=") << date.h << (DSTApplies ? F (" DST Applies"): F (" DST does not apply")) << TimeZone << endl;
//	printf("__ Date : %s %d/%d/%d � %gh UT   %s  TZ (include DST)=%d\n", Days[CalculeJour(T+(TimeZone+12)/24.0/36525.0)], date.j, date.m, date.a,
//																									date.h, DSTApplies ? "DST Applies": "DST does not apply", TimeZone);
	#endif
	T=JulianCentury (date.j, date.m, date.a, date.h);
	#ifdef Verbose
//	printf("T=%g JC\n",T);
	#endif

	Soleil.L=SoleilLongitude(T);
	#ifdef Verbose
//	printf("Soleil Longitude L=%g� ie : %s\n", Soleil.L*rd2deg, rd2hminsec(Soleil.L, SSbuf));
	#endif
  eclipticL2RA_delta( Soleil.L, & Soleil.RA, & Soleil.delta);
  theta0=TempsSideral( T);
	theta=theta0+longitude;
	#ifdef Verbose
//	printf("theta0=%g� ie : %s\n", theta0*rd2deg, rd2hminsec(theta0, SSbuf));
//	printf("theta=%g�  ie heure sid�rale %s\n", theta*rd2deg, rd2hminsec(theta, SSbuf));
	#endif
	Soleil.tau=theta-Soleil.RA;
	#ifdef Verbose
//	printf("RA=%g� ie : %s\n", Soleil.RA*rd2deg, rd2hminsec(Soleil.RA, SSbuf));
//	printf("delta=%g�  ie DE %s\n", Soleil.delta*rd2deg, rd2degminsec(Soleil.delta, SSbuf));
//	printf("tau=%g�  ie angle horaire %s\n", Soleil.tau*rd2deg, rd2hminsec(Soleil.tau, SSbuf));
	#endif
	equatorial2horizontal( latitude, Soleil.tau, Soleil.delta, & Soleil.h, & Soleil.az);
	#ifdef Verbose
//	printf("az=%g� (orgine Sud) ie : azimut %s\n", Soleil.az*rd2deg, rd2degminsec(Soleil.az, SSbuf));
//	printf("------az=%g� (orgine Nord) ie -180 : azimut %s\n", Soleil.az*rd2deg-180, rd2degminsec(Soleil.az-M_PI, SSbuf));
//	printf("h=%g�  ie altitude %s\n", Soleil.h*rd2deg, rd2degminsec(Soleil.h, SSbuf));
	Serial << F("az=") << Soleil.az*rd2deg << F("� (orgine Sud) ie : azimut ") << rd2degminsec(Soleil.az) <<endl;
	Serial << F("------az=") << Soleil.az*rd2deg-180 << F("� (orgine Nord) ie -180 : azimut ") << rd2degminsec(Soleil.az-M_PI) <<endl;
	Serial << F("h=") << Soleil.h*rd2deg << F("�  ie altitude ") << rd2degminsec(Soleil.h) <<endl;
	#endif
}
*/
//---------------------------------------------------------------------------
void ReadConfigFile()
{
// cod� en constante
	#ifdef Verbose
	Serial.print(CountryName);
	Serial.print(F(" - "));
	Serial.println(TownName);

	Serial.print(F("lat="));
	Serial.print(latitude*rd2deg, DEC);
	Serial.print(F(" long="));
	Serial.println(longitude*rd2deg, DEC);

	Serial << F("latitude=") << rd2degminsec(latitude) << F(" longitude=") << rd2degminsec(longitude) << endl;

	Serial.print(F("Conv "));
	Serial.println(Convention, DEC);
	#endif
}
//---------------------------------------------------------------------------
int CalculeJour(double T) // � partir du siecle Julien
{
double jour=T*36525+5;   
	return jour-7*floor(jour/7);
}
//---------------------------------------------------------------------------
