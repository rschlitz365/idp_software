/****************************************************************************
**
** Copyright (C) 1993-2025 Reiner Schlitzer. All rights reserved.
**
** This file is part of Ocean Data View.
**
** Global functions: varconversion.cpp
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/
#include "common/varConversion.h"

#include <string.h>

#include "common/declspec.h"
#include "common/constants.h"	// SWStdDensity
#include "tools/stringTools.h"

/**************************************************************************/
DECLSPEC
int  identifyConversion(const QString& label,const QList<CNVINFO> *cil)
/**************************************************************************/
/*!

  \brief Checks whether \a label is contained in one of the conversion
  labels from the \a cil list (case insensitive match).

  \return The zero-based index into \a cil, or -1 if no matching
  conversion can be found.

*/
{
  QString lbl=label;
  int i,nci=cil->size(); Qt::CaseSensitivity cs=Qt::CaseInsensitive;

  /* if label contains "latitude" or "longitude" but not "[deg min]"
     do failure return */
  if ((lbl.contains("latitude",cs) || lbl.contains("longitude",cs)) &&
      !lbl.contains("[deg min]",cs)) return -1;

  /* special treatment for SDN EventEndDateTime variable */
  //if (lbl.startsWith("EventEndDateTime",cs)) lbl="yyyy-mm-ddThh:mm:ss";

  /* loop over all entries in the cil list */
  for (i=0; i<nci; ++i)
    if (cil->at(i).label.contains(lbl,cs)) return i;

  return -1;
}

/**************************************************************************/
DECLSPEC
int  identifyTimeConversion(const QString& timeLabel,const QList<CNVINFO> *cil)
/**************************************************************************/
/*!

  \brief Checks whether \a timeLabel matches one of the time
  conversions in the \a cil list

  \return The zero-based index into \a cil, or -1 if no matching
  conversion can be found.

*/
{
  /* immediate return if timeLabel has less than five characters */
  if (timeLabel.length()<5) return -1;

  Qt::CaseSensitivity cs=Qt::CaseInsensitive; int cnvID;

  /* immediate return if timeLabel is equal to "date" or "time" (case
     insensitive match) */
  if (timeLabel.compare(QString("date"),cs)==0 ||
      timeLabel.compare(QString("time"),cs)==0) return -1;

  /* try to find an entry in cil that contains timeLabel in its label
     (case insensitive match) */
  if ((cnvID=identifyConversion(timeLabel,cil))>-1) return cnvID;

  /* if timeLabel contains 'year', 'day', 'hour', 'minute', or
     'second', find in the list of conversions cil the entry that
     contains in its first label part this and the 'since' strings. */
  QString s,u; int i,j,nci=cil->size();
  if      (timeLabel.contains("day",cs))    u="day";
  else if (timeLabel.contains("hour",cs))   u="hour";
  else if (timeLabel.contains("minute",cs)) u="minute";
  else if (timeLabel.contains("second",cs)) u="second";
  else if (timeLabel.contains("year",cs))   u="year";
  if (!u.isEmpty() && (timeLabel.contains("since",cs)))
    {
      for (i=0; i<nci; ++i)
	{
	  s=cil->at(i).label; j=s.indexOf(">>"); if (j>-1) s=s.left(j);
	  if (s.contains(u,cs) && s.contains("since",cs)) return i;
	}
    }

  return -1;
}

/**************************************************************************/
/* conversion function info */

/* linear conversions */
extern DECLSPEC const QList<CNVINFO> linearConversionInfos=QList<CNVINFO>()
  << CNVINFO(CNV_IDENTITY,      "Identity Transformation")
  << CNVINFO(CNV_LINEAR,        "General Linear Transformation")
  << CNVINFO(CNV_LAT_DEGMINSEC, "Latitude [deg min] >> Latitude")
  << CNVINFO(CNV_LON_DEGMINSEC, "Longitude [deg min] >> Longitude")
  ;

