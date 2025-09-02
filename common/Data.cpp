/****************************************************************************
**
** Copyright (C) 2025 Reiner Schlitzer. All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "Data.h"

#include <QDir>

#include "globalVars.h"
#include "globalFunctions.h"
#include "Cruises.h"
#include "Datasets.h"
#include "Events.h"
#include "Params.h"
#include "RRandomVar.h"

#include "common/odv.h"


/**************************************************************************/
DataItem::DataItem(DataItemsDB *dataItemDB,const QString& line,QChar splitChar)
/**************************************************************************/
/*!

  \brief Creates a DataItem object and retrieves its values from
  string \a line.

*/
{
  QStringList sl=line.split(splitChar); int i,columnCount=sl.size();
  for (i=0; i<columnCount; ++i)
    stripEnclosingChars(sl[i],'"','"');

  eventNumber=extractedInt(sl.at(dataItemDB->idxEventNumber));
  bodcBottleNumber=extractedInt(sl.at(dataItemDB->idxBottleNumber));
  rosetteBottleNumber=extractedInt(sl.at(dataItemDB->idxRosetteBottleNumber));
  subSampleNumber=extractedInt(sl.at(dataItemDB->idxSubSampleId));
  cellSampleId=sl.at(dataItemDB->idxcellSampleId);
  geotracesSampleId=sl.at(dataItemDB->idxGeotracesSampleId);
  bodcBottleFlag=sl.at(dataItemDB->idxBottleFlag).at(0).toLatin1();
  depth=extractedDouble(sl.at(dataItemDB->idxDepth));
  pressure=extractedDouble(sl.at(dataItemDB->idxPressure));
  parameter=sl.at(dataItemDB->idxParameter);
  parameterValue=extractedDouble(sl.at(dataItemDB->idxParameterValue));
  standardDevValue=extractedDouble(sl.at(dataItemDB->idxParameterStDev));
  flag=sl.at(dataItemDB->idxFlag).at(0).toLatin1();
  unit=sl.at(dataItemDB->idxUnits);

  double dmy;
  if (rosetteBottleNumber==ODV::missINT32)
    dmy=1.0;
  if (depth==ODV::missDOUBLE && pressure==ODV::missDOUBLE)
    dmy=1.0;

  if (depth==ODV::missDOUBLE && pressure!=ODV::missDOUBLE)
    depth=calDepthEOS80(pressure,0.);
  if (pressure==ODV::missDOUBLE && depth!=ODV::missDOUBLE)
    pressure=calPressEOS80(depth,0.);
}

/**************************************************************************/
int DataItem::paramId(ParamSet *paramSet)
/**************************************************************************/
/*!

  \brief \return The parameter ID of this DataItem object as in \a
  paramSet, or \c -1 if this is not found.

*/
{
  return paramSet->paramIdFor(Param::paramNameFromExtendedName(parameter));
}

/**************************************************************************/
QString DataItem::toString(QChar sepChar) const
/**************************************************************************/
/*!

  \brief \return The The values of this data item as concatenated string.

*/
{
  QString rbn=(rosetteBottleNumber==ODV::missINT32) ?
    QString() : QString::number(rosetteBottleNumber);
  QString std=(standardDevValue==ODV::missDOUBLE) ?
    QString() : QString::number(standardDevValue);
  return QString::number(eventNumber)+sepChar
    +QString::number(bodcBottleNumber)+sepChar
    +rbn+sepChar
    +QString(bodcBottleFlag)+sepChar
    +geotracesSampleId+sepChar
    +QString::number(depth)+sepChar
    +QString::number(pressure)+sepChar
    +cellSampleId+sepChar
    +QString::number(subSampleNumber)+sepChar
    +parameter+sepChar
    +QString::number(parameterValue)+sepChar
    +std+sepChar
    +QString(flag)+sepChar
    +unit+sepChar;
}



/**************************************************************************/
DataItemsDB::DataItemsDB(const QString& fn,QChar splitChar,
                         DatasetInfos *datasetInfos,EventsDB *eventsDB)
  : datasetInfosPtr(datasetInfos),eventsDBPtr(eventsDB)
