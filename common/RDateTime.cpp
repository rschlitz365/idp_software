/****************************************************************************
 **
 ** Copyright (C) 1993-2025 Reiner Schlitzer. All rights reserved.
 **
 ** This file is part of Ocean Data View.
 **
 ** Global functions: convertXXX.cpp
 **           Struct: RDateTime
 **
 ** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 ** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 **
 ****************************************************************************/
#include "common/compatibility.h"
#include "common/RDateTime.h"

#include <string.h>		// str...
#include <stdio.h>		// sscanf
#include <math.h>

#include <QString>

#include "common/odvDate.h"
#include "common/stringTools.h"
#include "common/varConversion.h"
#include "common/mathhelper.h"

/***************************************************************************
 ** NEW CLASS
 ***************************************************************************/

/**************************************************************************/
RDateTime::RDateTime(int year,int month,int day,
                     int hour,int minute,double second)
/**************************************************************************/
/*!
  \brief Default RDateTime constructor.
*/
{
  setDateTime(year,month,day,hour,minute,second);
}

/**************************************************************************/
RDateTime::RDateTime(const char *szSinceSpec)
/**************************************************************************/
/*!

  \brief RDateTime constructor using information from a "since"
  specification \a szSinceSpec.

  \a szSinceSpec must contain the keyword \c since followed by an ISO 8601
  date/time specification.

*/
{
  setDateTime(szSinceSpec);
}

/**************************************************************************/
RDateTime::RDateTime(const QString& varName,const QString& sinceSpec)
/**************************************************************************/
/*!

  \brief RDateTime constructor using information from a time
  variable name or a "since" specification \a sinceSpec.

*/
{
  setDateTime(varName,sinceSpec);
}

/**************************************************************************/
bool RDateTime::isValid()
/**************************************************************************/
/*!

  \brief Checks whether the date of this object is valid.

  \return \c true, if the date of this object is valid, or \c false
  otherwise.

*/
{
  if (yr==ODV::missINT32 || mon==ODV::missINT32 || dy==ODV::missINT32)
    return false;
  else
    return true;
}

/**************************************************************************/
void RDateTime::setDateTime(int year,int month,int day,
                            int hour,int minute,double second)
/**************************************************************************/
/*!

  \brief Sets the date/time of this object using the passed parameter
  values.

  Sets the daytime to midnight if \a hour or \a minute are equal to \c
  ODV::missINT32 on entry.

*/
{
  yr=year; mon=month; dy=day; sec=second;
  hh=(hour!=ODV::missINT32) ? hour : 0;
  mm=(minute!=ODV::missINT32) ? minute : 0;
  int dayShift; validateTime(hh,mm,sec,dayShift);

  setCalendarAndAbsDays();
}

/**************************************************************************/
bool RDateTime::setDateTime(const char *szSinceSpec)
/**************************************************************************/
/*!

  \brief Sets the date/time of this object to values extracted from
  the "since" specification \a szSinceSpec.

  \a szSinceSpec must contain the keyword \c since followed by an ISO 8601
  date/time specification.

  \return \c true, if the date/time of this object was set
  successfully, and \c false otherwise.

*/
{
  /* set default values */
  setDateTime();

  /* make local copy of szSinceSpec and convert to lower case */
  char szB[256],*psz; strcpy(szB,szSinceSpec); mystrlwr(szB);

  /* immediate return if szSinceSpec does not contain the keyword "since" */
  if (!(psz=strstr(szB,"since"))) return false;

  /* extract the date/time information and set this object */
  int year,month,day,hh,mm; double sec; bool b=false;
  if (convertDate(psz+5,CNV_ISODATETIME,0,year,month,day,hh,mm,sec))
    {
      setDateTime(year,month,day,hh,mm,sec);
      setCalendarAndAbsDays(); b=true;
    }

  return b;
}

