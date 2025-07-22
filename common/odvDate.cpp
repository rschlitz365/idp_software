/****************************************************************************
**
** Copyright (C) 1993-2025 Reiner Schlitzer. All rights reserved.
**
** This file is part of Ocean Data View.
**
** Global functions: date.cpp
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/
#include "common/odvDate.h"

#include <stdio.h>		// sprintf
#include <string.h>
#include <math.h>
#include <time.h>

#include <QLocale>
#include <QString>


/**************************************************************************/
void dateFromDecimalYear(double decYear,int& year,int& month,int& day,
			 int& hour,int& minute,double& sec)
/**************************************************************************/
/*!
  \brief Converts a decimal time \a decYear (in decimal years) to a
  Gregorian date consisting of year \a year, month \a month, day \a
  day and daytime \a hour, \a minute and \a sec.

  Returns the date & time values in the parameter references.
*/
{
  double dd,hh;

  year=(int) decYear; dd=(decYear-year)*gregorianDaysInYear(year);
  gregorianDateInYear(year,(int) dd+1,month,day);
  hh=(dd-(int) dd)*24.; hour=(int) hh;
  minute=(int) ((hh-hour)*60.); sec=(hh-(hour+minute/60.))*60.*60.;
  validateDate(year,month,day,hour,minute,sec);
}

/**************************************************************************/
void dateFromGregorianDay(double gregDay,int& year,int& month,int& day,
			  int& hour,int& minute,double& sec)
/**************************************************************************/
/*!

  \brief Converts a given fractional Gregorian Day \a gregDay to a
  Gregorian date consisting of \a year , \a month , \a day and daytime
  \a hour, \a minute and \a sec.

  Returns the date & time values in the parameter references.
*/
{
  double gDay=floor(gregDay),fracDay=gregDay-gDay;
  gregorianDate((int) gDay,year,month,day);
  daytimeFromFractionalDay(fracDay,hour,minute,sec);
}

/**************************************************************************/
void dateFromJulianDay(double julDay,int& year,int& month,int& day,
		       int& hour,int& minute,double& sec,bool isChronological)
/**************************************************************************/
/*!

  \brief Converts a Chronological Julian Day \a julDay to a Gregorian
  date consisting of \a year , \a month , \a day and daytime \a hour,
  \a minute and \a sec.

  \a julDay is considered to be a Chronological Julian Date if \a
  isChronological is true (the default). Otherwise, \a julDay is
  considered to be an Astronomical Julian Date.

  For the definition of CJD see
  <a href="http://www.hermetic.ch/cal_stud/chron_jdate.htm">here</a>.

  The chronological Julian date in the GMT timezone is the number of
  days and fraction of a day which have elapsed since midnight GMT at
  the start of -4712-01-01 in the proleptic Julian Calendar. For
  example, for 17:13 GMT on 2007-01-19 CE the corresponding
  chronological Julian date is 2454120.7176.
  \ifnot API
  (test OK. RS 2011-02-15)
  \endif

  The chronological Julian date at a particular timezone is the number
  of days and fraction of a day which have elapsed since midnight in
  that timezone at the start of -4712-01-01 in the proleptic Julian
  Calendar. For example, for 01:13 Beijing standard time on 2007-01-20
  CE the corresponding chronological Julian date is 2454121.0509.

  Returns the date & time values in the parameter references.
*/
{
  /* if we have an Astronomical Julian Date add half a day to turn it
     into a Chronological Julian Date before converting */
  double jd=(isChronological)? julDay:julDay+0.5;

  int i,j,l,n;

  l=(int) jd+68569;
  n=4*l/146097;
  l=l-(146097*n+3)/4;
  i=(4000*(l+1))/1461001;
  l=l-(1461*i)/4+31;
  j=(80*l)/2447;
  day=l-(2447*j)/80;
  l=j/11;
  month=j+2-(12*l);
  year=100*(n-49)+i+l;

  daytimeFromFractionalDay(jd-(int)jd,hour,minute,sec);
  validateDate(year,month,day,hour,minute,sec);
}

/**************************************************************************/
QString dateString(ODV::DateFormat dateFormat,int year,int month,int day)
/**************************************************************************/
/*!
  \brief \return The date string given by \a year, \a sMonth, and \a sDay
  in \a dateFormat format.
*/
{
  double dY=(year<=ODV::missINT32) ? ODV::missDOUBLE : (double) year;
  double dM=(month<=ODV::missINT32) ? ODV::missDOUBLE : (double) month;
  double dD=(day<=ODV::missINT32) ? ODV::missDOUBLE : (double) day;
  return dateString(dateFormat,dY,dM,dD);
}

