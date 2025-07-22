#ifndef STATIONS_H
#define STATIONS_H

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
#include "Events.h"
#include "InfoMap.h"

class Station;

/**************************************************************************/
class StationInfo
/**************************************************************************/
/*!

  \brief Container holding the information of one Station.

*/
{
public:
  StationInfo(Station st);

  QStringList castIdentifiers() const;
  double distanceFrom(double refLongitude,double refLatitude);
  void insertLabel(const QString& lbl,QMap<QString,int>& lblMap);
  static QString isoDateString(double gregDay);
  QStringList samplingDevices() const;
  double timeFrom(double refGregDay);

  QMap<QString,int> castIdentifierMap;
  QMap<QString,int> samplingDeviceMap;

  double minLon;   //!< minimal longitude
  double maxLon;   //!< maximal longitude
  double meanLon;  //!< mean longitude
  double sdvLon;   //!< standard deviation longitude
  double minLat;   //!< minimal latitude
  double maxLat;   //!< maximal latitude
  double meanLat;  //!< mean latitude
  double sdvLat;   //!< standard deviation latitude
  double duration;  //!< maximal Gregorian day
  double meanTime; //!< mean time as Gregorian days
  double sdvTime;  //!< standard deviation time as Gregorian days
  double maxBotd;  //!< maximal bottom depth [m]
  double sdvBotd;  //!< standard deviation bottom depth [m]
  double maxDist;  //!< maximal distance [km] of events from mean position
};


/**************************************************************************/
class Station : public QStringList
/**************************************************************************/
/*!

  \brief A station consists of one or more events and has Cruise label
  and a list of unique Station label takens from the contributing
  events.

  Mean and standard deviation for longitude, latitude and time, as
  well as maximum and standard deviation for bottom depth can be obtained
  by creating a StationInfo object for this station.

*/
{
public:
  Station();
  Station(EventsDB *eventsDB,const QString& bodcEventNumber);
  bool addEvent(EventsDB *eventsDB,const QString& bodcEventNumber);
  void addStation(const Station& st);
  void addStationLabel(const QString& lbl);
  EventInfo eventInfoAt(int idx);
  QStringList spreadsheetRecords() const;
  static QString stationKey(const QString& cruiseLbl,const QString& stationLbl);
  QString stationLabel() const;

  QString cruiseLbl;
  QStringList stationLbls;
  QList<EventInfo> eventInfos; //!< list of event infos in this station
};



/**************************************************************************/
class StationList : public QList<Station>
/**************************************************************************/
/*!

  \brief A station has non-empty Cruise and Station names and consists
  of one or more events.

*/
{
public:
  void autoAssignStationLabels();
  QStringList cruiseLabelList() const;
  QStringList spreadsheetRecords() const;
  void writeSpreadsheetFile(const QString& dir,const QString& fn,EventsDB *eventsDB);
};


#endif   // STATIONS_H