/**************************************************************************/
bool RDateTime::setDateTime(const QString& varName,const QString& sinceSpec)
/**************************************************************************/
/*!

  \brief Sets the date/time of this object to values extracted from a
  time variable name \a varName or the "since" specification \a
  sinceSpec.

  Sets the RDateTime to the beginning of the Julian
  calendar if the special string "Chronological Julian Date" is
  detected in \a varName.

  \a sinceSpec must contain the keyword \c since followed by an ISO
  8601 date/time specification.

  \return \c true, if the date/time of this object was set
  successfully, and \c false otherwise.
*/
{
  if (varName.startsWith("Chronological Julian Date",Qt::CaseInsensitive))
    {
      yr=-4713; mon=1; dy=1; hh=12; mm=0; sec=0.; setCalendarAndAbsDays(1);
      return true;
    }
  else
    return setDateTime(qPrintable(sinceSpec));
}

/**************************************************************************/
bool RDateTime::setDateTime(const char *szDateTime,ConversionType cnvType)
/**************************************************************************/
/*!

  \brief Sets the date/time of this object to values extracted from \a
  szDateTime using conversion \a cnvType.

  \note \a cnvType must not be a relative date/time conversion.

  \return \c true, if the date/time of this object was set
  successfully, and \c false otherwise.

*/
{
  /* set default values */
  setDateTime();

  /* extract the date/time information and set this object */
  int year,month,day,hh,mm; double sec; bool b=false;
  if (convertDate(szDateTime,cnvType,0,year,month,day,hh,mm,sec))
    {
      setDateTime(year,month,day,hh,mm,sec);
      setCalendarAndAbsDays(); b=true;
    }

  return b;
}

/**************************************************************************/
void RDateTime::setCalendarAndAbsDays(int calID)
/**************************************************************************/
/*!

  \brief Sets the calendar ID to \a calID and calculates the Gregorian
  or Julian days (depending on \a calID) of the current date.

*/
{
  /* if calID==-1 (the default) we assume Julian calendar if reference
     year is -4713 and Gregorian otherwise */
  calendarID=(calID==-1) ? ((yr==-4713) ? 1:0) : calID;

  if (calendarID==0)
    {
      absDays=(double) gregorianDay(yr,mon,dy);
      absDays+=COPYSIGN((hh+mm/60.)/24.,yr);
    }
  else
    absDays=0.5;
}

/**************************************************************************/
ConversionType conversionTypeFromTimeType(TimeType tt)
/**************************************************************************/
/*!

  \brief Determines the conversion type for TimeType \a tt.

  \return The conversion type for TimeType \a tt.
*/
{
  switch(tt)
    {
    case ISO8601Time:             return CNV_ISODATETIME;
    case CJDTime:                 return CNV_CJDDATETIME;
    case DecimalTimeYearsSince:   return CNV_RTIME_YS;
    case DecimalTimeDaysSince:    return CNV_RTIME_DS;
    case DecimalTimeHoursSince:   return CNV_RTIME_HS;
    case DecimalTimeMinutesSince: return CNV_RTIME_MS;
    case DecimalTimeSecondsSince: return CNV_RTIME_SS;
    case DecimalTimeMilliSecondsSince: return CNV_RTIME_MSS;
    case MonDayYearTime:          return CNV_DATE_MMDDYYYY;
    case DecimalDOYTime:
    case DecimalDOMTime:
    default:                      return CNV_IDENTITY;
    }
}

/**************************************************************************/
int  convertDate(double dVal,
                 ConversionType cnvType,RDateTime *refTime,
                 int& year,int& month,int& day,
                 int& hour,int& minute,double& sec)