/* conversions for meta variables during import */
extern DECLSPEC const QList<CNVINFO> metaVarConversionInfos=QList<CNVINFO>()
  << CNVINFO(CNV_IDENTITY,      "Identity Transformation")
  << CNVINFO(CNV_LINEAR,        "General Linear Transformation")
  << CNVINFO(CNV_ISODATETIME,   "Date as  yyyy-mm-ddThh:mm:ss.sss  >>  Date & Daytime")
  << CNVINFO(CNV_ISODATE,       "Date as  yyyy-mm-dd  >>  Date")
  << CNVINFO(CNV_DATE_YYYYMMDD, "Date as  yyyymmdd  >>  Date")
  << CNVINFO(CNV_DATE_YYYYMMDD1,"Date as  yyyy/mm/dd  >>  Date")
  << CNVINFO(CNV_DATE_MMDDYYYY, "Date as  mmddyyyy  >>  Date")
  << CNVINFO(CNV_DATE_MMDDYYYY1,"Date as  mm/dd/yyyy  >>  Date")
  << CNVINFO(CNV_DATE_DDMMYYYY, "Date as  ddmmyyyy  >>  Date")
  << CNVINFO(CNV_DATE_DDMMYYYY1,"Date as  dd/mm/yyyy  >>  Date")
  << CNVINFO(CNV_DATE_YMDF,     "Day as  %y%m%d.%f  >>  Date & Daytime")
  << CNVINFO(CNV_DATE_YYYYMMDD, "Day as  %y%m%d  >>  Date")
  << CNVINFO(CNV_DATE_MDF,      "Day as  %m%d.%f  >>  Date & Daytime")
  << CNVINFO(CNV_DATE_MD,       "Day as  %m%d  >>  Date")
  << CNVINFO(CNV_DATE_DF,       "Day as  .%f  >>  Date & Daytime")
  << CNVINFO(CNV_DATE_YMF,      "Month as  %y%m.%f  >>  Date")
  << CNVINFO(CNV_DATE_MF,       "Month as  %m.%f  >>  Date")
  << CNVINFO(CNV_DATE_YF,       "Year as  %y.%f  >>  Date")
  << CNVINFO(CNV_DATE_Y,        "Year as  %y  >>  Date")
  << CNVINFO(CNV_DATE_FY,       "Year as  .%f  >>  Date")
  << CNVINFO(CNV_TIME_HHMM1,    "Time as  hh:mm:ss.sss  >>  Daytime")
  << CNVINFO(CNV_TIME_HMS,      "Time as  hhmmss  >>  Daytime")
  << CNVINFO(CNV_TIME_HM,       "Time as  hhmm  >>  Daytime")
  << CNVINFO(CNV_TIME_DAYF,     "Fractional Day as  %d.%f  >>  Daytime")
  << CNVINFO(CNV_TIME_HF,       "Hour as  %h.%f  >>  Daytime")
  << CNVINFO(CNV_TIME_MF,       "Minute as  %m.%f  >>  Daytime")
  << CNVINFO(CNV_TIME_SF,       "Second as  %s.%f  >>  Daytime")
  << CNVINFO(CNV_RTIME_YS,      "Years since  yyyy-mm-ddThh:mm:ss.sss  >>  Date & Daytime")
  << CNVINFO(CNV_RTIME_DS,      "Days since  yyyy-mm-ddThh:mm:ss.sss  >>  Date & Daytime")
  << CNVINFO(CNV_RTIME_HS,      "Hours since  yyyy-mm-ddThh:mm:ss.sss  >>  Date & Daytime")
  << CNVINFO(CNV_RTIME_MS,      "Minutes since  yyyy-mm-ddThh:mm:ss.sss  >>  Date & Daytime")
  << CNVINFO(CNV_RTIME_SS,      "Seconds since yyyy-mm-ddThh:mm:ss.sss  >>  Date & Daytime")
  << CNVINFO(CNV_RTIME_MSS,     "Milliseconds since yyyy-mm-ddThh:mm:ss.sss  >>  Date & Daytime")
  << CNVINFO(CNV_LAT_DEGMINSEC, "Latitude [deg min] >> Latitude")
  << CNVINFO(CNV_LON_DEGMINSEC, "Longitude [deg min] >> Longitude")
;