/**************************************************************************/
QString dateString(ODV::DateFormat dateFormat,
		   double dYear,double dMonth,double dDay)
/**************************************************************************/
/*!

  \brief \returns The date string given by \a dYear, \a dMonth, and \a dDay
  in \a dateFormat format.

  \sa dateTimeIsoString(), timeString()

*/
{
  /* check validity of day and month values */
  if (dMonth<1. || dMonth>12.) dMonth=ODV::missDOUBLE;
  if (dDay<1. || dDay>31.) dDay=ODV::missDOUBLE;

  /* obtain string representation of day, month, and year values */
  QString D=(dDay!=ODV::missDOUBLE) ?
    QString("%1").arg((int) dDay,2,10,QChar('0')) : "  ";
  QString M=(dMonth!=ODV::missDOUBLE) ?
    QString("%1").arg((int) dMonth,2,10,QChar('0')) : "  ";
  QString Y=(dYear!=ODV::missDOUBLE) ?
    QString("%1").arg((int) dYear,4,10,QChar('0')) : "    ";

  QString s;
  switch(dateFormat)
    {
      case ODV::IsoDate:
	if (dYear!=ODV::missDOUBLE)
	  {
	    s=Y;
	    if (dMonth!=ODV::missDOUBLE)
	      {
		s+=QString("-%1").arg(M);
		if (dDay!=ODV::missDOUBLE) s+=QString("-%1").arg(D);
	      }
	  }
	return s.simplified();
      case ODV::mmddyyyyDate:
	return QString("%1/%2/%3").arg(M).arg(D).arg(Y).simplified();
    case ODV::yyyymmddDate:
	return QString("%1%2%3").arg(Y).arg(M).arg(D).simplified();
      case ODV::ddmonthyyyyDate:
	M=(dMonth!=ODV::missDOUBLE) ?
	  QString("%1").arg(QLocale("C").monthName((int)(dMonth))) : "  ";
	return QString("%1 %2 %3").arg(D).arg(M).arg(Y).simplified();
      case ODV::ddmmmyyyyDate:
	M=(dMonth!=ODV::missDOUBLE) ? QString("%1")
	  .arg(QLocale("C").monthName((int)(dMonth),QLocale::ShortFormat)) : "  ";
	return QString("%1 %2 %3").arg(D).arg(M).arg(Y).simplified();
      case ODV::mmmddyyyyDate:
      default:
	M=(dMonth!=ODV::missDOUBLE) ? QString("%1")
	  .arg(QLocale("C").monthName((int)(dMonth),QLocale::ShortFormat)) : "  ";
	return QString("%1 %2 %3").arg(M).arg(D).arg(Y).simplified();
    }
}

/**************************************************************************/
QDateTime dateTimeFromString(const QString &dateTimeString,
			     const QString &format)
/**************************************************************************/
/*!

  \brief Extracts date and time from string \a dateTimeString using
  format \a format.

  The default for \a format is 'MMM dd yyyy hh:mm:ss' where 'MMM'
  represents the month as american english abbreviation (e.g. May).

  \return A QDateTime object that holds date and time.  An invalid
  date/time is returned if the extraction fails.

*/
{
  QLocale loc(QLocale::English,QLocale::UnitedStates);
  return loc.toDateTime(dateTimeString,format);
}

/**************************************************************************/
QString dateTimeIsoString(double dYear,double dMonth,double dDay,
			  double dHH,double dMM,double dSS)
/**************************************************************************/
/*!

  \returns The ISO8601 date and time string for \a dYear, \a dMonth,
  \a dDay, \a dHH, \a dMM, and \a dSS.

  \sa dateString(), timeString()

*/
{
  QString d=dateString(ODV::IsoDate,dYear,dMonth,dDay);
  QString t=timeString(ODV::IsoTime,dHH,dMM,dSS);
  if (!t.isEmpty()) d+=QString("T")+t;
  return d;
}

/**************************************************************************/
void daytimeFromFractionalDay(double fracDay,int& hour,int& minute,double& sec)
/**************************************************************************/
/*!
  \brief Retrieves daytime \a hour, \a minute, and \a sec from
  fractional day \a fracDay.

  \a fracDay must be between 0 and 1.
*/
{
  double hh=fracDay*24.; hour=floor(hh);
  minute=floor((hh-hour)*60.); sec=(hh-(hour+minute/60.))*60.*60.;
}