/**************************************************************************/
/*!

  \brief Converts date \a dVal to \a year, \a month, \a day, \a hour,
  \a minute and \a sec values according to conversion type \a cnvType.

  The reference time object \a refTime is only used if \a dVal
  contains date/time relative to \a refTime and \a cnvType is one of
  the relative date/time conversions \c CNV_RTIME_MSS, \c
  CNV_RTIME_SS, \c CNV_RTIME_MS, \c CNV_RTIME_HS, \c CNV_RTIME_DS, and
  \c CNV_RTIME_YS. A \c NULL pointer may be passed in all other cases.

  \note The daytime parameters \a hour, \a minute, and \a sec are set to
  zero for pure date conversion types, such as \c CNV_DATE_YYYYMMDD.

  \return 1 if successful and 0 otherwise.
*/
{
  int i,iVal; double d; int calendarID=refTime ? refTime->calendarID : 0;

  /* use 00:00:00.0 as default daytime */
  hour=minute=0; sec=0.;

  /* if relative time: convert to days. add reference date */
  switch(cnvType)
    {
    case CNV_RTIME_MSS:
      dVal/=1000.;
    case CNV_RTIME_SS:
      dVal/=60.;
    case CNV_RTIME_MS:
      dVal/=60.;
    case CNV_RTIME_HS:
      dVal/=24.;
    case CNV_RTIME_DS:
      if (refTime) dVal+=refTime->absDays;
      break;
    case CNV_RTIME_YS:
      if (refTime)
        {
          d=dVal+decimalYear(refTime->yr,refTime->mon,refTime->dy,
                             refTime->hh,refTime->mm,refTime->sec);
          dateFromDecimalYear(d,year,month,day,hour,minute,sec);
          return 1;
        }
      break;
    default:
      break;
    }

  /* do conversion */
  iVal=(int) floor(dVal);
  switch(cnvType)
    {
    case CNV_CJDDATETIME:
      dateFromJulianDay(dVal,year,month,day,hour,minute,sec,true);
      break;
    case CNV_DATE_YYYYMMDD:
      year=(int) (dVal*0.0001); month=(int) ((dVal*0.01)-year*100);
      day=(int) ((dVal)-year*10000-month*100);
      break;
    case CNV_DATE_MMDDYYYY:
      d=dVal*0.0001; i=(int) d; year=(int) myround((d-i)*10000.,0);
      d=i*0.01; month=(int) d; day=(int) myround((d-month)*100.,0);
      break;
    case CNV_DATE_DDMMYYYY:
      d=dVal*0.0001; i=(int) d; year=(int) myround((d-i)*10000.,0);
      d=i*0.01; day=(int) d; month=(int) myround((d-day)*100.,0);
      break;
    case CNV_DATE_YMDF:
      year=(int) (dVal*0.0001); month=(int) (dVal*0.01)-year*100;
      day=(int) (dVal)-year*10000-month*100+1;
      dVal-=iVal; hour=(int) (dVal*24);
      minute=(int) ((dVal-hour/24.)*24.*60.);
      sec=(dVal-(hour+minute/60.)/24.)*24.*60.*60.;
      break;
    case CNV_DATE_MDF:
      month=(int) (dVal*0.01); day=(int) (dVal)-month*100+1;
      dVal-=iVal;  hour=(int) (dVal*24);
      minute=(short) ((dVal-hour/24.)*24.*60.);
      sec=(dVal-(hour+minute/60.)/24.)*24.*60.*60.;
      break;
    case CNV_DATE_MD:
      month=(int) (dVal*0.01); day=(int) (dVal)-month*100;
      break;
    case CNV_DATE_DF:
      day=(int) iVal+1; dVal-=iVal;  hour=(int) (dVal*24);
      minute=(int) ((dVal-hour/24.)*24.*60.);
      sec=(dVal-(hour+minute/60.)/24.)*24.*60.*60.;
      break;
    case CNV_DATE_YMF:
      year=(int) (dVal*0.01); month=(int) dVal-year*100+1;
      dVal-=iVal; i=(int) (dVal*30);
      if (month==2) i=qMin(i,28);
      i=qMax(i,1); day=i;
      break;
    case CNV_DATE_MF:
      month=iVal+1; dVal-=iVal; i=(int) (dVal*30);
      if (month==2) i=qMin(i,28);
      i=qMax(i,1); day=i;
      break;
    case CNV_DATE_YF:
      //year=iVal; dVal-=iVal; month=qMax((int) (dVal*12),1); day=15;
      dateFromDecimalYear(dVal,year,month,day,hour,minute,sec);
      break;
    case CNV_DATE_Y:
      year=iVal; month=6; day=15;
      break;
    case CNV_DATE_FY:
      year=0; month=qMax((int) (dVal*12),1); day=15;
      break;
    case CNV_RTIME_YS:
      dateFromDecimalYear(dVal,year,month,day,hour,minute,sec);
      break;
    case CNV_RTIME_DS:
    case CNV_RTIME_HS:
    case CNV_RTIME_MS:
    case CNV_RTIME_SS:
    case CNV_RTIME_MSS:
      if (calendarID==0)
        {
          gregorianDate(iVal,year,month,day);
          dVal-=iVal; dVal*=24.;
          hour=(int) dVal; minute=(int) ((dVal-hour)*60);
          sec=(dVal-(hour+minute/60.))*60.*60.;
        }
      else
        {
          dateFromJulianDay(dVal,year,month,day,hour,minute,sec,true);
        }
      break;
    default:
      break;
    }

  /* validate the date/time */
  validateDate(year,month,day,hour,minute,sec);

  return 1;
}

