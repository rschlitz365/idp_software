/****************************************************************************
**
** Copyright (C) 2025 Reiner Schlitzer. All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "Events.h"

#include <QDir>
// #include <QFile>
// #include <QMap>
// #include <QTextStream>
// #include <QVector>

#include "globalVars.h"
#include "globalFunctions.h"
// #include "Params.h"
// #include "RRandomVar.h"
#include "Stations.h"

// #include "common/odv.h"
#include "common/odvDate.h"
#include "common/RDateTime.h"


/**************************************************************************/
QString EventInfo::toString(const QString& sep) const
/**************************************************************************/
/*!

  \brief \return The concatenated values as string using \a sep as
  separation string.

*/
{
  return cruiseLbl
    +sep+stationLbl
    +sep+QString::number(eventNumber)
    +sep+castIdentifier
    +sep+samplingDevice
    +sep+formattedNumber(startGregorianDay,6)
    +sep+formattedNumber(endGregorianDay,6)
    +sep+formattedNumber(longitude,4)
    +sep+formattedNumber(latitude,4)
    +sep+formattedNumber(startLatitude,4)
    +sep+formattedNumber(startLongitude,4)
    +sep+formattedNumber(endLatitude,4)
    +sep+formattedNumber(endLongitude,4)
    +sep+QString::number(bottomDepth);
}


/**************************************************************************/
/**************************************************************************/



/**************************************************************************/
EventsDB::EventsDB(const QString& fn,const QString& keyLabel,QChar splitChar)
  : InfoMap(fn,keyLabel,splitChar)
/**************************************************************************/
/*!

  \brief Creates a EventsDB object and appends the events from file \a fn.

*/
{
  /* determine various column indexes */
  idxCruise=columnIndexOf("CRUISE");
  idxStation=columnIndexOf("STATION");
  idxEventNumber=columnIndexOf("BODC_EVENT_NUMBER");
  idxCastIdentifier=columnIndexOf("CAST_IDENTIFIER");
  idxSamplingDevice=columnIndexOf("SAMPLING_DEVICE");
  idxStartTimeDate=columnIndexOf("EVENT_START_TIME_DATE");
  idxEndTimeDate=columnIndexOf("EVENT_END_TIME_DATE");
  idxStartLongitude=columnIndexOf("EVENT_START_LONGITUDE");
  idxEndLongitude=columnIndexOf("EVENT_END_LONGITUDE");
  idxStartLatitude=columnIndexOf("EVENT_START_LATITUDE");
  idxEndLatitude=columnIndexOf("EVENT_END_LATITUDE");
  idxLongitude=columnIndexOf("LONGITUDE");
  idxLatitude=columnIndexOf("LATITUDE");
  idxBottomDepth=columnIndexOf("BOTTOM DEPTH [M]");
}

/**************************************************************************/
void EventsDB::autoCorrectStationLabels()
/**************************************************************************/
/*!

  \brief Auto-corrects station and/or cast identifier labels in "official"
  event records by removing cruise prefixes.

*/
{
  QMap<QString,InfoItem>::ConstIterator it;
  QString key,cruise,stLbl,castLbl,s; InfoItem ii;
  for (it=constBegin(); it!=constEnd(); ++it)
    {
      key=it.key(); ii=it.value(); cruise=ii.at(idxCruise);
      if      (cruise=="PS71")
        {
          ii[idxStation].replace("PS71/","");
          insert(key,ii);
        }
      else if (cruise=="PS100")
        {
          ii[idxStation].replace("PS100/","");
          insert(key,ii);
        }
      else if (cruise=="PS94")
        {
          ii[idxStation].replace("PS94/","");
          insert(key,ii);
        }
      else if (cruise=="SK304")
        {
          s=ii.at(idxCastIdentifier); s.replace("SK-304A/","");
          ii[idxStation]=s.replace("SK-304B/","");
          insert(key,ii);
        }
      else if (cruise=="SK311")
        {
          s=ii.at(idxCastIdentifier);
          ii[idxStation]=s.replace("SK-311/","");
          insert(key,ii);
        }
      else if (cruise=="SK312")
        {
          s=ii.at(idxCastIdentifier);
          ii[idxStation]=s.replace("SK - 312/","");
          insert(key,ii);
        }
      else if (cruise=="SK324")
        {
          s=ii.at(idxCastIdentifier);
          ii[idxStation]=s.replace("SK-324/","");
          insert(key,ii);
        }
      else if (cruise=="SK338")
        {
          s=ii.at(idxCastIdentifier);
          ii[idxStation]=s.replace("SK-338A/","");
          insert(key,ii);
        }
    }
}

