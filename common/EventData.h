#ifndef EVENTDATA_H
#define EVENTDATA_H

/****************************************************************************
**
** Copyright (C) 2025 Reiner Schlitzer. All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <QMap>
#include <QString>
#include <QStringList>

#include "globalDefines.h"
#include "Events.h"
#include "InfoMap.h"
#include "RMemArea.h"

class CruisesDB;
class DataItemList;
class DatasetInfos;
class ParamSet;
class Station;
class UnitConverter;


/**************************************************************************/
class EventData
/**************************************************************************/
/*!

  \brief Container holding the data of one event.

*/
{
public:
  EventData(Station *station,int eventIdx,
            DatasetInfos *datasetInfos,CruisesDB *cruises,
            ParamSet *paramSet,DataItemList *dataItemList,
            InfoMap *docuByExtPrmName,InfoMap *bioGeotracesInfos,
            InfoMap *piInfosByName,UnitConverter *unitConverter,
            QMap<char,QString> *bottleFlagDescr,
            const QString& infoFileDir);

  int dataIdFromExtendedName(const QString& extPrmName,bool hasUnifiedPrms);
  int firstSampleId(int bodcBottleNumber);
  void getValues(const QString& uPrmName,int smplIdx,
                 double &val,double &err,char &qf,QString &infoStr);
  QString infoFileName(const QString& prmName,const QList<int> idxList);
  QString metaValueString(bool inclMetaValues);
  QString methodsIdFromUrl(const QString& methodsUrl);
  QString piNameFromExtName(const QString& extPiName,QString *emailAddress=NULL);
  QStringList paramNamesForUPrmName(const QString& uPrmName,QStringList& barcodes);
  int sampleCount(int bodcBottleNumber);
  int sampleId(int bodcBottleNumber,const QString& cellSampleId);
  QString spreadsheetDataPart(int bodcBottleNumber,int cellSampleIdx);
  QStringList spreadsheetDataRecords(int bodcBottleNumber,bool inclMetaValues);
  QStringList spreadsheetDataLines();
  static QStringList spreadsheetHeaderLines(ParamSet *paramSet,
                                            InfoMap *keyVarsByDataVar);
  void writeInfoFile(const QString& fn,const QString& prmName,
                     const QList<int> idxList,const QStringList& barcodes);

  Station *stationPtr; //!< pointer to parent station
  EventInfo eventInfo; //!< event info object
  CruisesDB *cruisesPtr; //!< pointer to CRUISES set
  ParamSet *paramSetPtr; //!< pointer to parameter set for the data type
  DataItemList *dataItemListPtr; //!< pointer to data item set for the data type
  DatasetInfos *datasetInfosPtr; //!< pointer to data set infos object
  InfoMap *docuByExtPrmNamePtr; //!< pointer to data documentation object
  InfoMap *bioGeotracesInfosPtr; //!< pointer to bioGEOTRACES information
  InfoMap *piInfosByNamePtr; //!< pointer to PI information
  UnitConverter *unitConvPtr; //!< pointer to unit converter
  QMap<char,QString> *bottleFlagDescrPtr; //!<
  //!< pointer to bottle flag description dictionary
  QString infoDir; //!< directory for info files
  bool unifiedPrms; //!< Flag indicating whether parameters are unified or not

  /* storage for numeric and string data variables */
  QList<int> bodcBottleNumbers; //!< list of BODC bottle numbers for this event
  QList<int> firstSmplIds; //!< list of BODC bottle numbers for this event
  QMap<int,QStringList> cellSampleIdsByBodcBottleNumber;
  //!< cellSampleIds by BODC bottle number for this event
  QMap<QString,QStringList> barcodesByPrmName;
  //!< barcode suffixes by full parameter name for this event
  QMap<QString,QList<int> > dataIdsByUPrmName;
  //!< data ids by (unified) parameter name for this event
  QMap<QString,QStringList> extPrmNamesByUPrmName;
  //!< extended parameter names by (unified) parameter name for this event
  QList<char> bodcBottleFlags; //!< list of BODC bottle flags for this event
  QList<int> rosetteBottleNumbers; //!< list of BODC bottle numbers for this event
  QStringList geotracesSampleIds; //!< list of GEOTRACES sample ids for this event
  RMemArea dblData; //!< Storage for values of numeric data variables
  RMemArea errData; //!< Storage for 1-sigma values of numeric data variables
  RMemArea qfData; //!< Storage for quality flags of numeric data variables

  int pressureID,depthID;
};


#endif   // EVENTDATA_H