/**************************************************************************/
int convertDate(const char *szDateTime,
                ConversionType cnvType,RDateTime *refTime,
                int& year,int& month,int& day,
                int& hour,int& minute,double& sec)
/**************************************************************************/
/*!

  \brief Converts date in \a szDateTime to \a year, \a month, and \a
  day values according to conversion type \a cnvType.

  ISO 8601 dates also provide \a hour, \a minute and \a sec
  values. Supports date formats with separators between items.

  ISO 8601 dates: accepts any of the following "yyyy", "yyyy-mm",
  "yyyy-mm-dd", "yyyy-mm-dd hh", "yyyy-mm-dd hh:mm". separation
  character between date and time may be either " " or "T". Any
  missing or unreadable item is left at its respective entry value.

  The reference time object \a refTime is only used if \a szDateTime
  contains date/time relative to \a refTime and \a cnvType is one of
  the relative date/time conversions \c CNV_RTIME_MSS, \c
  CNV_RTIME_SS, \c CNV_RTIME_MS, \c CNV_RTIME_HS, \c CNV_RTIME_DS, and
  \c CNV_RTIME_YS. A NULL pointer may be passed in all other cases.

  \note Time zone designators, such as Z, +hh:mm or -hh:mm are ignored.

  \return 1 if successful and 0 otherwise.
*/
{
  int i,r=0; double d; char szB[256],*psz,*psB=0,*psT=0;

  /* make local copy of szDateTime and convert to lower case */
  strcpy(szB,szDateTime); mystrlwr(szB); psz=szB;

  /* initialize date and time parameters */
  year=month=day=hour=minute=ODV::missINT32; sec=ODV::missDOUBLE;

  switch(cnvType)
    {
    case CNV_ISODATE:
    case CNV_ISODATETIME:
      /* yyyy */
      if (sscanf(psz,"%d",&i)==1) { year=i; r=1; }
      /* mm */
      if (!(psz=strstr(psz,"-")) || !r) return r;
      ++psz; if (sscanf(psz,"%d",&i)==1) month=i;
      /* dd */
      if (!(psz=strstr(psz,"-"))) return r;
      ++psz; if (sscanf(psz,"%d",&i)==1) day=i;
      /* break in case of CNV_ISODATE */
      if (cnvType==CNV_ISODATE) break;
      /* do we have time included? */
      psB=strstr(psz," "); psT=strstr(psz,"t");
      if (strlen(psz)<3 && !psB && !psT) return r;
      if (psB) psz=psB; else if (psT) psz=psT;
      /* hh */
      ++psz; if (sscanf(psz,"%d",&i)==1) hour=i;
      /* mm */
      if (!(psz=strstr(psz,":"))) return 1;
      ++psz; if (sscanf(psz,"%d",&i)==1) minute=i;
      /* seconds */
      if (!(psz=strstr(psz,":"))) return 1;
      ++psz; if (sscanf(psz,"%lg",&d)==1) sec=d;
      break;
    case CNV_DATE_MMDDYYYY:
      /* date is mmddyyyy */
      hour=minute=0; sec=0.;
      if (sscanf(szB,"%2d%2d%4d",&month,&day,&year)<3) return 0;
      r=1;
      break;
    case CNV_DATE_MMDDYYYY1:
      /* date is mm/dd/yyyy */
      hour=minute=0; sec=0.;
      if (sscanf(szB,"%d/%d/%d",&month,&day,&year)<3) return 0;
      r=1;
      break;
    case CNV_DATE_DDMMYYYY:
      /* date is ddmmyyyy */
      hour=minute=0; sec=0.;
      if (sscanf(szB,"%2d%2d%4d",&day,&month,&year)<3) return 0;
      r=1;
      break;
    case CNV_DATE_DDMMYYYY1:
      /* date is dd/mm/yyyy */
      hour=minute=0; sec=0.;
      if (sscanf(szB,"%d/%d/%d",&day,&month,&year)<3) return 0;
      r=1;
      break;
    case CNV_DATE_YYYYMMDD:
      /* date is yyyymmdd */
      hour=minute=0; sec=0.;
      if (sscanf(szB,"%4d%2d%2d",&year,&month,&day)<3) return 0;
      r=1;
      break;
    case CNV_DATE_YYYYMMDD1:
      /* date is yyyy/mm/dd */
      hour=minute=0; sec=0.;
      if (sscanf(szB,"%d/%d/%d",&year,&month,&day)<3) return 0;
      r=1;
      break;
    case CNV_CJDDATETIME:
    case CNV_DATE_YMDF:
    case CNV_DATE_MDF:
    case CNV_DATE_MD:
    case CNV_DATE_DF:
    case CNV_DATE_YMF:
    case CNV_DATE_MF:
    case CNV_DATE_YF:
    case CNV_DATE_Y:
    case CNV_DATE_FY:
      /* date is a pure numeric format */
      if (sscanf(szDateTime,"%lg",&d))
        { convertDate(d,cnvType,0,year,month,day,hour,minute,sec); r=1; }
      break;
    case CNV_RTIME_MSS:
    case CNV_RTIME_SS:
    case CNV_RTIME_MS:
    case CNV_RTIME_HS:
    case CNV_RTIME_DS:
    case CNV_RTIME_YS:
      /* date is a relative date */
      if (refTime && sscanf(szDateTime,"%lg",&d))
        { convertDate(d,cnvType,refTime,year,month,day,hour,minute,sec); r=1; }
      break;
    default:
      break;
    }

  return r;
}