/**************************************************************************/
StationList EventsDB::collateStations(const QStringList& eventNumbers,
                                      double distanceTolerance,
                                      double timeTolerance,
                                      EventsDB *eventsDB)
/**************************************************************************/
/*!

  \brief Auto-collates stations based on the list of event number \a
  eventNumbers.

  First performs collation by station label, then the events without
  station label are collated by proximity using tolerances \a
  distanceTolerance (in km) and \a timeTolerance (in days). Empty
  station labels are assigned automatically.

  \return The combined list of collated stations.

*/
{
  QStringList noStNameEvents;

  StationList stations=
    collateStationsByStationLabel(eventNumbers,noStNameEvents,eventsDB);
  StationList stationsNn=
    collateStationsByProximity(noStNameEvents,distanceTolerance,
                               timeTolerance,stations,eventsDB);

  stationsNn.autoAssignStationLabels();

  StationList sl; sl.append(stations); sl.append(stationsNn);
  return sl;
}

/**************************************************************************/
StationList EventsDB::collateStationsByProximity(const QStringList& eventNumbers,
                                                 double distanceTolerance,
                                                 double timeTolerance,
                                                 StationList& stLstByStLbl,
                                                 EventsDB *eventsDB)
/**************************************************************************/
/*!

  \brief Auto-collates stations based on the list of event number \a
  eventNumbers.

  For collation to occur, the distance between events has to be
  smaller than \a distanceTolerance (in km) and time difference
  smaller than \a timeTolerance (in days).

  \return The list of collated stations.

*/
{
  if (eventNumbers.isEmpty()) return StationList();

  QStringList evtNumbers=eventNumbers;
  StationList stations; Station st,stRef;
  int i,j,m,n; double dTime,dDist,rTime,rLon,rLat;
  InfoItem ii; EventInfo ei;
  QString evtNumber,stationLbl,stationKey;

  do
    {
      st=Station(this,evtNumbers.takeFirst()); n=evtNumbers.size()-1;
      for (i=n; i>=0; --i)
        {
          StationInfo si(st);
          evtNumber=evtNumbers.at(i); ei=eventInfoOf(evtNumber);
          dTime=si.timeFrom(meanOf(ei.startGregorianDay,ei.endGregorianDay));
          dDist=si.distanceFrom(ei.longitude,ei.latitude);
          if (fabs(dTime)<timeTolerance && dDist<distanceTolerance)
            { if (st.addEvent(eventsDB,evtNumber)) evtNumbers.takeAt(i); }
        }
      stations.append(st);
    }
  while (!evtNumbers.isEmpty());

  /* loop over all stations and see whether any one matches one of the
  staions in stLstByStLbl (collated by station label) within given
  position and time tolerances. if a match is found the station
  without station label is appended to the station with station label*/
  if ((m=stLstByStLbl.size())>0)
    {
      n=stations.size()-1;
      for (i=n; i>=0; --i)
        {
          st=stations.at(i); StationInfo si(st);
          rTime=si.meanTime; rLon=si.meanLon; rLat=si.meanLat;
          for (j=0; j<m; ++j)
            {
              stRef=stLstByStLbl.at(j); si=StationInfo(stRef);
              dTime=si.timeFrom(rTime); dDist=si.distanceFrom(rLon,rLat);
              if (fabs(dTime)<timeTolerance && dDist<distanceTolerance)
                {
                  stRef.addStation(st); stLstByStLbl[j]=stRef;
                  stations.takeAt(i); break;
                }
            }
        }
    }

  return stations;
}

/**************************************************************************/
StationList EventsDB::
collateStationsByStationLabel(const QStringList& eventNumbers,
                              QStringList& noNameEventNumbers,
                              EventsDB *eventsDB)
