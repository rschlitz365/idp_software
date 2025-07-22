#ifndef CRUISES_H
#define CRUISES_H

/****************************************************************************
**
** Copyright (C) 2025 Reiner Schlitzer. All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <QString>

#include "globalDefines.h"
#include "InfoMap.h"


/**************************************************************************/
class CruisesDB : public InfoMap
/**************************************************************************/
/*!

  \brief Container holding the set of cruises.

*/
{
public:
  CruisesDB(const QString& fn,const QString& keyLabel,QChar splitChar);

  int idxCruise,idxAliases,idxCountry,idxShipName,idxChiefScientist;
  int idxStartTimeDate,idxEndTimeDate,idxLocation,idxGeotracesPi;
  int idxCruiseReportUrl,idxBodcCruiseNumber;
};


#endif   // CRUISES_H