/**************************************************************************/
int convertTime(double dVal,
                ConversionType cnvType,RDateTime *refTime,
                int& hour,int& minute,double& sec)
/**************************************************************************/
/*!

  \brief Converts day-time \a dVal to hh:mm:s.ss.

  The reference time object \a refTime is only used if \a szDateTime
  contains date/time relative to \a refTime and \a cnvType is one of
  the relative date/time conversions \c CNV_RTIME_MSS, \c
  CNV_RTIME_SS, \c CNV_RTIME_MS, \c CNV_RTIME_HS, \c CNV_RTIME_DS, and
  \c CNV_RTIME_YS. A NULL pointer may be passed in all other cases.

  \return 1 if successful and 0 otherwise.
*/
{
  int dayShift=0; bool modified; hour=minute=0; sec=0.;

  /* if relative time: convert to days. add reference date and get
     daytime from fractional day */
  switch(cnvType)
    {
    case CNV_RTIME_MSS: // Milliseconds since YYYY-MM-DDThh:mm:ss.sss
      dVal/=1000.;
    case CNV_RTIME_SS: // Seconds since YYYY-MM-DDThh:mm:ss.sss
      dVal/=60.;
    case CNV_RTIME_MS: // Minutes since YYYY-MM-DDThh:mm:ss.sss
      dVal/=60.;
    case CNV_RTIME_HS: // Hours since YYYY-MM-DDThh:mm:ss.sss
      dVal/=24.;
    case CNV_RTIME_DS: // Days since YYYY-MM-DDThh:mm:ss.sss
      if (refTime) dVal+=refTime->absDays;
      daytimeFromFractionalDay(dVal-floor(dVal),hour,minute,sec);
      modified=validateTime(hour,minute,sec,dayShift);
      return 1;
      break;
    case CNV_RTIME_YS: // Years since YYYY-MM-DDThh:mm:ss.sss
      if (refTime)
        {
          int year,month,day; double d;
          d=dVal+decimalYear(refTime->yr,refTime->mon,refTime->dy,
                             refTime->hh,refTime->mm,refTime->sec);
          dateFromDecimalYear(d,year,month,day,hour,minute,sec);
          return 1;
        }
      break;
    default:
      break;
    }

  /* do other conversions */
  switch(cnvType)
    {
    case CNV_TIME_HMS:  // hhmmss
      dVal*=0.01;
    case CNV_TIME_HHMM: // hhmm
    case CNV_TIME_HM:   // hhmm
      hour=(int) (dVal*0.01); minute=(int) (dVal-100.*hour);
      if (cnvType==CNV_TIME_HMS) sec=100.*(dVal-floor(dVal));
      break;
    case CNV_TIME_DAYF: // fractional day
      if (dVal<0. || dVal>1.)
        { hour=minute=ODV::missINT32; sec=ODV::missDOUBLE; return 0; }
      dVal*=24.; // convert to hh.fff; no break
    case CNV_TIME_HF:   // hh.fff
      hour=(int) dVal; minute=(int) ((dVal-hour)*60.);
      sec=(dVal-(hour+minute/60.))*60.*60.;
      break;
    case CNV_TIME_MF:   // mm.fff
      hour=(int) (dVal/60.); minute=(int) (dVal-hour*60.);
      sec=(dVal-(hour*60+minute))*60;
      break;
    case CNV_TIME_SF:   // sec.fff
      hour=(int) (dVal/3600.);
      minute=(int) ((dVal-hour*3600.)/60.);
      sec=(dVal-(hour*60+minute)*60.);
      break;
    default:
      break;
    }

  modified=validateTime(hour,minute,sec,dayShift);

  return 1;
}