/* conversions for data variables during import */
extern DECLSPEC const QList<CNVINFO> dataVarConversionInfos=QList<CNVINFO>()
  << CNVINFO(CNV_IDENTITY,     "Identity Transformation")
  << CNVINFO(CNV_PRESS2DEPTH,  "Pressure [dBar] >> Depth [m]")
  << CNVINFO(CNV_DEPTH2PRESS,  "Depth [m] >> Pressure [dBar]")
  << CNVINFO(CNV_THETA2T,      "Potential Temperature [degC] >> in situ Temperature [degC]")
  << CNVINFO(CNV_O2_ML2UMOL,   "Oxygen [ml/l] >> Oxygen [umol/kg]", 44.661/SWStdDensity)
  << CNVINFO(CNV_O2_ML2UMOLL,  "Oxygen [ml/l] >> Oxygen [umol/l]",               44.661)
  << CNVINFO(CNV_O2_UMOL2ML,   "Oxygen [umol/kg] >> Oxygen [ml/l]", SWStdDensity/44.661)
  << CNVINFO(CNV_O2_UMOLL2ML,  "Oxygen [umol/l] >> Oxygen [ml/l]",            1./44.661)
  << CNVINFO(CNV_PER_KG2PER_L, "Any [umol/kg] >> Any [umol/l]",            SWStdDensity)
  << CNVINFO(CNV_PER_L2PER_KG, "Any [umol/l] >> Any [umol/kg]",         1./SWStdDensity)
  << CNVINFO(CNV_CJDDATETIME,  "Chronological Julian Day >> Decimal Year")
  << CNVINFO(CNV_LINEAR,       "General Linear Transformation")
  ;

extern DECLSPEC const QList<CNVINFO> timeVarConversionInfos=QList<CNVINFO>()
  << CNVINFO(CNV_ISODATETIME,   "date as yyyy-mm-ddThh:mm:ss  >>  Time [yr]")
  << CNVINFO(CNV_ISODATETIME,   "time_ISO8601                 >>  Time [yr]")
  << CNVINFO(CNV_DATE_YYYYMMDD, "date as            YYYYMMDD  >>  Time [yr]")
  << CNVINFO(CNV_DATE_YYYYMMDD1,"date as          YYYY/MM/DD  >>  Time [yr]")
  << CNVINFO(CNV_DATE_MMDDYYYY, "date as            MMDDYYYY  >>  Time [yr]")
  << CNVINFO(CNV_DATE_MMDDYYYY1,"date as          MM/DD/YYYY  >>  Time [yr]")
  << CNVINFO(CNV_DATE_DDMMYYYY, "date as            DDMMYYYY  >>  Time [yr]")
  << CNVINFO(CNV_DATE_DDMMYYYY1,"date as          DD/MM/YYYY  >>  Time [yr]")
  << CNVINFO(CNV_DATE_YMDF,     "day as            %Y%M%D.%f  >>  Time [yr]")
  << CNVINFO(CNV_DATE_YYYYMMDD, "day as               %Y%M%D  >>  Time [yr]")
  << CNVINFO(CNV_DATE_MDF,      "day as              %M%D.%f  >>  Time [yr]")
  << CNVINFO(CNV_DATE_MD,       "day as                 %M%D  >>  Time [yr]")
  << CNVINFO(CNV_DATE_DF,       "day as                  .%f  >>  Time [yr]")
  << CNVINFO(CNV_DATE_YMF,      "month as            %Y%M.%f  >>  Time [yr]")
  << CNVINFO(CNV_DATE_MF,       "month as              %M.%f  >>  Time [yr]")
  << CNVINFO(CNV_DATE_YF,       "year as               %Y.%f  >>  Time [yr]")
  << CNVINFO(CNV_DATE_Y,        "year as                  %Y  >>  Time [yr]")
  << CNVINFO(CNV_DATE_FY,       "year as                 .%f  >>  Time [yr]")
  << CNVINFO(CNV_RTIME_DS,      "days since    yyyy-mm-ddThh:mm:ss  >>  Time [yr]")
  << CNVINFO(CNV_RTIME_HS,      "hours since   yyyy-mm-ddThh:mm:ss  >>  Time [yr]")
  << CNVINFO(CNV_RTIME_MS,      "minutes since yyyy-mm-ddThh:mm:ss  >>  Time [yr]")
  << CNVINFO(CNV_RTIME_SS,      "seconds since yyyy-mm-ddThh:mm:ss  >>  Time [yr]")
  << CNVINFO(CNV_RTIME_MSS,     "milliseconds since yyyy-mm-ddThh:mm:ss  >>  Time [yr]")
  << CNVINFO(CNV_CJDDATETIME,   "Chronological Julian Date [days]   >>  Time [yr]")
;

