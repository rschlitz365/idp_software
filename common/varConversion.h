#ifndef VARCONVERSION_H
#define VARCONVERSION_H

/****************************************************************************
**
** Copyright (C) 1993-2025 Reiner Schlitzer. All rights reserved.
**
** This file is part of Ocean Data View.
**
** Global functions: varConversion.h
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <QtGlobal>
#include <QList>
#include <QString>

#include "common/declspec.h"

/****************************************************************************
** GENERAL ENUMERATIONS
****************************************************************************/

/* conversion types (a date consists of year, month and day. daytime
   consists of hour, minute and seconds. all dates aren on the
   Gregorian calendar) */
enum ConversionType {
  /* linear transformations */
  CNV_IDENTITY=0,       //!< X                     -> X
  CNV_LINEAR=1,         //!< X                     -> f*X + o
  /* date/time conversions */
  CNV_DATE_YYYYMMDD=2,  //!< YYYYMMDD              -> date
  CNV_DATE_YMDF=3,      //!< YYYYMMDD.fff          -> date + daytime
  CNV_DATE_MDF=4,       //!< MMDD.fff              -> date + daytime (year=0)
  CNV_DATE_MD=5,        //!< MMDD                  -> date (year=0)
  CNV_DATE_DF=6,        //!< DD.fff                -> date + daytime (year=0,month=1)
  CNV_DATE_YMF=7,       //!< YYYYMM.fff            -> date + daytime
  CNV_DATE_MF=8,        //!< MM.fff                -> date + daytime (year=0)
  CNV_DATE_YF=9,        //!< YYYY.fff              -> date + daytime
  CNV_DATE_Y=10,        //!< YYYY                  -> year
  CNV_DATE_FY=11,       //!< Year as 0.fff         -> date + daytime (year=0)
  CNV_TIME_HMS=12,      //!< hhmmss                -> daytime
  CNV_TIME_HM=13,       //!< hhmm                  -> daytime
  CNV_TIME_DAYF=14,     //!< hh.fff                -> daytime
  CNV_TIME_HF=15,       //!< hh.fff                -> daytime
  CNV_TIME_MF=16,       //!< mm.fff                -> daytime
  CNV_TIME_SF=17,       //!< sec.fff               -> daytime
  CNV_TIME_HHMM=18,     //!< hhmm                  -> daytime
  CNV_RTIME_YS=30,      //!< Years since YYYY-MM-DDThh:mm:ss.sss   -> date + daytime
  CNV_RTIME_DS=31,      //!< Days since YYYY-MM-DDThh:mm:ss.sss    -> date + daytime
  CNV_RTIME_HS=32,      //!< Hours since YYYY-MM-DDThh:mm:ss.sss   -> date + daytime
  CNV_RTIME_MS=33,      //!< Minutes since YYYY-MM-DDThh:mm:ss.sss -> date + daytime
  CNV_RTIME_SS=34,      //!< Seconds since YYYY-MM-DDThh:mm:ss.sss -> date + daytime
  CNV_RTIME_MSS=35,     //!< Milliseconds since YYYY-MM-DDThh:mm:ss.sss -> date + daytime
  CNV_TIME_HHMM1=40,    //!< hh:mm                 -> daytime
  CNV_DATE_MMDDYYYY=50, //!< MMDDYYYY              -> date
  CNV_DATE_MMDDYYYY1=-1,//!< MM/DD/YYYY            -> date
  CNV_DATE_DDMMYYYY=51, //!< DDMMYYYY              -> date
  CNV_ISODATETIME=52,   //!< YYYY-MM-DDThh:mm:ss.s -> date + daytime
  CNV_DATE_DDMMYYYY1=53,//!< DD/MM/YYYY            -> date
  CNV_DATE_YYYYMMDD1=54,//!< YYYY/MM/DD            -> date
  CNV_CJDDATETIME=55,   //!< Decimal Chronological Julian day  -> date + daytime
  CNV_ISODATE=56,       //!< YYYY-MM-DD            -> date
  /* lat/lon conversions */
  CNV_LAT_DEGMINSEC=75, //!< Degree, minute[, second]  -> decimal latitude
  CNV_LON_DEGMINSEC=76, //!< Degree, minute[, second]  -> decimal longitude
  /* temperrature conversions https://www.teos-10.org/pubs/gsw/pdf/t90_from_t48.pdf */
  CNV_IPTS68toITS90=80, //!< IPTS-68 -> ITS-90
  CNV_ITS90toIPTS68=81, //!< ITS-90 -> IPTS-68
  CNV_T_degK2degC=82,   //!< Kelvin -> Celcius
  CNV_T_degC2degK=83,   //!< Celcius -> Kelvin
  /* property conversions (ODV3.x value + 100) */
  CNV_PRESS2DEPTH=101,
  CNV_DEPTH2PRESS=102,
  CNV_THETA2T=103,
  CNV_O2_ML2UMOL=104,
  CNV_O2_UMOL2ML=105,
  CNV_PER_KG2PER_L=106,
  CNV_PER_L2PER_KG=107,
  CNV_O2_ML2UMOLL=109,
  CNV_O2_UMOLL2ML=110
 };

/**************************************************************************/
/*!
  \struct CNVINFO
  \brief Conversion info
*/
struct DECLSPEC CNVINFO
/**************************************************************************/
{
  CNVINFO()
    { type=CNV_IDENTITY; fac=1.; off=0.; }
  CNVINFO(ConversionType t,const QString& l,double f=1.,double o=0.)
    { type=t; label=l; fac=f; off=o; }

  ConversionType type; //!< Type of the conversion
  QString label;       //!< Descriptive label of the conversion
  double fac;          //!< Factor of the conversion
  double off;          //!< Offset of the conversion
};

/* predefined conversion lists */
extern DECLSPEC const QList<CNVINFO> linearConversionInfos;
//!< Linear conversions
extern DECLSPEC const QList<CNVINFO> metaVarConversionInfos;
//!< Conversions for meta variables during import
extern DECLSPEC const QList<CNVINFO> dataVarConversionInfos;
//!< Conversions for data variables during import
extern DECLSPEC const QList<CNVINFO> timeVarConversionInfos;
extern DECLSPEC const QList<CNVINFO> metaTimeConversionInfos;
extern DECLSPEC const QList<CNVINFO> aggregatedVarConversionInfos;
//!< Conversions for aggregated variables

DECLSPEC
int identifyConversion(const QString& label,const QList<CNVINFO> *cil);
DECLSPEC
int identifyTimeConversion(const QString& timeLabel,const QList<CNVINFO> *cil);

#endif // !VARCONVERSION_H