/**************************************************************************/
/*!

  \brief Auto-collates stations based on the list of event number \a
  eventNumbers.

  For collation to occur, the cruise and station labels of events has
  to match.

  \return The list of collated stations. On exit, \a
  noNameEventNumbers containts the list of event numbers with empty
  station labels.

*/
{
  QMap<QString,Station> stations; InfoItem ii;
  int i,n=eventNumbers.size();
  QString evtNumber,stationLbl,stationKey;

  for (i=0; i<n; ++i)
    {
      evtNumber=eventNumbers.at(i); ii=value(evtNumber);
      stationLbl=ii.at(idxStation);
      stationKey=Station::stationKey(ii.at(idxCruise),stationLbl);

      if (stationLbl.isEmpty())
        {
          noNameEventNumbers.append(evtNumber);
        }
      else
        {
          if (stations.contains(stationKey))
            {
              Station station=stations.value(stationKey);
              station.addEvent(eventsDB,evtNumber);
              stations.insert(stationKey,station);
            }
          else
            stations.insert(stationKey,Station(this,evtNumber));
        }
    }

  StationList sl; sl.append(stations.values());
  return sl;
}

/**************************************************************************/
void EventsDB::diagnoseEventCorrections()
/**************************************************************************/
/*!

  \brief Diagnoses the event corrections in file
  input/data/discrete/event_corrections/EVENTS_corrected.csv.

*/
{
  const QString dataDir=idpDataInpDir+"discrete/";
  EventsDB eventsDB(dataDir+"EVENTS.csv","BODC_EVENT_NUMBER",comma);
  EventsDB eventsCorr(dataDir+"event_corrections/EVENTS_corrected.csv",
                      "BODC_EVENT_NUMBER",comma);
  QStringList sl,slC,slU,slP,corrEventNums=eventsCorr.keys();
  int i,n=corrEventNums.size(),idxDiff; InfoItem ii,iiC;
  int idxBotDep=eventsDB.columnIndexOf("BOTTOM DEPTH [M]");
  EventInfo ei,eiC; double posTol=0.01; QString orig,corr,sDiff;

  slC << eventsCorr.columnLabels.join(",");
  for (i=0; i<n; ++i)
    {
      ii=eventsDB.value(corrEventNums.at(i));
      iiC=eventsCorr.value(corrEventNums.at(i));
      ei=eventsDB.eventInfoOf(corrEventNums.at(i));
      eiC=eventsCorr.eventInfoOf(corrEventNums.at(i));
      //if (iiC.at(idxBotDep).isEmpty() && !ii.at(idxBotDep).isEmpty())
      iiC[idxBotDep]=ii.at(idxBotDep);
      orig=ii.join(","); corr=iiC.join(",");

      if (orig!=corr)
        {
          idxDiff=indexOfFirstDiff(orig,corr);
          if (idxDiff>-1) idxDiff+=6;
          sDiff=firstDiffIndicatorStr(idxDiff);

          sl << QString("orig: %1").arg(orig);
          sl << QString("corr: %1").arg(corr);
          sl << sDiff;
          sl << QString();

          if (fabs(eiC.longitude-ei.longitude)>posTol ||
              fabs(eiC.latitude-ei.latitude)>posTol)
            {
              slP << QString("orig: %1").arg(orig);
              slP << QString("corr: %1").arg(corr);
              slP << sDiff;
              slP << QString();
            }

        slC << corr;
        }
      else
        {
          slU << corr;
        }

    }

  const QString diagnDir=idpDiagnDir+"events/"; QDir().mkpath(diagnDir);
  appendRecords(diagnDir+"EVENTS_corrections.txt",sl,true);
  appendRecords(diagnDir+"EVENTS_no-change.txt",slU,true);
  appendRecords(diagnDir+"EVENTS_pos-change.txt",slP,true);
  appendRecords(diagnDir+"EVENTS_corrected.csv",slC,true);
}

/**************************************************************************/
EventInfo EventsDB::eventInfoOf(const InfoItem& ii)
/**************************************************************************/
/*!

  \brief \return The EventInfo object for event info item \a ii.

*/
{
  EventInfo ei;

  if (!ii.isEmpty())
    {
      ei.eventNumber=extractedInt(ii.at(idxEventNumber));
      ei.cruiseLbl=ii.at(idxCruise);
      ei.stationLbl=ii.at(idxStation);
      ei.castIdentifier=ii.at(idxCastIdentifier);
      ei.samplingDevice=ii.at(idxSamplingDevice);
      ei.startGregorianDay=gregorianDay(ii.at(idxStartTimeDate));
      ei.endGregorianDay=gregorianDay(ii.at(idxEndTimeDate));
      ei.startLongitude=extractedDouble(ii.at(idxStartLongitude));
      ei.startLatitude=extractedDouble(ii.at(idxStartLatitude));
      ei.endLongitude=extractedDouble(ii.at(idxEndLongitude));
      ei.endLatitude=extractedDouble(ii.at(idxEndLatitude));
      ei.longitude=extractedDouble(ii.at(idxLongitude));
      ei.latitude=extractedDouble(ii.at(idxLatitude));
      ei.bottomDepth=extractedDouble(ii.at(idxBottomDepth));

      if (ei.startLongitude!=ODV::missDOUBLE && ei.endLongitude!=ODV::missDOUBLE)
        ei.longitude=0.5*(ei.startLongitude+ei.endLongitude);
      if (ei.startLatitude!=ODV::missDOUBLE && ei.endLatitude!=ODV::missDOUBLE)
        ei.latitude=0.5*(ei.startLatitude+ei.endLatitude);
    }

  return ei;
}