/**************************************************************************/
int convertTime(const char *szTime,
                ConversionType cnvType,RDateTime *refTime,
                int& hour,int& minute,double& sec)
/**************************************************************************/
/*!

  \brief Converts daytime \a szTime to hour, minutes and seconds.

  \a szTime may contain time in many formats.

  The reference time object \a refTime is only used if \a szDateTime
  contains date/time relative to \a refTime and \a cnvType is one of
  the relative date/time conversions \c CNV_RTIME_MSS, \c
  CNV_RTIME_SS, \c CNV_RTIME_MS, \c CNV_RTIME_HS, \c CNV_RTIME_DS, and
  \c CNV_RTIME_YS. A NULL pointer may be passed in all other cases.

  \return 1 if successful and 0 otherwise.
*/
{
  int r=0,n,year,month,day,hh,mm; double d,ss; char szB[256];

  /* make local copy of szTime */
  strcpy(szB,szTime);

  switch(cnvType)
    {
    case CNV_TIME_HHMM1: // hh:mm:ss.sss
      n=sscanf(szB,"%d:%d:%lg",&hh,&mm,&ss);
      if (n>=1) hour=hh;
      if (n>=2) minute=mm;
      if (n==3) sec=ss;
      r=1;
      break;
    case CNV_TIME_HHMM: // hhmm
    case CNV_TIME_HM:   // hhmm
    case CNV_TIME_HMS:  // hhmmss
    case CNV_TIME_DAYF: // fractional day
    case CNV_TIME_HF:   // hh.fff
    case CNV_TIME_MF:   // mm.fff
    case CNV_TIME_SF:   // sec.fff
      if (sscanf(szTime,"%lg",&d))
        r=convertTime(d,cnvType,0,hour,minute,sec);
      break;
    case CNV_RTIME_MSS: // Milliseconds since YYYY-MM-DDThh:mm:ss.sss
    case CNV_RTIME_SS: // Seconds since YYYY-MM-DDThh:mm:ss.sss
    case CNV_RTIME_MS: // Minutes since YYYY-MM-DDThh:mm:ss.sss
    case CNV_RTIME_HS: // Hours since YYYY-MM-DDThh:mm:ss.sss
    case CNV_RTIME_DS: // Days since YYYY-MM-DDThh:mm:ss.sss
    case CNV_RTIME_YS: // Years since YYYY-MM-DDThh:mm:ss.sss
      /* date is a relative date */
      if (refTime && sscanf(szTime,"%lg",&d))
        { convertDate(d,cnvType,refTime,year,month,day,hour,minute,sec); r=1; }
      break;
    default:
      break;
    }

  return r;
}

