#ifndef EVENTS_H
#define EVENTS_H

/****************************************************************************
**
** Copyright (C) 2025 Reiner Schlitzer. All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <QString>
#include <QStringList>

#include "globalDefines.h"
#include "InfoMap.h"

class CruisesDB;
class StationList;


/**************************************************************************/
class EventInfo
/**************************************************************************/
/*!

  \brief Container holding the information of one Event.

*/
{
public:
  EventInfo() { eventNumber=-1; };
  QString toString(const QString& sep) const;

  int eventNumber;
  QString cruiseLbl;
  QString stationLbl;
  QString castIdentifier;
  QString samplingDevice;
  double startGregorianDay;
  double endGregorianDay;
  double startLongitude;
  double startLatitude;
  double endLongitude;
  double endLatitude;
  double longitude;
  double latitude;
  double bottomDepth;
  IdpDataType dataType;
};



/**************************************************************************/
class EventsDB : public InfoMap
/**************************************************************************/
/*!

  \brief Container holding the set of Events.

*/
{
public:
  EventsDB(const QString& fn,const QString& keyLabel,QChar splitChar);

  void autoCorrectStationLabels();
  StationList collateStations(const QStringList& eventNumbers,
                              double distanceTolerance,
                              double timeTolerance,
                              EventsDB *eventsDB);
  StationList collateStationsByProximity(const QStringList& eventNumbers,
                                         double distanceTolerance,
                                         double timeTolerance,
                                         StationList& stLstByStLbl,
                                         EventsDB *eventsDB);
  StationList collateStationsByStationLabel(const QStringList& eventNumbers,
                                            QStringList& noNameEventNumbers,
                                            EventsDB *eventsDB);
  static void diagnoseEventCorrections();
  EventInfo eventInfoOf(const InfoItem& ii);
  EventInfo eventInfoOf(const QString& eventNumberStr);
  double gregorianDay(const QString& dateTimeStr);
  QStringList spreadsheetHeader();
  QStringList uniqueValuesFor(const QStringList& eventNumbers,int idx);

  int idxCruise,idxStation,idxEventNumber,idxCastIdentifier,idxSamplingDevice;
  int idxStartTimeDate,idxEndTimeDate,idxLongitude,idxLatitude,idxBottomDepth;
  int idxStartLongitude,idxEndLongitude,idxStartLatitude,idxEndLatitude;
};


#endif   // EVENTS_H