extern DECLSPEC const QList<CNVINFO> metaTimeConversionInfos=QList<CNVINFO>()
  << CNVINFO(CNV_ISODATETIME,   "date as yyyy-mm-ddThh:mm:ss  >>  Day of Year [days]")
  << CNVINFO(CNV_DATE_YYYYMMDD, "date as            YYYYMMDD  >>  Day of Year [days]")
  << CNVINFO(CNV_DATE_YYYYMMDD1,"date as          YYYY/MM/DD  >>  Day of Year [days]")
  << CNVINFO(CNV_DATE_YMDF,     "day as            %Y%M%D.%f  >>  Day of Year [days]")
  << CNVINFO(CNV_DATE_YYYYMMDD, "day as               %Y%M%D  >>  Day of Year [days]")
  << CNVINFO(CNV_DATE_MDF,      "day as              %M%D.%f  >>  Day of Year [days]")
  << CNVINFO(CNV_DATE_MD,       "day as                 %M%D  >>  Day of Year [days]")
  << CNVINFO(CNV_DATE_DF,       "day as                  .%f  >>  Day of Year [days]")
  << CNVINFO(CNV_DATE_YMF,      "month as            %Y%M.%f  >>  Day of Year [days]")
  << CNVINFO(CNV_DATE_MF,       "month as              %M.%f  >>  Day of Year [days]")
  << CNVINFO(CNV_DATE_YF,       "year as               %Y.%f  >>  Day of Year [days]")
  << CNVINFO(CNV_DATE_Y,        "year as                  %Y  >>  Day of Year [days]")
  << CNVINFO(CNV_DATE_FY,       "year as                 .%f  >>  Day of Year [days]")
  << CNVINFO(CNV_RTIME_YS,      "years since yyyy-mm-ddThh:mm:ss    >>  Day of Year [days]")
  << CNVINFO(CNV_RTIME_DS,      "days since yyyy-mm-ddThh:mm:ss     >>  Day of Year [days]")
  << CNVINFO(CNV_RTIME_HS,      "hours since yyyy-mm-ddThh:mm:ss    >>  Day of Year [days]")
  << CNVINFO(CNV_RTIME_MS,      "minutes since yyyy-mm-ddThh:mm:ss  >>  Day of Year [days]")
  << CNVINFO(CNV_RTIME_SS,      "seconds since yyyy-mm-ddThh:mm:ss  >>  Day of Year [days]")
  << CNVINFO(CNV_RTIME_MSS,     "milliseconds since yyyy-mm-ddThh:mm:ss  >>  Day of Year [days]")
  << CNVINFO(CNV_CJDDATETIME,   "Chronological Julian Date [days]   >>  Time [yr]")
;

/* conversions for aggregated Variables (see
   https://www.ices.dk/data/tools/Pages/Unit-conversions.aspx) */
extern DECLSPEC const QList<CNVINFO> aggregatedVarConversionInfos=QList<CNVINFO>()
  << CNVINFO(CNV_IDENTITY,     "Identity Transformation")
  << CNVINFO(CNV_LINEAR,       "General Linear Transformation")
  << CNVINFO(CNV_PRESS2DEPTH,  "Pressure [dBar] >> Depth [m]")
  << CNVINFO(CNV_DEPTH2PRESS,  "Depth [m] >> Pressure [dBar]")
  << CNVINFO(CNV_IPTS68toITS90,  "IPTS-68 [degC] >> ITS-90 [degC]", 1./1.00024)
  << CNVINFO(CNV_IPTS68toITS90,  "ITS-90 [degC] >> IPTS-68 [degC]", 1.00024)
  << CNVINFO(CNV_T_degK2degC,  "Temperature [degK] >> Temperature [degC]", 1., -1.*DegKAt0DegC)
  << CNVINFO(CNV_T_degC2degK,  "Temperature [degC] >> Temperature [degK]", 1., DegKAt0DegC)
  << CNVINFO(CNV_O2_ML2UMOL,   "Oxygen [ml/l] >> Oxygen [umol/kg]", 44.661/SWStdDensity)
  << CNVINFO(CNV_O2_ML2UMOLL,  "Oxygen [ml/l] >> Oxygen [umol/l]",               44.661)
  << CNVINFO(CNV_O2_UMOL2ML,   "Oxygen [umol/kg] >> Oxygen [ml/l]", SWStdDensity/44.661)
  << CNVINFO(CNV_O2_UMOLL2ML,  "Oxygen [umol/l] >> Oxygen [ml/l]",            1./44.661)
  << CNVINFO(CNV_PER_KG2PER_L, "Any [.../kg] >> Any [.../l]",              SWStdDensity)
  << CNVINFO(CNV_PER_L2PER_KG, "Any [.../l] >> Any [.../kg]",           1./SWStdDensity)
  ;

