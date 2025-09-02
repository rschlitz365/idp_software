#ifndef DATA_H
#define DATA_H

/****************************************************************************
**
** Copyright (C) 2025 Reiner Schlitzer. All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <QList>
#include <QMap>
#include <QString>
#include <QStringList>

#include "globalDefines.h"
#include "InfoMap.h"

class CruisesDB;
class DataItemsDB;
class DatasetInfos;
class EventsDB;
class ParamSet;

/**************************************************************************/
class DataItem
/**************************************************************************/
/*!

  \brief Container holding the information of one data item.

*/
{
public:
  DataItem() { eventNumber=-1; }
  DataItem(DataItemsDB *dataItemDB,const QString& line,QChar splitChar);

  int paramId(ParamSet *paramSet);
  QString toString(QChar sepChar=',') const;

  int eventNumber;
  int bodcBottleNumber;
  int rosetteBottleNumber;
  int subSampleNumber;
  QString geotracesSampleId;
  QString cellSampleId;
  char bodcBottleFlag;
  double depth;
  double pressure;
  QString parameter;
  double parameterValue;
  double standardDevValue;
  char flag;
  QString unit;
};

/**************************************************************************/
class DataItemsDB : public QList<DataItem>
/**************************************************************************/
/*!

  \brief Container holding all data items.

*/
{
public:
  DataItemsDB(const QString& fn,QChar splitChar,
              DatasetInfos *datasetInfos,EventsDB *eventsDB);
              void appendFile(const QString& fn,QChar splitChar);
  void aggregateSubSamples();
  void appendItems(const QStringList& lines,QChar splitChar);
  static QStringList columnLabelsFromHeader(const QString& headerLine,QChar splitChar);
  QList<int> dataItemIndexes(const QString& sampleKey);
  void writeDiagnostics(CruisesDB *cruisesDBPtr);

  int idxEventNumber,idxBottleNumber,idxRosetteBottleNumber,idxBottleFlag;
  int idxcellSampleId,idxSubSampleId,idxGeotracesSampleId,idxDepth,idxPressure;
  int idxParameter,idxParameterValue,idxParameterStDev,idxFlag,idxUnits;

  DatasetInfos *datasetInfosPtr; //!< pointer to DOoR dataset infos
  EventsDB *eventsDBPtr;         //!< pointer to events database
  QStringList columnLabels;      //!< column header labels
  QMap<QString,int> multiSubSampleItems; //!< BODC_BOTTLE_NUMBER/PARAMETER with sub-sample Id > 1

  QMap<QString,QString> acceptedCruises;
  //!< accepted GEOTRACES IDs (value) by cruise names (keys)
  QMap<QString,int> acceptedPrmNames; //!< accepted parameter names
  QMap<QString,int> acceptedExtPrmNames; //!< accepted extended parameter names
  QMap<QString,int> errMsgs; //!< error messages
};

/**************************************************************************/
class DataItemList
/**************************************************************************/
/*!

  \brief Container holding the approved data records obtained from
  BODC for a given data type.

*/
{
public:
  DataItemList(IdpDataType dataType,
               DataItemsDB *dataItemsDB,DatasetInfos *datasetInfos);
  void buildIndexListsByEventNumber();
  bool hasDataFor(const QString& prmName)
  { return dataItemsDBPtr->acceptedPrmNames.contains(prmName); }
  DataItem itemAt(int idx);
  void validateUnits(ParamSet *paramSet);

  IdpDataType type;              //!< data type
  DataItemsDB *dataItemsDBPtr;   //!< pointer to parent DataItemsDB
  DatasetInfos *datasetInfosPtr; //!< pointer to DOoR dataset infos

  QList<int> idxIntoDataItemDB;
  QMap<int,QList<int> > dataIdxsByEvent;
  //!< data index values (into dataItemsDBPtr) by event number (key)
  QMap<QString,QString> acceptedCruises;
  //!< accepted GEOTRACES IDs (value) by cruise names (keys) for this data type
  QMap<QString,int> acceptedEventNumbers;
  //!< accepted event numbers for this data type
  QMap<QString,int> acceptedPrmNames;
  //!< accepted parameter names for this data type
  QMap<QString,int> acceptedExtPrmNames;
  //!< accepted extended parameter names for this data type
};


#endif   // DATA_H