/**************************************************************************/
/*!

  \brief Creates a DataItemsDB object and loads the data items from
  file \a fn.

  Only data items with PI permission and S&I approval and not removed
  are kept.

*/
{
  QStringList sl=fileContents(fn),vals; DataItem di;
  columnLabels=columnLabelsFromHeader(sl.at(0),splitChar);

  idxEventNumber=columnLabels.indexOf("BODC_EVENT_NUMBER");
  idxBottleNumber=columnLabels.indexOf("BODC_BOTTLE_NUMBER");
  idxRosetteBottleNumber=columnLabels.indexOf("ROSETTE_BOTTLE_NUMBER");
  idxBottleFlag=columnLabels.indexOf("BODC_BOTTLE_FLAG");
  idxcellSampleId=columnLabels.indexOf("SAMPLE_CELL_ID");
  idxSubSampleId=columnLabels.indexOf("SUB_SAMPLE_NUMBER");
  idxGeotracesSampleId=columnLabels.indexOf("GEOTRACES_SAMPLE_ID");
  idxDepth=columnLabels.indexOf("DEPTH");
  idxPressure=columnLabels.indexOf("PRESSURE");
  idxParameter=columnLabels.indexOf("PARAMETER");
  idxParameterValue=columnLabels.indexOf("PARAMETER_VALUE");
  idxParameterStDev=columnLabels.indexOf("1SD::PARAMETER_VALUE");
  idxFlag=columnLabels.indexOf("FLAG");
  idxUnits=columnLabels.indexOf("UNIT");

  appendItems(sl,splitChar);
}

/**************************************************************************/
void DataItemsDB::aggregateSubSamples()
/**************************************************************************/
/*!

  \brief Finds all subsample values for given BODC bottle number and
  parameter and replaces the value of subsample 1 with the median of all
  values.

  The quality flag of the median value is obtained as poorest flag among
  the contributing values

  The data items of all other subsamples (2 or higher) are removed from
  this DataItemsDB.

*/
{
  /* create multi_subsample_data_lines summary */
  QStringList subSampleKeys=multiSubSampleItems.keys();
  int i,j,m=subSampleKeys.size(),valueCount,idx0; QList<int> idxs;
  QVector<double> vals; QList<char> qfs; DataItem di0;
  for (i=0; i<m; ++i)
    {
      idxs=dataItemIndexes(subSampleKeys.at(i));
      if (idxs.size()>1)
        {
          idx0=idxs.at(0); di0=at(idx0);

          /* obtain median value and poorest quality flag */
          vals.clear(); qfs.clear();
          for (j=0; j<idxs.size(); ++j)
            {
              vals.append(at(idxs.at(j)).parameterValue);
              qfs.append(at(idxs.at(j)).flag);
            }

          /* obtain median value and poorest quality flag */
          valueCount=vals.size();
          if (valueCount>1)
            {
              RRandomVar rv(valueCount,vals.data(),ODV::missDOUBLE);
              di0.parameterValue=rv.median();
              di0.standardDevValue=ODV::missDOUBLE;
              di0.flag=combinedSdnQualityFlag(qfs);
            }

          /* update data item at idx0 */
          replace(idx0,di0);

          /* remove the other subsample data items, reverse order */
          for (j=(idxs.size()-1); j>0; --j)
            removeAt(idxs.at(j));
        }
    }

    /* clear the subsample list */
    subSampleKeys.clear();
}

/**************************************************************************/
void DataItemsDB::appendFile(const QString& fn,QChar splitChar)
/**************************************************************************/
/*!

  \brief Loads the data items from file \a fn and appends these to
  this object.

  Only data items with PI permission and S&I approval and not removed
  are kept.

*/
{
  QStringList sl=fileContents(fn),vals; DataItem di;
  if (columnLabels!=columnLabelsFromHeader(sl.at(0),splitChar)) return;

  appendItems(sl,splitChar);
}

