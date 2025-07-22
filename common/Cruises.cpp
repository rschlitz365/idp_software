/****************************************************************************
**
** Copyright (C) 2025 Reiner Schlitzer. All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "Cruises.h"


/**************************************************************************/
CruisesDB::CruisesDB(const QString& fn,const QString& keyLabel,QChar splitChar)
  : InfoMap(fn,keyLabel,splitChar)
/**************************************************************************/
/*!

  \brief Creates a EventsDB object and appends the events from file \a fn.

*/
{
/* determine various column indexes */
  idxCruise=columnIndexOf("CRUISE");
  idxAliases=columnIndexOf("ALIASES");
  idxCountry=columnIndexOf("COUNTRY");
  idxShipName=columnIndexOf("SHIP_NAME");
  idxChiefScientist=columnIndexOf("CHIEF_SCIENTIST");
  idxStartTimeDate=columnIndexOf("CRUISE_START_TIME_DATE");
  idxEndTimeDate=columnIndexOf("CRUISE_END_TIME_DATE");
  idxLocation=columnIndexOf("LOCATION");
  idxGeotracesPi=columnIndexOf("GEOTRACES_PI");
  idxCruiseReportUrl=columnIndexOf("CRUISE_REPORT_URL");
  idxBodcCruiseNumber=columnIndexOf("BODC_CRUISE_NUMBER");
}
