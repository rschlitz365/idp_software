/****************************************************************************
**
** Copyright (C) 2025 Reiner Schlitzer. All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "Stations.h"

#include <QDir>

#include "globalFunctions.h"
#include "RRandomVar.h"
#include "common/odv.h"
#include "common/odvDate.h"


/**************************************************************************/
StationInfo::StationInfo(Station st)
/**************************************************************************/
/*!

  \brief Creates a StationInfo object for Station \a st.

*/
{
  int i,n=st.size(),nTime=0; EventInfo ei;
  double *d=(double*) malloc(8*n*sizeof(double));
  double *gds=d,*lons=(d+n),*lats=(d+2*n),*botds=(d+3*n);
  double *sLons=(d+4*n),*eLons=(d+5*n),*sLats=(d+6*n),*eLats=(d+7*n);
  double minTime=ODV::largeDOUBLE,maxTime=ODV::missDOUBLE;
  int posLonCount=0,negLonCount=0;

  for (i=0; i<n; ++i)
    {
      ei=st.eventInfos.at(i);

      // if (ei.eventNumber==1895715)
      //   nTime=0;

      insertLabel(ei.castIdentifier,castIdentifierMap);
      insertLabel(ei.samplingDevice,samplingDeviceMap);

      gds[i]=meanOf(ei.startGregorianDay,ei.endGregorianDay);
      lons[i]=ei.longitude;
      lats[i]=ei.latitude;
      botds[i]=ei.bottomDepth;
      sLons[i]=ei.startLongitude;
      sLats[i]=ei.startLatitude;
      eLons[i]=ei.endLongitude;
      eLats[i]=ei.endLatitude;

      if (ei.startGregorianDay!=ODV::missDOUBLE)
        {
          minTime=qMin(minTime,ei.startGregorianDay);
          maxTime=qMax(maxTime,ei.startGregorianDay); ++nTime;
        }
      if (ei.endGregorianDay!=ODV::missDOUBLE)
        {
          minTime=qMin(minTime,ei.endGregorianDay);
          maxTime=qMax(maxTime,ei.endGregorianDay); ++nTime;
        }

      /* count positive and negative longitude values */
      if      (ei.longitude>100.)  ++posLonCount;
      else if (ei.longitude<-100.) ++negLonCount;
    }

    duration=(nTime>0) ? (maxTime-minTime)*24. : ODV::missDOUBLE;

  /* correct longitude values to all positive, if we have positive and
    negative ones originally. this deals with issues of stations near
    the date line */
  if (posLonCount>0 && negLonCount>0)
    {
      for (i=0; i<n; ++i)
        {
          if (lons[i]<0.) lons[i]+=360.;
          if (sLons[i]<0.) sLons[i]+=360.;
          if (eLons[i]<0.) eLons[i]+=360.;
        }
    }

  RRandomVar lonRv(n,lons,ODV::missDOUBLE),latRv(n,lats,ODV::missDOUBLE);
  RRandomVar gdRv(n,gds,ODV::missDOUBLE),botdRv(n,botds,ODV::missDOUBLE);

  minLon=lonRv.minValue(); maxLon=lonRv.maxValue();
  meanLon=lonRv.mean(); sdvLon=lonRv.standardDeviation();
  minLat=latRv.minValue(); maxLat=latRv.maxValue();
  meanLat=latRv.mean(); sdvLat=latRv.standardDeviation();
  meanTime=gdRv.mean(); sdvTime=gdRv.standardDeviation();
  maxBotd=botdRv.maxValue(); sdvBotd=botdRv.standardDeviation();

  maxDist=0.;
  for (i=0; i<n; ++i)
    {
      maxDist=qMax(maxDist,distance(meanLon,meanLat,lons[i],lats[i]));
      maxDist=qMax(maxDist,distance(meanLon,meanLat,sLons[i],sLats[i]));
      maxDist=qMax(maxDist,distance(meanLon,meanLat,eLons[i],eLats[i]));
    }

  free(d);
}

/**************************************************************************/
QStringList StationInfo::castIdentifiers() const
/**************************************************************************/
/*!

  \brief \return The cast identifier labels of the station.

*/
{
  return castIdentifierMap.keys();
}