/**************************************************************************/
double decimalDay(int hour,int minute,double sec)
/**************************************************************************/
/*!

  \brief Calculates and returns the fractional day from daytime \a
  hour, \a minute, and \a sec.

  \note Invalid input values are set to zero.

  Fractional day values are between 0 and 1.

*/
{
  /* check input values */
  if (hour<0 || hour>23 || minute<0 || minute>59) { hour=minute=0; }
  if (sec<0. || sec>=60.) sec=0.;

  double dDay=((double) hour+minute/60.+sec/3600.)/24.;

  return dDay;
}

/**************************************************************************/
double decimalYear(int year,int month,int day,int hour,int minute,double sec)
/**************************************************************************/
/*!
  \returns Time (in decimal years) for a given calendar date & time.

  \c ODV::missDOUBLE is returned for invalid date. Invalid \a hour, \a
  minute, or \a sec values are set to zero.

*/
{
  /* check whether date is valid. for year we are comparing
     <=ODV::missINT32 to catch (int) casts of ODV::missDOUBLE values */
  if (year<=ODV::missINT32 || month<1 || month>12 || day<1 || day>31)
    return ODV::missDOUBLE;

  double daysInYear=(isGregorianLeapYear(year)) ? 366. : 365.;
  double days=(double) gregorianDayOfYear(year,month,day)-1.;

  if (hour<0 || hour>23 || minute<0 || minute>59) { hour=minute=0; }
  if (sec<0. || sec>=60.) sec=0.;

  /* add day-time contribution */
  days+=decimalDay(hour,minute,sec);

  return (double) year+days/daysInYear;
}

/**************************************************************************/
double decimalYearFromGregorianDay(int gregDay)
/**************************************************************************/
/*!
  \returns Time (in decimal years) for a given Gregorian day \a gregDay.
*/
{
  if (gregDay<=ODV::missINT32) return ODV::missDOUBLE;
  int year,mon,day; gregorianDate(gregDay,year,mon,day);
  return decimalYear(year,mon,day,0,0);
}

/**************************************************************************/
double decimalYearFromGregorianDay(double gregDay)
/**************************************************************************/
/*!
  \returns Time (in decimal years) for a given Gregorian day \a gregDay
  including daytime as fractional part.
*/
{
  if (gregDay==ODV::missDOUBLE) return ODV::missDOUBLE;
  int iDay=(int) gregDay; double frDay=gregDay-iDay; int year,mon,day;
  gregorianDate(iDay,year,mon,day);
  return decimalYear(year,mon,day,0,0)+frDay/gregorianDaysInYear(year);
}

/**************************************************************************/
void getDateAndTime(char* szDT)
/**************************************************************************/
/*!
  \brief Retrieves the current date and time in \a szDT.

  \a szDT must be at least 26 characters long.
*/
{
  struct tm *nT; time_t lTime;

  time(&lTime); nT=localtime(&lTime);
  sprintf(szDT,"%s/%02d/%4d  %02d:%02d:%02d",
	  qPrintable(QLocale("C").monthName(nT->tm_mon+1,QLocale::ShortFormat)),
	  nT->tm_mday,nT->tm_year+1900,nT->tm_hour,nT->tm_min,nT->tm_sec);
}

/**************************************************************************/
int getDayOfYear(int year,int month,int day)
/**************************************************************************/
/*!
  \returns Gregorian day of the year for a Gregorian calendar date.
*/
{
  return gregorianDayOfYear(year,month,day)
    -gregorianDayOfYear(year,1,1)+1;
}

/**************************************************************************/
int getDayOfYear(double decYear)
/**************************************************************************/
/*!
  \brief Calculates the Gregorian day of the year for a decimal time
  [years] \a decYear.

  \returns The calculated Gregorian day of the year, or
  \c ODV::missINT32 if \a decYear is \c ODV::missDOUBLE on entry.
*/
{
  int year,month,day,hour,minute,doy=ODV::missINT32; double sec;
  if (decYear!=ODV::missDOUBLE)
    {
      dateFromDecimalYear(decYear,year,month,day,hour,minute,sec);
      doy=getDayOfYear(year,month,day);
    }
  return doy;
}

/**************************************************************************/
void gregorianDate(int gregDay,int& year,int& month,int& day)
/**************************************************************************/
/*!

  \brief Given the Gregorian day \a gregDay calculates the associated
  date.

*/
{
  year=(int) (floor((double) gregDay)/366),month;

  /* search forward year by year from approximate year */
  while (gregDay>=gregorianDay(year+1,1,1)) ++year;
  /* search forward month by month from January */
  month=1;
  while (gregDay>gregorianDay(year,month,gregorianDaysInMonth(year,month)))
    ++month;
  day=gregDay-gregorianDay(year,month,1)+1;
}