/**************************************************************************/
void DataItemsDB::appendItems(const QStringList& lines,QChar splitChar)
/**************************************************************************/
/*!

  \brief Extracts the data items from string list \a lines and appends
  these to this object.

  Only data items with PI permission and S&I approval and not removed
  are kept.

*/
{
  int i,lineCount=lines.size(); bool isApproved,isRemoved; DataItem di;
  QString prmName,extPrmName,cruise,cruiseFromEvents,geotracesCruise,s;
  InfoItem datasetInfoItem,eventInfoItem;
  for (i=1; i<lineCount; ++i)
    {
      di=DataItem(this,lines.at(i),splitChar);
      extPrmName=di.parameter;
      eventInfoItem=eventsDBPtr->value(QString::number(di.eventNumber));
      datasetInfoItem=datasetInfosPtr->value(extPrmName);

      if (datasetInfoItem.isEmpty())
        {
          errMsgs.insert(QString("DataItemsDB::Dataset not found %1").arg(extPrmName),1);
          continue;
        }

      cruise=datasetInfoItem.at(datasetInfosPtr->idxCruise);
      cruiseFromEvents=eventInfoItem.at(0);
      geotracesCruise=datasetInfosPtr->geotracesCruiseNameFor(cruise);
      prmName=Param::paramNameFromExtendedName(extPrmName);
      isApproved=datasetInfosPtr->hasApprovalsForExtendedParamName(extPrmName);
      isRemoved=datasetInfosPtr->isRemovedDataset(cruise,prmName);

      if (di.subSampleNumber>1)
        multiSubSampleItems.insert(QString("%1\t%2").arg(di.bodcBottleNumber).arg(di.parameter),1);

      if (cruise!=cruiseFromEvents)
        {
          errMsgs.insert(QString("DataItemsDB::CruiseMismatch(%1,%2) event#: %3 %4")
            .arg(cruise).arg(cruiseFromEvents)
            .arg(di.eventNumber).arg(di.parameter),1);
        }

      /* skip if not approved or removed */
      if (!isApproved || isRemoved) continue;

      append(di);
      acceptedCruises.insert(cruise,geotracesCruise);
      acceptedPrmNames.insert(prmName,1);
      acceptedExtPrmNames.insert(extPrmName,1);
    }
}

/**************************************************************************/
QStringList DataItemsDB::columnLabelsFromHeader(const QString& headerLine,
                                                QChar splitChar)
/**************************************************************************/
/*!

  \brief Extracts the column labels from the header line \a headerLine.

  \return The extracted header line.

*/
{
  QStringList sl=headerLine.split(splitChar); int i,columnCount=sl.size();
  for (i=0; i<columnCount; ++i)
    { stripEnclosingChars(sl[i],'"','"'); }
  return sl;
}

/**************************************************************************/
QList<int> DataItemsDB::dataItemIndexes(const QString& sampleKey)
/**************************************************************************/
/*!

\brief Determines the data item indexes matching the sample key
\a sampleKey.

Sample keys consist of the BODC_BOTTLE_NUMBER and PARAMETER values
separated by TAB.

\return The determined index list.

*/
{
  QStringList sl=sampleKey.split("\t"); QString prm=sl.at(1); DataItem di;
  QList<int> idxs; int i,n=size(),bn=sl.at(0).toInt();

  for (i=0; i<n; ++i)
    {
      di=at(i);
      if (di.bodcBottleNumber==bn && di.parameter==prm)  idxs << i;
    }

  return idxs;
}

/**************************************************************************/
void DataItemsDB::writeDiagnostics(CruisesDB *cruisesDBPtr)
/**************************************************************************/
/*!

  \brief Write diagnostics information to files.

*/
{
  const QString fmt="%1\t%2\t%3\t%4\t%5\t%6 - %7\t%8";
  const QString dir=idpOutputDir+"data/"; QDir().mkpath(dir);

  appendRecords(dir+"DataItemsDB_error_messages.txt",errMsgs.keys(),true);
  appendRecords(dir+"DataItemsDB_accepted_cruises.txt",
                acceptedCruises.keys(),true);
  appendRecords(dir+"DataItemsDB_accepted_parameters.txt",
                acceptedPrmNames.keys(),true);
  appendRecords(dir+"DataItemsDB_accepted_extended_parameters.txt",
                acceptedExtPrmNames.keys(),true);

  QStringList sl;

  /* create multi_subsample_data_lines summary */
  QStringList subSampleKeys=multiSubSampleItems.keys();
  int i,j,m=subSampleKeys.size(); QList<int> idxs;
  for (i=0; i<m; ++i)
    {
      idxs=dataItemIndexes(subSampleKeys.at(i));
      if (idxs.size()>1)
        {
          for (j=0; j<idxs.size(); ++j)
            sl << at(idxs.at(j)).toString();
          sl << QString(" ");
        }
    }
  appendRecords(dir+"DataItemsDB_multi_subsample_data_lines.txt",sl,true);

  /* create accepted_cruises summary */
  QString cruise,gtCruise,s; QMultiMap<QString,QString> crInfos;
  QMap<QString,QString>::ConstIterator it; InfoItem cr;
  for (it=acceptedCruises.constBegin(); it!=acceptedCruises.constEnd(); ++it)
    {
      cruise=it.key(); gtCruise=it.value(); cr=cruisesDBPtr->value(cruise);
      if (gtCruise.isEmpty() || cr.isEmpty()) continue;

      s=QString("%1\t%2\t%3\t%4\t%5\t%6 - %7\t%8").arg(gtCruise).arg(cruise)
        .arg(cr.at(cruisesDBPtr->idxAliases)).arg(cr.at(cruisesDBPtr->idxShipName))
        .arg(cr.at(cruisesDBPtr->idxGeotracesPi))
        .arg(cr.at(cruisesDBPtr->idxStartTimeDate).left(10))
        .arg(cr.at(cruisesDBPtr->idxEndTimeDate).left(10))
        .arg(cr.at(cruisesDBPtr->idxCountry));
      crInfos.insert(gtCruise,s);
    }
  sl.clear();
  sl << "Section\tCruise\tCruise Alias\tShip\tGEOTRACES Scientist\tPeriod\tCountry";
  sl << QString();
  QMultiMap<QString,QString>::ConstIterator itm;
  for (itm=crInfos.constBegin(); itm!=crInfos.constEnd(); ++itm)
    { sl << itm.value(); }
  appendRecords(dir+"DataItemsDB_accepted_cruises_full.txt",sl,true);
}