/**************************************************************************/
double StationInfo::distanceFrom(double refLongitude,double refLatitude)
/**************************************************************************/
/*!

  \brief \return The distance in km of this station from reference
  position \a refLongitude / \a refLatitude.

*/
{
  return distance(meanLon,meanLat,refLongitude,refLatitude);
}

/**************************************************************************/
void StationInfo::insertLabel(const QString& lbl,QMap<QString,int>& lblMap)
/**************************************************************************/
/*!

  \brief Appends label \a lbl to \a lblMap as key and increments the value.

*/
{
  if (lblMap.contains(lbl))
    {
      int i=lblMap.value(lbl);
      lblMap.insert(lbl,i+1);
    }
  else
    lblMap.insert(lbl,1);
}

/**************************************************************************/
QString StationInfo::isoDateString(double gregDay)
/**************************************************************************/
/*!

  \brief \return The station label of the station.

*/
{
  return isoDateFromGregorianDay(gregDay);
}

/**************************************************************************/
QStringList StationInfo::samplingDevices() const
/**************************************************************************/
/*!

  \brief \return The sampling device labels of the station.

*/
{
  return samplingDeviceMap.keys();
}

/**************************************************************************/
double StationInfo::timeFrom(double refGregDay)
/**************************************************************************/
/*!

  \brief \return The time difference in days of this station from
  reference Gregorian day \a refGregDay.

*/
{
  return meanTime-refGregDay;
}



/**************************************************************************/
Station::Station()
/**************************************************************************/
/*!

  \brief Creates an empty Station object.

*/
{
}

/**************************************************************************/
Station::Station(EventsDB *eventsDB,const QString& bodcEventNumber)
  : Station()
/**************************************************************************/
/*!

  \brief Creates a Station object and appends the event \a
  bodcEventNumber.

*/
{
  /* immediate return if event bodcEventNumber does not exist */
  if (!eventsDB || !eventsDB->contains(bodcEventNumber)) return;
  addEvent(eventsDB,bodcEventNumber);
}

/**************************************************************************/
bool Station::addEvent(EventsDB *eventsDB,const QString& bodcEventNumber)
/**************************************************************************/
/*!

  \brief Adds event \a bodcEventNumber to this station.

  \return \true if the event was added, or \c false if the event does
  not exist or is already present.
*/
{
  /* immediate return if station already contains bodcEventNumber or
    bodcEventNumber is not in the events set */
  if (contains(bodcEventNumber) ||
      !eventsDB->contains(bodcEventNumber)) return false;

  EventInfo ei=eventsDB->eventInfoOf(bodcEventNumber);

  /* remember cruise label if this is the first event */
  if (isEmpty()) cruiseLbl=ei.cruiseLbl;

  if (cruiseLbl==ei.cruiseLbl)
    {
      append(bodcEventNumber); eventInfos.append(ei);
      addStationLabel(ei.stationLbl);
      return true;
    }
  else
    return false;
}

/**************************************************************************/
void Station::addStation(const Station& st)
/**************************************************************************/
/*!

  \brief Adds all events from other station \a st to this station.

*/
{
  if (cruiseLbl!=st.cruiseLbl) return;

  int i,n=st.size();
  for (i=0; i<n; ++i)
    {
      append(st.at(i)); eventInfos.append(st.eventInfos.at(i));
    }
}

/**************************************************************************/
void Station::addStationLabel(const QString& lbl)
/**************************************************************************/
/*!

  \brief Adds station lbl \a lbl to this station.

*/
{
  if (!(lbl.isEmpty() || stationLbls.contains(lbl)))
    stationLbls.append(lbl);
}

/**************************************************************************/
EventInfo Station::eventInfoAt(int idx)
/**************************************************************************/
/*!

  \brief \return The EventInfo object for event at index \a idx.

*/
{
  return eventInfos.at(idx);
}