/**************************************************************************/
double gregorianDayFromIsoDateTime(const QString& isoDateTime)
/**************************************************************************/
/*!

  \brief Returns the fractional Gregorian day for ISO date/time \a
  isoDateTime, or \c ODV::missDOUBLE if \a isoDateTime cannot be
  interpreted.

*/
{
  double sec; int year,month,day,hour,minute;
  return convertDate(qPrintable(isoDateTime),CNV_ISODATETIME,0,
                     year,month,day,hour,minute,sec) ?
    (double) gregorianDay(year,month,day)+decimalDay(hour,minute,sec) :
    ODV::missDOUBLE;
}

/**************************************************************************/
QString timeTypeName(TimeType tt,RDateTime *refTime)
/**************************************************************************/
/*!

  \return The human readable name for time type type \a tt.

  If \a refTime is non-zero a "since yyyy-mm-dd" suffix is added to the
  units of the decimal time types.
*/
{
  switch (tt)
    {
    case NotATime:       return QString("Not a Time");
    case ISO8601Time:    return QString("time_ISO8601");
    case CJDTime:        return QString("Chronological Julian Date [days]");
    case DecimalDOYTime: return QString("Day of Year [days]");
    case DecimalDOMTime: return QString("Day of Month [days]");
    case MonDayYearTime: return QString("mon/day/yr");
    case DecimalTimeYearsSince:
      return refTime ?
        QString("Time [years since %1-%2-%3]")
        .arg((int) refTime->yr,4,10,QChar('0'))
        .arg((int) refTime->mon,2,10,QChar('0'))
        .arg((int) refTime->dy,2,10,QChar('0')) : QString("Time [years]");
    case DecimalTimeDaysSince:
      return refTime ?
        QString("Time [days since %1-%2-%3]")
        .arg((int) refTime->yr,4,10,QChar('0'))
        .arg((int) refTime->mon,2,10,QChar('0'))
        .arg((int) refTime->dy,2,10,QChar('0')) : QString("Time [days]");
    case DecimalTimeHoursSince:
      return refTime ?
        QString("Time [hours since %1-%2-%3]")
        .arg((int) refTime->yr,4,10,QChar('0'))
        .arg((int) refTime->mon,2,10,QChar('0'))
        .arg((int) refTime->dy,2,10,QChar('0')) : QString("Time [hours]");
    case DecimalTimeSecondsSince:
      return refTime ?
        QString("Time [seconds since %1-%2-%3]")
        .arg((int) refTime->yr,4,10,QChar('0'))
        .arg((int) refTime->mon,2,10,QChar('0'))
        .arg((int) refTime->dy,2,10,QChar('0')) : QString("Time [seconds]");
    case DecimalTimeMilliSecondsSince:
      return refTime ?
        QString("Time [milliseconds since %1-%2-%3]")
        .arg((int) refTime->yr,4,10,QChar('0'))
        .arg((int) refTime->mon,2,10,QChar('0'))
        .arg((int) refTime->dy,2,10,QChar('0')) : QString("Time [milliseconds]");
    default: return QString();
    }
}