/**************************************************************************/
DataItemList::DataItemList(IdpDataType dataType,
                           DataItemsDB *dataItemsDB,DatasetInfos *datasetInfos)
  : type(dataType),dataItemsDBPtr(dataItemsDB),datasetInfosPtr(datasetInfos)
/**************************************************************************/
/*!

  \brief Creates a DataItemList object for data type \a dataType.

*/
{
  QList<int> idxs; int i,k,dataItemCount=dataItemsDBPtr->size();
  QString prmName,extPrmName,cruise,geotracesCruise;
  DataItem di; InfoItem datasetInfoItem; IdpDataType dType;

  for (i=0; i<dataItemCount; ++i)
    {
      di=dataItemsDBPtr->at(i);
      prmName=Param::paramNameFromExtendedName(di.parameter);
      dType=Param::dataType(prmName);

      /* skip if wrong data type */
      if (dType!=type)
        {
          if (dType==UnknownDT)
            k=1; // set breakpoint here to detect unknown sampling systems.
          continue;
        }

      idxIntoDataItemDB.append(i);
      acceptedEventNumbers.insert(QString::number(di.eventNumber),1);
      acceptedPrmNames.insert(prmName,1);
      acceptedExtPrmNames.insert(di.parameter,1);
    }
  buildIndexListsByEventNumber();
}

/**************************************************************************/
void DataItemList::buildIndexListsByEventNumber()
/**************************************************************************/
/*!

  \brief Builds the index map \a dataIdxsByEvent.

*/
{
  dataIdxsByEvent.clear();

  int i,idx,n=idxIntoDataItemDB.size(),evtNum; QList<int> idxs;
  for (i=0; i<n; ++i)
    {
      idx=idxIntoDataItemDB.at(i);
      evtNum=itemAt(idx).eventNumber;
      if (dataIdxsByEvent.contains(evtNum))
        {
          idxs=dataIdxsByEvent.value(evtNum);
          idxs.append(idx); dataIdxsByEvent.insert(evtNum,idxs);
        }
      else
        {
          idxs.clear();
          idxs.append(idx); dataIdxsByEvent.insert(evtNum,idxs);
        }
    }
}

/**************************************************************************/
DataItem DataItemList::itemAt(int idx)
/**************************************************************************/
/*!

  \brief Retrieves the DataItem object at index \a idx.

  \return The retrieved DataItem object.

*/
{
  return dataItemsDBPtr->at(idx);
}

/**************************************************************************/
void DataItemList::validateUnits(ParamSet *paramSet)
/**************************************************************************/
/*!

  \brief Validate units in the data items against units of parameters
  in \a paramSet.

*/
{
  const QString fmt="Bad units: %1 [%2] should be [%3]";
  QMap<QString,int> bu; int i,idx,n=idxIntoDataItemDB.size();
  QString prmUnits,trgUnits; DataItem di;
  for (i=0; i<n; ++i)
    {
      idx=idxIntoDataItemDB.at(i);
      di=dataItemsDBPtr->at(idx); prmUnits=di.unit;
      if (prmUnits=="dimensionless") prmUnits=QString();
      trgUnits=paramSet->paramUnitsOf(Param::paramNameFromExtendedName(di.parameter));
      if (trgUnits=="unknown_units") continue;
      if (trgUnits!=prmUnits)
        bu.insert(fmt.arg(di.parameter).arg(di.unit).arg(trgUnits),1);
    }

  QString dir=idpDiagnosticsDir+"data/"; QDir().mkpath(dir);
  QString fn=QString("BadUnits_%1.txt")
    .arg(ParamSet::dataTypeNameFromType(type));
  appendRecords(dir+fn,bu.keys(),true);
}