/**************************************************************************/
void gregorianDateInYear(int year,int dayOfYear,int& month,int& day)
/**************************************************************************/
/*!
  \brief Given the year \a year and the day in this year \a dayOfYear
  calculates and returns the month \a month and day \a day.
*/
{
  int days=gregorianDay(year-1,12,31)+dayOfYear;
  gregorianDate(days,year,month,day);
}

/**************************************************************************/
int gregorianDay(int year,int month,int day)
/**************************************************************************/
/*!
  \returns The Gregorian days for a date on the Gregorian calender.
*/
{
  int ym1=year-1;
  return (gregorianDayOfYear(year,month,day)+365*ym1+ym1/4-ym1/100+ym1/400);
}

/**************************************************************************/
int gregorianDayOfWeek(int year,int month,int day)
/**************************************************************************/
/*!
  \brief Returns the day of the week (Gregorian calender) given a date
  on the Gregorian calender.

  \return 0=Monday, ... 5=Saturday, 6=Sunday

   Reference: C. Zeller, Kalender-Formeln, Acta Mathematica, 9 (1887)
   131-136
*/
{
   if (month<3) { month+=12; year--; }
   return ((13*month+3)/5+day+year+year/4-year/100+year/400) % 7;
}

/**************************************************************************/
int gregorianDayOfYear(int year,int month,int day)
/**************************************************************************/
/*!
  \returns The day of the year (Gregorian calender) given a date
  on the Gregorian calender.
*/
{
  int N=day,m;
  for (m=month-1; m>0; --m) N+=gregorianDaysInMonth(year,m);
  return N;
}

/**************************************************************************/
int gregorianDaysInMonth(int year,int month)
/**************************************************************************/
/*!
  \returns The last day of \a month for the Gregorian calendar.
*/

{
  switch (month)
    {
    case 2:
      if (isGregorianLeapYear(year)) return 29;
      else return 28;
    case 4:
    case 6:
    case 9:
    case 11: return 30;
    default: return 31;
    }
}

/**************************************************************************/
int gregorianDaysInYear(int year)
/**************************************************************************/
/*!
  \returns The number of days in \a year.
*/
{
  if (isGregorianLeapYear(year)) return 366;
  else return 365;
}

/**************************************************************************/
bool isGregorianLeapYear(int year)
/**************************************************************************/
/*!
  \returns \c true if \a year is a leap year according to the
  Gregorian calendar, or \c false otherwise.
*/
{
  return ((((year%4)==0) && ((year%100)!=0)) || ((year%400)==0));
}

/**************************************************************************/
QString isoDateFromGregorianDay(double gregDay)
/**************************************************************************/
/*!
  \returns The ISO date/time string for fractional Gregorian day \a gregDay.
*/
{
  int year,month,day,hour,minute,gd=(int) gregDay; double sec;

  gregorianDate(gd,year,month,day);
  daytimeFromFractionalDay(gregDay-gd,hour,minute,sec);

  return dateTimeIsoString(year,month,day,hour,minute,sec);
}

/**************************************************************************/
int julianDay(int year,int month,int day)
/**************************************************************************/
/*!
  \returns Julian day for a given date on the Gregorian calender.
*/
{
  // int jul,ja,jy,jm;

  // if (year==0) return -1;
  // if (year<0) ++year;
  // if (month > 2) { jy=year; jm=month+1; }
  // else        { jy=year-1; jm=month+13; }
  // jul=(int) (floor(365.25*jy)+floor(30.6001*jm)+day+1720995);
  // if (day+31*(month+12*year)>=(15+31*(10+12*1582)))
  //   { ja=(int) (0.01*jy); jul+=2-ja+(int) (0.25*ja); }
  // return jul;

  int jd=(1461*(year+4800+(month-14)/12))/4
    +(367*(month-2-12*((month-14)/12)))/12
    -(3*((year+4900+(month-14)/12)/100))/4
    +day-32075;
  return jd;
}