/**************************************************************************/
EventInfo EventsDB::eventInfoOf(const QString& eventNumberStr)
/**************************************************************************/
/*!

  \brief \return The EventInfo object for BODC event number \a eventNumberStr.

*/
{
  return eventInfoOf(value(eventNumberStr));
}

/**************************************************************************/
double EventsDB::gregorianDay(const QString& dateTimeStr)
/**************************************************************************/
/*!

  \brief \return The decimal Gregorian day for date/time string \a
  dateTimeStr.

*/
{
  int year,month,day,hour,minute; double sec;
  double gd=ODV::missDOUBLE; QStringList sl=dateTimeStr.split(" ");

  if (convertDate(qPrintable(sl.at(0)),CNV_DATE_DDMMYYYY1,NULL,
      year,month,day,hour,minute,sec))
    {
      gd=::gregorianDay(year,month,day);
      if (sl.size()>1 &&
          convertTime(qPrintable(sl.at(1)),CNV_TIME_HHMM1,NULL,hour,minute,sec))
        gd+=decimalDay(hour,minute,sec);
    }

  return gd;
}

/**************************************************************************/
QStringList EventsDB::spreadsheetHeader()
/**************************************************************************/
/*!

  \brief \return The ODV spreadsheet file header records.

  \sa Station::spreadsheetRecords()

*/
{
  QStringList sl,colLbls=prependedColumnLabels("_"); int i,n=columnLabels.size();

  /* write //<MetaVariable> and //<DataVariable> lines */
  sl << fmtMvDef.arg("Station").arg("METASTATION")
    .arg("INDEXED_TEXT").arg("0").arg("");
  sl << fmtMvDef.arg("StDev Time [days]").arg("METABASIC")
    .arg("FLOAT").arg("4").arg("");
  sl << fmtMvDef.arg("StDev Distance [km]").arg("METABASIC")
    .arg("FLOAT").arg("4").arg("");
  sl << fmtMvDef.arg("StDev Longitude [degrees]").arg("METABASIC")
    .arg("FLOAT").arg("4").arg("");
  sl << fmtMvDef.arg("StDev Latitude [degrees]").arg("METABASIC")
    .arg("FLOAT").arg("4").arg("");
  sl << fmtMvDef.arg("StDev Bot. Depth [m]").arg("METABASIC")
    .arg("FLOAT").arg("4").arg("");
  sl << fmtDvDef.arg("BODC_EVENT_NUMBER")
    .arg("INTEGER").arg("0").arg("T").arg("").arg("");
  for (i=0; i<n; ++i)
    sl << fmtDvDef.arg(colLbls.at(i))
      .arg("INDEXED_TEXT").arg("0").arg("F").arg("").arg("");

  /* write header line */
  sl << "";
  sl << "Cruise\tStation\tType\tyyyy-mm-ddThh:mm:ss.sss\tLongitude [degrees_east]\tLatitude [degrees_north]\tBot. Depth [m]\tStDev Time [days]\tStDev Distance [km]\tStDev Longitude [degrees]\tStDev Latitude [degrees]\tStDev Bot. Depth [m]\tBODC_EVENT_NUMBER\t*StDev Time [days]\t*StDev Distance [km]\t*StDev Longitude [degrees]\t*StDev Latitude [degrees]\t*StDev Bot. Depth [m]\t"+colLbls.join("\t");

  return sl;
}

/**************************************************************************/
QStringList EventsDB::uniqueValuesFor(const QStringList& eventNumbers,int idx)
/**************************************************************************/
/*!

  \brief \return The list of unique values in column index \a idx for
  event numbers \a eventNumbers.

*/
{
  int i,n=eventNumbers.size(); QMap<QString,int> vals;
  for (i=0; i<n; ++i)
    vals.insert(value(eventNumbers.at(i)).at(idx),1);
  return vals.keys();
}