/**************************************************************************/
QStringList Station::spreadsheetRecords() const
/**************************************************************************/
/*!

  \brief The ODV spreadsheet records for this station.

  \sa EventsDB::spreadsheetHeader()

*/
{
  StationInfo si(*this); QString s,statLbl=stationLabel();
  QStringList sl; EventInfo ei; int i,n=size(); double sdvDist;

  for (i=0; i<n; ++i)
    {
      ei=eventInfos.at(i);
      sdvDist=(si.sdvLon==ODV::missDOUBLE || si.sdvLat==ODV::missDOUBLE) ?
        ODV::missDOUBLE : distance(si.meanLon,si.meanLat,
                                   si.meanLon+si.sdvLon,si.meanLat-si.sdvLat);

      s=QString("%1\t%2\tB\t%3\t%4\t%5\t%6\t%7\t%8\t%9\t%10\t%11\t%12\t%13\t%14\t%15\t%16\t%17\t")
        .arg(cruiseLbl).arg(statLbl).arg(isoDateFromGregorianDay(si.meanTime))
        .arg(si.meanLon).arg(si.meanLat).arg(si.maxBotd)
        .arg(si.sdvTime).arg(sdvDist).arg(si.sdvLon).arg(si.sdvLat)
        .arg(si.sdvBotd).arg(ei.eventNumber)
        .arg(si.sdvTime).arg(sdvDist).arg(si.sdvLon).arg(si.sdvLat).arg(si.sdvBotd);
      s+=ei.toString("\t");
      sl.append(s);
    }

  return sl;
}

/**************************************************************************/
QString Station::stationKey(const QString& cruiseLbl,const QString& stationLbl)
/**************************************************************************/
/*!

  \brief \return The station key composed of \a cruiseLbl and \a stationLbl.

*/
{
  return cruiseLbl+"::"+stationLbl;
}

/**************************************************************************/
QString Station::stationLabel() const
/**************************************************************************/
/*!

  \brief \return The station label of this station.

*/
{
  return stationLbls.isEmpty() ?
    QString("<%1>").arg(rand() % 10000) :
    stationLbls.join(" | ");
}


/**************************************************************************/
/**************************************************************************/


/**************************************************************************/
void StationList::autoAssignStationLabels()
/**************************************************************************/
/*!

  \brief Auto-assigns empty station labels in this Station list.

*/
{
  QStringList cruiseLbls=cruiseLabelList(); QString cruise; Station st;
  int i,j,cruiseCount=cruiseLbls.size(),stationCount=size(),nextStNumber;
  QMap<double,int> sortedStats; QMap<double,int>::ConstIterator it;

  for (i=0; i<cruiseCount; ++i)
    {
      cruise=cruiseLbls.at(i); sortedStats.clear(); nextStNumber=0;
      for (j=0; j<stationCount; ++j)
        {
          StationInfo si(at(j));
          if (at(j).cruiseLbl==cruise && at(j).stationLbls.isEmpty())
            {
              if (sortedStats.contains(si.meanTime)) si.meanTime+=0.01;
              sortedStats.insert(si.meanTime,j);
            }
        }

      for (it=sortedStats.constBegin(); it!=sortedStats.constEnd(); ++it)
        {
          j=it.value(); st=at(j); st.stationLbls.clear();
          st.stationLbls.append(QString("(%1)").arg(++nextStNumber));
          replace(j,st);
        }
    }
}

/**************************************************************************/
QStringList StationList::cruiseLabelList() const
/**************************************************************************/
/*!

  \brief The unique cruise label list for all stations in this list.

*/
{
  int i,n=size(); QMap<QString,int> vals;

  for (i=0; i<n; ++i)
    vals.insert(at(i).cruiseLbl,1);

  return vals.keys();
}

/**************************************************************************/
QStringList StationList::spreadsheetRecords() const
/**************************************************************************/
/*!

  \brief \return The ODV spreadsheet records for all stations in this
  list.

  \sa Station::spreadsheetRecords()

*/
{
  int i,n=size(); QStringList sl;

  for (i=0; i<n; ++i)
    sl.append(at(i).spreadsheetRecords());

  return sl;
}

/**************************************************************************/
void StationList::writeSpreadsheetFile(const QString& dir,const QString& fn,
                                       EventsDB *eventsDB)
/**************************************************************************/
/*!

  \brief Writes ODV spreadsheet file \a fn containing all stations in
  this list.

*/
{
  if (isEmpty()) return;

  QDir().mkpath(dir); //ensure output directory exists

  appendRecords(dir+fn,eventsDB->spreadsheetHeader(),true);
  appendRecords(dir+fn,spreadsheetRecords(),false);
}


