#ifndef CONSTANTS_H
#define CONSTANTS_H

/****************************************************************************
**
** Copyright (C) 1993-2025 Reiner Schlitzer. All rights reserved.
**
** This file is part of Ocean Data View.
**
** It provides globally used constants.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "common/declspec.h"

/* named colors */
#define COLOR_AUTOMATIC   -2
#define COLOR_NONE        -1
#define COLOR_BLACK        0
#define COLOR_BLUE         1
#define COLOR_GREEN        2
#define COLOR_RED         12
#define COLOR_WHITE       15

/* limits */
#define MAXDOMSELPOINTS   1500	/*!< maximum points for domain selection  */
#define MAXPTCPOINTS        20	/*!< maximum points for patch selection  */

/* general constants */
extern DECLSPEC const double PI;
extern DECLSPEC const double HALFPI;
extern DECLSPEC const double FORTHPI;
extern DECLSPEC const double TWOPI;
extern DECLSPEC const double RAD2DEG;
extern DECLSPEC const double DEG2RAD;
extern DECLSPEC const double CMPERINCH;
extern DECLSPEC const double PTPERCM;
extern DECLSPEC const double ONEOVER3600;
extern DECLSPEC const double ONEOVER60;
extern DECLSPEC const double ONEOVER24;
extern DECLSPEC const double EARTHMEANRADIUS;

/* Gas constant */
extern DECLSPEC const double RGasConstant;

/* Kelvin/Celsius offset */
extern DECLSPEC const double DegKAt0DegC;

/* Oceanic Constants */
extern DECLSPEC const double OceanDepthMax;
extern DECLSPEC const double SWStdDensity;

/* choice values */
extern DECLSPEC const double XCHOICE0;
extern DECLSPEC const double XCHOICE1;
extern DECLSPEC const double XCHOICE2;
extern DECLSPEC const double XCHOICE3;
extern DECLSPEC const double XCHOICE4;
extern DECLSPEC const double XCHOICE5;
extern DECLSPEC const double XCHOICE6;
extern DECLSPEC const double XCHOICE7;
extern DECLSPEC const double XCHOICE8;
extern DECLSPEC const double XCHOICE9;

/* epsilon: difference between 1 and the smallest value greater than 1 */
extern DECLSPEC const float  xeps;
extern DECLSPEC const double XEPS;

/* pre-computed exp(-x) */
#define MAXEXPVALUES	201
extern DECLSPEC const double EXPARGINC;
extern DECLSPEC const double EXPARGIINC;  // 1/EXPARGINC
extern DECLSPEC const double EXPMAXARG;   // (MAXEXPVALUES-1)*EXPARGINC

#endif // CONSTANTS_H
