/****************************************************************************
**
** Copyright (C) 1993-2025 Reiner Schlitzer. All rights reserved.
**
** This file is part of Ocean Data View.
**
** It contains globally used constants.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <QDataStream>

#include "common/declspec.h"
#include "common/odv.h"

/* general constants */
extern DECLSPEC const double PI=3.14159265358979323846;    // pi
extern DECLSPEC const double HALFPI=1.5707963267948966;    // pi/2
extern DECLSPEC const double FORTHPI=0.78539816339744833;  // pi/4
extern DECLSPEC const double TWOPI=6.2831853071795864769;  // 2 pi
extern DECLSPEC const double RAD2DEG=57.29577951308232;
extern DECLSPEC const double DEG2RAD=0.0174532925199432958;
extern DECLSPEC const double CMPERINCH=2.54;
extern DECLSPEC const double PTPERCM=28.35;
extern DECLSPEC const double ONEOVER3600=1./3600.;
extern DECLSPEC const double ONEOVER60=1./60.;
extern DECLSPEC const double ONEOVER24=1./24.;
extern DECLSPEC const double EARTHMEANRADIUS=6371.0088;    // km

/*! Gas constant. james.rae@bristol.ac.uk 2010-03-20: NIST and Dickson
   2007 recommended R value is 83.14472; 83.145 was earlier value */
extern DECLSPEC const double RGasConstant=83.14472;

/* Kelvin/Celsius offset */
extern DECLSPEC const double DegKAt0DegC=273.15;

/* Oceanic Constants */
extern DECLSPEC const double OceanDepthMax=20000.;
extern DECLSPEC const double SWStdDensity=1.025;    // kg/l

/* missing value indicators */
const qint8   ODV::missINT8=(qint8) 127;
const quint8  ODV::missUINT8=(quint8) 255;
const qint16  ODV::missINT16=(qint16) -32768;
const quint16 ODV::missUINT16=(quint16) 65535;
const qint32  ODV::missINT32=-2147483646;
const quint32 ODV::missUINT32=4294967295U;
const qint64  ODV::missINT64=(-9223372036854775807LL-1);
const quint64 ODV::missUINT64=(18446744073709551615ULL);
const float   ODV::missFLOAT=(float) -1.e10;
const double  ODV::missDOUBLE=-1.e10;

/* large value indicators */
const qint8   ODV::largeINT8=(qint8) 127;
const quint8  ODV::largeUINT8=(quint8) 255;
const qint16  ODV::largeINT16=(qint16) 32767;
const quint16 ODV::largeUINT16=(quint16) 65535;
const qint32  ODV::largeINT32=2147483647;
const quint32 ODV::largeUINT32=4294967294U;
const qint64  ODV::largeINT64=(9223372036854775807LL);
const quint64 ODV::largeUINT64=(18446744073709551614ULL);
const float   ODV::largeFLOAT=(float) 1.e10;
const double  ODV::largeDOUBLE=1.e10;

/* special value indicators */
const quint32 ODV::specialUINT32=4294967293U;

/* Version for QDataStream binary files */
const qint32  ODV::STREAMVERSION=QDataStream::Qt_4_4;

/* choice values */
extern DECLSPEC const double  XCHOICE0=ODV::missDOUBLE;
extern DECLSPEC const double  XCHOICE1=ODV::largeDOUBLE;
extern DECLSPEC const double  XCHOICE2=2.e10;
extern DECLSPEC const double  XCHOICE3=3.e10;
extern DECLSPEC const double  XCHOICE4=4.e10;
extern DECLSPEC const double  XCHOICE5=5.e10;
extern DECLSPEC const double  XCHOICE6=6.e10;
extern DECLSPEC const double  XCHOICE7=7.e10;
extern DECLSPEC const double  XCHOICE8=8.e10;
extern DECLSPEC const double  XCHOICE9=9.e10;

/* epsilon: difference between 1 and the smallest value greater than 1 */
extern DECLSPEC const float  xeps=(float) 1.2e-07;
extern DECLSPEC const double XEPS=2.3e-16;

/* pre-computed exp(-x) */
extern DECLSPEC const double EXPARGINC=0.1;
extern DECLSPEC const double EXPARGIINC=10.;  // 1/EXPARGINC
extern DECLSPEC const double EXPMAXARG=20.;   // (MAXEXPVALUES-1)*EXPARGINC