/**************************************************************************/
QString timeString(ODV::TimeFormat timeFormat,double dHH,double dMM,double dSS)
/**************************************************************************/
/*!
  \returns The time as string in form \a timeFormat.

  If hour \a dHH is invalid an empty string is returned, if minute \a
  dMM is invalid only the hours are returned in string, if second \a
  dSS is invalid it is omitted.

  Only the integer part of \a dSS is used.

  \sa dateTimeIsoString(), dateString()

*/
{
  /* check validity of hour, minute and second values */
  if (dHH<0. || dHH>=24.) dHH=ODV::missDOUBLE;
  if (dMM<0. || dMM>=60.) dMM=ODV::missDOUBLE;
  if (dSS<0. || dSS>=60.) dSS=ODV::missDOUBLE;

  /* obtain string representation of hour, minute, and sec values */
  QString H=(dHH!=ODV::missDOUBLE) ?
    QString("%1").arg((int) dHH,2,10,QChar('0')) : "  ";
  QString M=(dMM!=ODV::missDOUBLE) ?
    QString("%1").arg((int) dMM,2,10,QChar('0')) : "  ";
  QString S=(dSS!=ODV::missDOUBLE) ?
    QString("%1").arg((int) dSS,2,10,QChar('0')) : "  ";

  QString s;
  switch(timeFormat)
    {
      case ODV::hhmmssTime:
	return QString("%1%2%3").arg(H).arg(M).arg(S);
      case ODV::hhmmTime:
	return QString("%1%2").arg(H).arg(M);
      case ODV::IsoTime:
      default:
	if (dHH!=ODV::missDOUBLE)
	  {
	    s=H;
	    if (dMM!=ODV::missDOUBLE)
	      {
		s+=QString(":%1").arg(M);
		if (dSS!=ODV::missDOUBLE) s+=QString(":%1").arg(S);
	      }
	  }
	return s.simplified();
    }
}

/**************************************************************************/
template <class T>
bool validateDateT(T& year,T& month,T& day,T& hour,T& minute,double& sec)
/**************************************************************************/
{
  bool modifiedD=false; int dayShift=0;

  /* check daytime and adjust day if necessary */
  bool modifiedT=validateTime(hour,minute,sec,dayShift);
  day+=(T) dayShift;

  /* check day */
  T lastDayInMonth=(T) gregorianDaysInMonth((int) year,(int) month);
  if (day>lastDayInMonth) { day-=lastDayInMonth; ++month; modifiedD=true; }
  /* check month */
  if (month>12) { month-=12; ++year; modifiedD=true; }

  return modifiedT||modifiedD;
}

/**************************************************************************/
template <class T>
bool validateTimeT(T& hour,T& minute,double& sec,int& dayShift)
/**************************************************************************/
{
  /* initialize the day shift value and modified flag */
  bool modified=false; dayShift=0;

  /* check seconds */
  if (fabs(sec)<0.1) sec=0.;
  if (sec>59.9) { sec=0.; ++minute; modified=true; }
  /* check minutes */
  if (minute>=60) { minute-=60; ++hour; modified=true; }
  /* check hours */
  if (hour>=24) { hour-=24; dayShift=1; modified=true; }

  return modified;
}

/**************************************************************************/
bool validateDate(short& year,short& month,short& day,
		  short& hour,short& minute,double& sec)
/**************************************************************************/
/*!

  \brief Ensures that the specified date values are valid, and make
  modifications if necessary.

  \return \c true if modifications were made and \c false otherwise.
*/
{ return validateDateT(year,month,day,hour,minute,sec); }

/**************************************************************************/
bool validateDate(int& year,int& month,int& day,
		  int& hour,int& minute,double& sec)
/**************************************************************************/
/*!
  Overloaded method.
  \sa validateDate(short&,short&,short&,short&,short&,double&)
*/
{ return validateDateT(year,month,day,hour,minute,sec); }

/**************************************************************************/
bool validateDate(double& year,double& month,double& day,
		  double& hour,double& minute, double& sec)
/**************************************************************************/
/*!
  Overloaded method.
*/
{ return validateDateT(year,month,day,hour,minute,sec); }

/**************************************************************************/
bool validateTime(short& hour,short& minute,double& sec,int& dayShift)
/**************************************************************************/
/*!
  Ensures that the specified time values are valid, and make
  modifications if necessary. If a day shift arises from the modifications
  it is returned in \a dayShift.

  \return \c true if modifications were made and \c false otherwise.
*/
{ return validateTimeT(hour,minute,sec,dayShift); }

/**************************************************************************/
bool validateTime(int& hour,int& minute,double& sec,int& dayShift)
/**************************************************************************/
/*!
  Overloaded method.
  \sa validateTime(short&,short&,double&,short&)
*/
{ return validateTimeT(hour,minute,sec,dayShift); }

/**************************************************************************/
bool validateTime(double& hour,double& minute,double& sec,int& dayShift)
/**************************************************************************/
/*!
  Overloaded method.
  \sa validateTime(short&,short&,double&,short&)
*/
{ return validateTimeT(hour,minute,sec,dayShift); }
