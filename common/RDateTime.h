#ifndef RDATETIME_H
#define RDATETIME_H

/****************************************************************************
 **
 ** Copyright (C) 1993-2025 Reiner Schlitzer. All rights reserved.
 **
 ** This file is part of Ocean Data View.
 **
 ** Structure & global functions for: RDateTime
 **
 ** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 ** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 **
 ****************************************************************************/

#include <QString>

#include "common/varConversion.h"

/*! \file

  This file declares enumerations, structures and global functions
  dealing with date and time values that have a reference time.

*/

/*! Time types */
enum TimeType
  {
    NotATime=-1, ISO8601Time=0, CJDTime=1, DecimalTimeYearsSince=2,
    DecimalDOYTime=3, DecimalDOMTime=4, MonDayYearTime=5,
    DecimalTimeDaysSince=6, DecimalTimeHoursSince=7,
    DecimalTimeMinutesSince=8, DecimalTimeSecondsSince=9,
    DecimalTimeMilliSecondsSince=10
  };


/**************************************************************************/
/*!
  \struct RDateTime
  \brief Structure to hold reference date/time information
*/
struct RDateTime
/**************************************************************************/
{
  RDateTime(int year=0,int month=1,int day=1,
            int hour=0,int minute=0,double second=0.);
  RDateTime(const char *szSinceSpec);
  RDateTime(const QString& varName,const QString& sinceSpec);

  bool  isValid();
  void	setDateTime(int year=0,int month=1,int day=1,
                    int hour=0,int minute=0,double second=0.);
  bool	setDateTime(const char *szSinceSpec);
  bool	setDateTime(const QString& varName,const QString& sinceSpec);
  bool	setDateTime(const char *szDateTime,ConversionType cnvType);
  void	setCalendarAndAbsDays(int calID=-1);

  int yr;         //!< Reference year
  int mon;        //!< Reference month
  int dy;         //!< Reference day
  int hh;         //!< Reference hour
  int mm;         //!< Reference minute
  double sec;     //!< Reference second
  int calendarID; //!< Calendar ID. 0: Gregorian 1: Julian
  double absDays; //!< Number of days in calendar \a calendarID for given date/time
};

ConversionType conversionTypeFromTimeType(TimeType tt);
int    convertDate(double dVal,
                   ConversionType cnvType,RDateTime *refTime,
                   int& year,int& month,int& day,
                   int& hour,int& minute,double& sec);
int    convertDate(const char *szDateTime,
                   ConversionType cnvType,RDateTime *refTime,
                   int& year,int& month,int& day,
                   int& hour,int& minute,double& sec);
int    convertTime(double dVal,
                   ConversionType cnvType,RDateTime *refTime,
                   int& sHH,int& sMM,double& dSS);
int    convertTime(const char *szTime,
                   ConversionType cnvType,RDateTime *refTime,
                   int& sHH,int& sMM,double& dSS);
double gregorianDayFromIsoDateTime(const QString& isoDateTime);
QString	timeTypeName(TimeType tt,RDateTime *refTime=NULL);

#endif // RDATETIME_H
