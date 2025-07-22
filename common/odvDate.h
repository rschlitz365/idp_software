#ifndef ODVDATE_H
#define ODVDATE_H

/****************************************************************************
**
** Copyright (C) 1993-2025 Reiner Schlitzer. All rights reserved.
**
** This file is part of Ocean Data View.
**
** Global functions: odvDate.h
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <QDateTime>
#include <QString>

#include "common/declspec.h"
#include "common/odv.h"

/*! \file

  \brief This file declares global functions for date and time
  manipulations.

*/

DECLSPEC
void	dateFromDecimalYear(double decYear,int& year,int& month,int& day,
			  int& hour,int& minute,double& sec);
DECLSPEC
void	dateFromGregorianDay(double gregDay,int& year,int& month,int& day,
			     int& hour,int& minute,double& sec);
DECLSPEC
void	dateFromJulianDay(double julDay,int& year,int& month,int& day,
			  int& hour,int& minute,double& sec,bool isChronological=true);
DECLSPEC
QString	dateString(ODV::DateFormat dateFormat,int year,int month,int day);
DECLSPEC
QString	dateString(ODV::DateFormat dateFormat,double dYear,double dMonth,double dDay);
DECLSPEC
QDateTime dateTimeFromString(const QString &dateTimeString,
			     const QString &format="MMM dd yyyy hh:mm:ss");
DECLSPEC
QString	dateTimeIsoString(double dYear,double dMonth,double dDay,
			  double dHH,double dMM,double dSS);
DECLSPEC
void   daytimeFromFractionalDay(double fracDay,int& hour,int& minute,double& sec);
DECLSPEC
double decimalDay(int hour,int minute,double sec=0.);
DECLSPEC
double decimalYear(int year,int month,int day,int hour,int minute,double sec=0.);
DECLSPEC
double decimalYearFromGregorianDay(int gregDay);
DECLSPEC
double decimalYearFromGregorianDay(double gregDay);
DECLSPEC
void   getDateAndTime(char* szDT);
DECLSPEC
int    getDayOfYear(int year,int month,int day);
DECLSPEC
int    getDayOfYear(double decYear);
DECLSPEC
void   gregorianDate(int gregDay,int& year,int& month,int& day);
DECLSPEC
void   gregorianDateInYear(int year,int dayOfYear,int& month,int& day);
DECLSPEC
int    gregorianDay(int year,int month,int day);
DECLSPEC
int    gregorianDayOfWeek(int year,int month,int day);
DECLSPEC
int    gregorianDayOfYear(int year,int month,int day);
DECLSPEC
int    gregorianDaysInMonth(int year,int month);
DECLSPEC
int    gregorianDaysInYear(int year);
DECLSPEC
bool    isGregorianLeapYear(int year);
DECLSPEC
QString isoDateFromGregorianDay(double gregDay);
DECLSPEC
int    julianDay(int year,int month,int day);
DECLSPEC
QString timeString(ODV::TimeFormat timeFormat,double dHH,double dMM,double dSS);
DECLSPEC
bool validateDate(short& sYear,short& sMonth,short& sDay,
		  short& sHH,short& sMM, double& sec);
DECLSPEC
bool validateDate(int& year,int& month,int& day,
		  int& hour,int& minute,double& sec);
DECLSPEC
bool validateDate(double& year,double& month,double& day,
		  double& hour,double& minute, double& sec);
DECLSPEC
bool validateTime(short& sHH,short& sMM,double& sec,int& dayShift);
DECLSPEC
bool validateTime(int& hour,int& min,double& sec,int& dayShift);
DECLSPEC
bool validateTime(double& hour,double& min,double& sec,int& dayShift);


#endif // ODVDATE_H
