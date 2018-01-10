//---------------------------------------------------------------------------
#ifndef MAINROUTINES
#define MAINROUTINES

#include "Arduino.h"
#include <RTClib.h>

extern RTC_DS1307 rtc;

#define Verbose

//---------------------------------------------------------------------------
#define idFajr 0
#define idChourouk 1
#define idZuhr 2
#define idAsr 3
#define idMaghrib 4
#define idIsha 5


#define NConvention 9

//-----------------------------------------------------------------------------
struct Astre  { double az,  h,      // coord. horizontal
                delta,  tau, // coord equatorial
                RA,                // ascension droite
								L;                //longitude réele
							};
//-----------------------------------------------------------------------------
struct SalatTimeStruct { double Fajr, Chourouk, Zuhr, Asr, Maghrib, Isha;};
struct SalatTimeDetStruct { byte h[5], m[5];};
//-----------------------------------------------------------------------------
struct SalatTimeConfigStruct {
//					char Name[NameConventionlenght];
					int AsrShafi;
					double FajrTwilightAngle, IshaTwilightAngle, Sunrise_setAngle;
					int Isha_is_Maghrib_plus;
					int Isha_is_Maghrib_plus_min;
					int ZuhrFixe;
					double ZuhrFixeHour;
					struct SalatTimeStruct SalatTimeAdjust;
					} ;
//-----------------------------------------------------------------------------

struct DateUT  { int j, m, a; double h; // jour/mois/année/heure.xxx
							};
struct Heure  { int ms, s, m, h;  // milisec, secondes/, minutes, heures
							};
//---------------------------------------------------------------------------
	double JulianDay (int date, int month, int year, double UT);
	//double rang_du_jour (int date, int month, int year);
	double JulianCentury (int date, int month, int year, double UT);
	double SoleilLongitude( double T);
	void eclipticL2RA_delta( double L, double * RA, double * delta);
	double TempsSideral( double T);
	void equatorial2horizontal( double beta, double tau, double delta, double * h, double * az);
	int CalcTimeZone( int a, int m, int j, int DST, int TimeZoneTown);
	String rd2degminsec( double angle);
#ifdef Verbose
	String rd2hminsec( double angle);
	String h2hminsec( double h);
#endif
	String h2hmiarrondie( double h);
	int CalculeNjdumois(int m, int annee);
	int CalculeJour(double T);
	void GetDate();
	void GetHeure();
//---------------------------------------------------------------------------
	void ReadConfigFile();
	void ComputeSalatTime();
//	void Calc_hazClick();
	void About1Click();
	void ComputeZuhrHour();
	int ComputeTwilightHour(int TypeSalat, double * SalatHour);
	void STinit();
//---------------------------------------------------------------------------
		extern struct Astre Soleil;
		extern struct SalatTimeStruct SalatTime, SalatTimeC;
		extern  struct SalatTimeDetStruct SalatT;
		extern double JD, T,  // Jour Julien, Julien Century : T,
					theta0, theta;  // theta0 : temps sideral à Greenwich
		extern struct DateUT date; // date en jours
		extern struct Heure heure; // heure (pour la hauteur du soleil)
//		extern const String CountryName, TownName;
//		extern const double longitude, latitude;
//		extern const int TimeZoneTown, Convention, DST;
		extern int TimeZone;
//		extern const PROGMEM char * Days[];
//		extern const struct SalatTimeConfigStruct SalatTimeConfig[];
//---------------------------------------------------------------------------
#endif
