/****************************************************************************
**
** Copyright (C) 2025 Reiner Schlitzer. All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "EventData.h"

#include <QDir>
#include <QFile>
#include <QMap>
#include <QTextStream>
#include <QVector>

#include "globalVars.h"
#include "globalFunctions.h"
#include "Cruises.h"
#include "Params.h"
#include "RRandomVar.h"
#include "UnitConverter.h"

#include "common/odv.h"
#include "common/odvDate.h"
#include "common/RDateTime.h"


/**************************************************************************/
EventData::EventData(Station *station,int eventIdx,
                     DatasetInfos *datasetInfos,CruisesDB *cruises,
                     ParamSet *paramSet,DataItemList *dataItemList,
                     InfoMap *docuByExtPrmName,InfoMap *bioGeotracesInfos,
                     InfoMap *piInfosByName,UnitConverter *unitConverter,
                     QMap<char,QString> *bottleFlagDescr,
                     const QString& infoFileDir)
  : stationPtr(station),datasetInfosPtr(datasetInfos),cruisesPtr(cruises),
    paramSetPtr(paramSet),dataItemListPtr(dataItemList),
    docuByExtPrmNamePtr(docuByExtPrmName),bioGeotracesInfosPtr(bioGeotracesInfos),
    unitConvPtr(unitConverter),bottleFlagDescrPtr(bottleFlagDescr),
    piInfosByNamePtr(piInfosByName),infoDir(infoFileDir),
    pressureID(-2),depthID(-1),unifiedPrms(paramSet->hasUnifiedPrms())
/**************************************************************************/
/*!

  \brief Creates a EventData object and collates the data of event
  index \a eventIdx of station \a station.

  A data id is the id used for retrieving data, errors, and quality flags
  for a given parameter from the RMemArea objects dblData, errData, and qfData.
*/
{
  StationInfo si(*stationPtr);
  eventInfo=stationPtr->eventInfoAt(eventIdx);

  /* construct list of bottle numbers for this event */
  QList<int> dataIdxs=dataItemListPtr->dataIdxsByEvent.value(eventInfo.eventNumber);
  int i,n,bodcBottleNumber,cellCount,smplCount,dataItemCount=dataIdxs.size();
  DataItem di; QString prmName,uPrmName,extPrmName,barcode,cellSampleId,ssSuffix;
  QStringList barcodes,extPrmNames,cellSampleIds; QList<int> dataIds;
  int nextDataId=-1;
  for (i=0; i<dataItemCount; ++i)
    {
      di=dataItemListPtr->itemAt(dataIdxs.at(i));
      bodcBottleNumber=di.bodcBottleNumber;
      if (!bodcBottleNumbers.contains(bodcBottleNumber))
        {
          bodcBottleNumbers.append(bodcBottleNumber);
          bodcBottleFlags.append(di.bodcBottleFlag);
          rosetteBottleNumbers.append(di.rosetteBottleNumber);
          geotracesSampleIds.append(di.geotracesSampleId);
        }

      cellSampleId=di.cellSampleId;
      cellSampleIds=cellSampleIdsByBodcBottleNumber.value(bodcBottleNumber);
      if (!cellSampleId.isEmpty() && !cellSampleIds.contains(cellSampleId))
        {
          cellSampleIds.append(cellSampleId);
          cellSampleIdsByBodcBottleNumber.insert(bodcBottleNumber,cellSampleIds);
        }

      extPrmName=di.parameter;
      prmName=Param::paramNameFromExtendedName(extPrmName,&barcode);
      uPrmName=(paramSet->hasUnifiedPrms()) ?
        Param::unifiedNameLabel(prmName,ssSuffix) : prmName;

      barcodes=barcodesByPrmName.value(prmName);
      dataIds=dataIdsByUPrmName.value(uPrmName);
      extPrmNames=extPrmNamesByUPrmName.value(uPrmName);
      if (!barcodes.contains(barcode))
      // if (!extPrmNames.contains(extPrmName))
        {
          barcodes.append(barcode);
          barcodesByPrmName.insert(prmName,barcodes);
          dataIds.append(++nextDataId);
          dataIdsByUPrmName.insert(uPrmName,dataIds);
          extPrmNames.append(extPrmName);
          extPrmNamesByUPrmName.insert(uPrmName,extPrmNames);
        }
    }

  /* loop over all bottle numbers in this event and determine the sample count */
  n=bodcBottleNumbers.size(); smplCount=0;
  for (i=0; i<n; ++i)
    {
      firstSmplIds.append(smplCount);
      bodcBottleNumber=bodcBottleNumbers.at(i);
      cellCount=cellSampleIdsByBodcBottleNumber.value(bodcBottleNumber).size();
      smplCount+=(cellCount==0) ? 1 : cellCount;
    }

  /* allocate workspace for all data variables: values, error values
    and quality flags. initialize the arrays */
  dblData.requestMulti(pressureID,nextDataId,smplCount*sizeof(double));
  errData.requestMulti(pressureID,nextDataId,smplCount*sizeof(double));
  qfData.requestMulti(pressureID,nextDataId,smplCount*sizeof(char));
  for (i=pressureID; i<=nextDataId; ++i)
    {
      initArray<double>((double*)dblData.data(i),smplCount,ODV::missDOUBLE);
      initArray<double>((double*)errData.data(i),smplCount,ODV::missDOUBLE);
      initArray<quint8>((quint8*)qfData.data(i),smplCount,'0');
    }

  /* loop over all data items. extract value, error value and quality flag */
  int smplId,dataId; double pressVal,depthVal;
  double *pressure=(double*) dblData.data(pressureID);
  double *depth=(double*) dblData.data(depthID);
  // int dmy;
  for (i=0; i<dataItemCount; ++i)
    {
      di=dataItemListPtr->itemAt(dataIdxs.at(i));
      bodcBottleNumber=di.bodcBottleNumber;

      // if (bodcBottleNumber==1400600 && di.parameter.startsWith("Ra_226_D_CONC_BOTTLE"))
      //   dmy=1;

      smplId=sampleId(bodcBottleNumber,di.cellSampleId);
      dataId=dataIdFromExtendedName(di.parameter,paramSetPtr->hasUnifiedPrms());
      if (smplId==-1 || dataId==-1) continue;

      /* retrieve depth and pressure values. ensure both exist. */
      pressVal=di.pressure;
      depthVal=di.depth;
      if      (pressVal==ODV::missDOUBLE && depthVal!=ODV::missDOUBLE)
        pressVal=calPressEOS80(depthVal,si.meanLat);
      else if (pressVal!=ODV::missDOUBLE && depthVal==ODV::missDOUBLE)
        depthVal=calDepthEOS80(pressVal,si.meanLat);

      /* assign values */
      pressure[smplId]=pressVal;
      depth[smplId]=depthVal;
      ((double*) dblData.data(dataId))[smplId]=di.parameterValue;
      ((double*) errData.data(dataId))[smplId]=di.standardDevValue;
      ((char*) qfData.data(dataId))[smplId]=di.flag;
    }
}

/**************************************************************************/
QStringList EventData::paramNamesForUPrmName(const QString& uPrmName,
                                             QStringList& barcodes)
/**************************************************************************/
/*!

  \brief Retrieve all parameter names and barcodes associated with
  unified parameter name \a uPrmName.

  \return The retrieved parameter names and barcode list in argument
  \a barcodes.

*/
{
  QStringList extPrmNames=extPrmNamesByUPrmName.value(uPrmName),sl;
  int i,n=extPrmNames.size(); QString prmName,barcode;

  barcodes.clear();
  for (i=0; i<n; ++i)
    {
      prmName=Param::paramNameFromExtendedName(extPrmNames.at(i),&barcode);
      sl << prmName; barcodes << barcode;
    }

  return sl;
}

/**************************************************************************/
int EventData::dataIdFromExtendedName(const QString& extPrmName,
                                      bool hasUnifiedPrms)
/**************************************************************************/
/*!

  \brief Determines the data id for extended parameter name \a extPrmName.

  The data id can be used to retrieve the data storage addresses to data,
  data errors and quality flags for \a extPrmName.

  \return The retrieved data id, or \c -1 if \a extPrmName is not found.

*/
{
  QString prmName,uPrmName,barcode,ssSuffix;
  prmName=Param::paramNameFromExtendedName(extPrmName,&barcode);
  uPrmName=(hasUnifiedPrms) ?
    Param::unifiedNameLabel(prmName,ssSuffix) : prmName;
  int idx=extPrmNamesByUPrmName.value(uPrmName).indexOf(extPrmName);
  return (idx==-1) ? -1 : dataIdsByUPrmName.value(uPrmName).at(idx);
}

/**************************************************************************/
int EventData::firstSampleId(int bodcBottleNumber)
/**************************************************************************/
/*!

  \brief \return The first sample id for BODC bottle number \a
  bodcBottleNumber.

*/
{
  int bottleIdx=bodcBottleNumbers.indexOf(bodcBottleNumber);
  return (bottleIdx==-1) ? -1 : firstSmplIds.at(bottleIdx);
}

/**************************************************************************/
void EventData::getValues(const QString& uPrmName,int smplIdx,
                          double &val,double &err,char &qf,QString &infoStr)
/**************************************************************************/
/*!

  \brief Gets the values for parameter name \a uPrmName and sample
  index \a smplIdx.

  \return \c true if successful, or \c false otherwise.

*/
{
  /* initialize values */
  val=err=ODV::missDOUBLE; qf='9'; infoStr="";

  QStringList extPrmNames=extPrmNamesByUPrmName.value(uPrmName);
  QStringList barcodes,prmNames=paramNamesForUPrmName(uPrmName,barcodes);
  int i,contribCount=barcodes.size(),valueCount,dataId; QString prmName;
  if (contribCount==0) return;

  QVector<double> vals,errs; QList<char> qfs;
  QList<int> contribIdxs; double lVal,lErr; char lQf;
  QMap<QString,int> infoFiles;

  /* loop over all contributors and collect values */
  for (i=0; i<contribCount; ++i)
    {
      prmName=prmNames.at(i);
      dataId=dataIdFromExtendedName(prmName+"::"+barcodes.at(i),unifiedPrms);
      lVal=((double*) dblData.data(dataId))[smplIdx];
      lErr=((double*) errData.data(dataId))[smplIdx];
      lQf=((char*) qfData.data(dataId))[smplIdx];

      if (lVal!=ODV::missDOUBLE)
        {
          contribIdxs.append(i);
          vals.append(lVal); errs.append(lErr); qfs.append(lQf);
        }
    }

  valueCount=vals.size();
  if      (valueCount==1)
    { val=vals.at(0); err=errs.at(0); qf=qfs.at(0); }
  else if (valueCount>1)
    {
      RRandomVar rv(valueCount,vals.data(),ODV::missDOUBLE);
      val=rv.median(); err=ODV::missDOUBLE; qf=combinedSdnQualityFlag(qfs);
    }

  /* assign the info string and write info file, if necessary */
  if (valueCount>0)
    {
      QString infoFn=infoFileName(prmName,contribIdxs);
      infoStr=QString("lf:infos/%1.html").arg(infoFn);
      if (!infoFiles.contains(infoFn))
        {
          writeInfoFile(infoFn,prmName,contribIdxs);
          infoFiles.insert(infoFn,1);
        }
    }
}

/**************************************************************************/
QString EventData::infoFileName(const QString& prmName,const QList<int> idxList)
/**************************************************************************/
/*!

  \brief Constructs the info file name for parameter \a prmName and
  contribution from barcode index values \a idxList.

  \return The constructed info file name.

*/
{
  QString fn=stationPtr->cruiseLbl+"_"+prmName+"_"; int i,n=idxList.size();
  for (i=0; i<n; ++i) fn+=QString::number(idxList.at(i));
  return fn;
}

/**************************************************************************/
QString EventData::metaValueString(bool inclMetaValues)
/**************************************************************************/
/*!

  \brief Constructs the spreadsheet-style meta value line.

  The meta values are included if \a inclMetaValues is \c true,
  otherwise the meta variable fields are left empty.

  \return The constructed meta value line.

*/
{
  const QString fmt=
    "%1\t%2\t%3\t%4\t%5\t%6\t%7\t%8\t%9\t%10\t%11\t%12\t%13\t%14\t%15\t%16\t%17\t%18\t%19\t%20";
  const QString fmtPeriod="%1 - %2";

if (inclMetaValues)
  {
    // int dmy;
    // if (stationPtr->contains("1895715"))
    //   dmy=0;

    StationInfo si(*stationPtr);
    QMap<QString,QString> *sectionsByCruisePtr=
      datasetInfosPtr->sectionsByCruisePtr();
    InfoItem cruise=cruisesPtr->value(stationPtr->cruiseLbl);
    QString geotracesCruise=sectionsByCruisePtr->value(stationPtr->cruiseLbl);
    if (geotracesCruise.isEmpty()) geotracesCruise="unknown_cruise";

    return fmt.arg(geotracesCruise)
      .arg(stationPtr->stationLbls.isEmpty() ?
           QString() : stationPtr->stationLbls.at(0))
      .arg("B")
      .arg(isoDateFromGregorianDay(si.meanTime))
      .arg(si.meanLon)
      .arg(si.meanLat)
      .arg(si.maxBotd)
      .arg(si.samplingDevices().join(" | "))
      .arg(si.castIdentifiers().join(" | "))
      .arg(stationPtr->join(" | "))
      .arg(formattedNumber(si.maxDist,2,true))
      .arg(formattedNumber(si.duration,2,true))
      .arg(stationPtr->cruiseLbl)
      .arg(cruise.at(cruisesPtr->idxShipName))
      .arg(fmtPeriod.arg(cruise.at(cruisesPtr->idxStartTimeDate).left(10))
           .arg(cruise.at(cruisesPtr->idxEndTimeDate).left(10)))
      .arg(cruise.at(cruisesPtr->idxChiefScientist))
      .arg(cruise.at(cruisesPtr->idxGeotracesPi))
      .arg(cruise.at(cruisesPtr->idxAliases))
      .arg(cruise.at(cruisesPtr->idxCruiseReportUrl))
      .arg(cruise.at(cruisesPtr->idxBodcCruiseNumber));
  }
  else
    return "\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t";
}

/**************************************************************************/
QString EventData::methodsIdFromUrl(const QString& methodsUrl)
/**************************************************************************/
/*!

  \brief Extracts the methods id from the methods url \a methodsUrl.

  \return The extracted methods id.

*/
{
  QStringList sl=methodsUrl.split("/");
  return sl.at(sl.size()-2);
}

/**************************************************************************/
QString EventData::piNameFromExtName(const QString& extPiName,
                                     QString *emailAddress)
/**************************************************************************/
/*!

  \brief Extracts the pi name from the extended pi name \a extPiName.

  \return The extracted methods id.

*/
{
  int i=extPiName.indexOf("("),j=extPiName.indexOf(")");
  QString pin=extPiName.trimmed();
  if (i>-1 && j>-1)
    {
      pin=extPiName.left(i).trimmed();
      if (emailAddress) *emailAddress=extPiName.mid(i+1,j-i-1);
    }
  return pin;
}

/**************************************************************************/
int EventData::sampleCount(int bodcBottleNumber)
/**************************************************************************/
/*!

  \brief \return The number of samples for BODC bottle number \a
  bodcBottleNumber.

  Normally, the sample count is 1, except for bottles with individual
  cell measurements, where the number of samples is equal to the
  number of individual cells analyzed.

*/
{
  return cellSampleIdsByBodcBottleNumber.contains(bodcBottleNumber) ?
    cellSampleIdsByBodcBottleNumber.value(bodcBottleNumber).size() : 1;
}

/**************************************************************************/
int EventData::sampleId(int bodcBottleNumber,const QString& cellSampleId)
/**************************************************************************/
/*!

  \brief \return The sample id for BODC bottle number \a
  bodcBottleNumber and cell id \a cellSampleId.

*/
{
  int bottleIdx=bodcBottleNumbers.indexOf(bodcBottleNumber);
  int id=(bottleIdx==-1) ? -1 : firstSmplIds.at(bottleIdx);
  if (id>-1 && cellSampleIdsByBodcBottleNumber.contains(bodcBottleNumber))
    {
      QStringList cellSampleIds=cellSampleIdsByBodcBottleNumber.value(bodcBottleNumber);
      int idx=cellSampleIds.indexOf(cellSampleId);
      if (idx>-1) id+=idx;
    }

  return id;
}

/**************************************************************************/
QString EventData::spreadsheetDataPart(int bodcBottleNumber,int cellSampleIdx)
/**************************************************************************/
/*!

  \brief Constructs the spreadsheet-style data part for bottle
  number \a bodcBottleNumber and cell index \a cellSampleIdx.

  \return The constructed string.

*/
{
  const QString fmt="\t%1\t%2\t%3\t%4";
  int smplIdx=firstSampleId(bodcBottleNumber);
  if (smplIdx==-1) return QString();
  smplIdx+=cellSampleIdx;

  QString s,infoStr; double val,err; char qf;

  /* iterate over all parameters */
  QMap<int,Param> *paramMap=paramSetPtr->paramMapPtr();
  QMap<int,Param>::ConstIterator it;
  for (it=paramMap->constBegin(); it!=paramMap->constEnd(); ++it)
    {
      getValues(it.value().name,smplIdx,val,err,qf,infoStr);

      s+=fmt.arg(formattedNumber(val,6))
        .arg(formattedNumber(err,6)).arg(qf).arg(infoStr);
    }

  return s;
}

/**************************************************************************/
QStringList EventData::spreadsheetDataRecords(int bodcBottleNumber,
                                              bool inclMetaValues)
/**************************************************************************/
/*!

  \brief Constructs the spreadsheet-style data line(s) for bottle
  number \a bodcBottleNumber.

  The meta values are included if \a inclMetaValues is \c true,
  otherwise the meta variable fields are left empty.

  Normally, only 1 record is produced, except for bottles with
  individual cell measurements, where the number of records is equal
  to the number of individual cells analyzed.

  \return The constructed data line(s).

*/
{
  const QString fmt="\t%1";

  QStringList sl;
  bool isSeaWater=(paramSetPtr->dataType()==SeawaterDT);

  int bottleIdx=bodcBottleNumbers.indexOf(bodcBottleNumber);
  int smplIdx=firstSampleId(bodcBottleNumber);
  if (bottleIdx==-1 || smplIdx==-1) return sl;

  /* start with the meta values */
  QString mvStr=metaValueString(inclMetaValues);
  char bf=bodcBottleFlags.at(bottleIdx);
  InfoItem bi=bioGeotracesInfosPtr->value(QString::number(bodcBottleNumber));
  bool haveBi=bi.size()>0;
  QStringList cellSampleIds=cellSampleIdsByBodcBottleNumber.value(bodcBottleNumber);

  /* loop over all records for this bottle number */
  int i,n=sampleCount(bodcBottleNumber);
  for (i=0; i<n; ++i)
    {
      QString l=mvStr;
      /* append data of lead data variables */
      l+=fmt.arg(((double*) dblData.data(depthID))[smplIdx]);
      if (isSeaWater) l+=fmt.arg(((double*) dblData.data(pressureID))[smplIdx]);
      if (isSeaWater) l+=fmt.arg(rosetteBottleNumbers.at(bottleIdx));
      l+=fmt.arg(geotracesSampleIds.at(bottleIdx));
      if (isSeaWater) l+=fmt.arg(QString("%1 (%2)").arg(bottleFlagDescrPtr->value(bf)).arg(bf));
      l+=fmt.arg(eventInfo.castIdentifier);
      l+=fmt.arg(eventInfo.samplingDevice);
      l+=fmt.arg(bodcBottleNumbers.at(bottleIdx));
      l+=fmt.arg(eventInfo.eventNumber);
      if (isSeaWater)
        {
          l+=fmt.arg((cellSampleIds.size()>0) ? cellSampleIds.at(i) : QString());
          l+=fmt.arg(haveBi ? bi.at(1) : QString());
          l+=fmt.arg(haveBi ? bi.at(2) : QString());
          l+=fmt.arg(haveBi ? bi.at(3) : QString());
          l+=fmt.arg(haveBi ? bi.at(4) : QString());
        }

      sl << l+spreadsheetDataPart(bodcBottleNumber,i);
    }

  return sl;
}

/**************************************************************************/
QStringList EventData::spreadsheetDataLines()
/**************************************************************************/
/*!

  \brief Constructs the spreadsheet-style data lines.

  \return The constructed data lines.

*/
{
  QStringList sl; QString l; int i,n=bodcBottleNumbers.size();

  /* loop over all BODC bottle numbers */
  for (i=0; i<n; ++i)
    sl.append(spreadsheetDataRecords(bodcBottleNumbers.at(i),i==0));

  return sl;
}

/**************************************************************************/
QStringList EventData::spreadsheetHeaderLines(ParamSet *paramSet,
                                              InfoMap *keyVarsByDataVar)
/**************************************************************************/
/*!

  \brief Constructs the spreadsheet-style header lines.

  \return The constructed header lines.

*/
{
  const QString fmtPrm="\t%1\tSTANDARD_DEV\tQV:SEADATANET\tINFOS";
  QString header=paramSet->metaVarHeader()+"\t"+paramSet->leadDataVarHeader();

  QStringList sl; InfoItem ii; QString prmLbl,kvLbl;
  sl << QString("//<Encoding>UTF-8</Encoding>\n//<DataField>%1</DataField>\n//<DataType>Profiles</DataType>\n//<Description>%2</Description>\n//").arg(paramSet->collectionField()).arg(paramSet->collectionDescription());
  sl << paramSet->metaVarDefinitionStyledLines();
  sl << "//" << paramSet->leadDataVarDefinitionStyledLines(keyVarsByDataVar);

  QMap<int,Param> *paramMap=paramSet->paramMapPtr(); Param prm;
  QMap<int,Param>::ConstIterator it;
  for (it=paramMap->constBegin(); it!=paramMap->constEnd(); ++it)
    {
      prm=it.value(); prmLbl=prm.fullLabel();
      ii=keyVarsByDataVar->value(prmLbl);
      kvLbl=(ii.isEmpty()) ? QString() : ii.at(1);
      header+=fmtPrm.arg(prmLbl);
      sl << fmtDvDef.arg(prmLbl).arg("FLOAT").arg(3).arg("F")
              .arg(prm.description).arg(kvLbl);
    }

  sl << "//" << header;

  return sl;
}

/**************************************************************************/
void EventData::writeInfoFile(const QString& fn,const QString& prmName,
                              const QList<int> idxList)
/**************************************************************************/
/*!

  \brief Writes the INFO file for parameter \a prmName with contributing data
  from barcode indexes in \a idxList.

*/
{
  const QString fmtA="<a href=\"%1\">%2</a>\n";
  const QString proc1="As provided.";
  const QString proc2="Value obtained as median of data values from above originators. Quality flag is combination of individual flags (poorest quality).";
  QString cruise=stationPtr->cruiseLbl;
  QString cruiseInfoUrl=cruisesPtr->value(cruise).at(cruisesPtr->idxCruiseReportUrl);
  QString geotracesCruise=datasetInfosPtr->sectionsByCruisePtr()->value(cruise);
  int i,n=idxList.size(); InfoItem mi,di;
  QString extPrmName,uPrmName,sSuffix,methodsUrl,methodsId,piEmail;
  QStringList piNames,extPrmNames,sl;
  uPrmName=(unifiedPrms) ? Param::unifiedNameLabel(prmName,sSuffix) : prmName;
  extPrmNames=extPrmNamesByUPrmName.value(uPrmName);

  QFile f(infoDir+fn+".html");
  if (f.open(QIODevice::WriteOnly))
    {
      QTextStream out(&f); out.setCodec("UTF-8");

      out << QString("<!DOCTYPE html>\n<html>\n\n<head>\n<title>%1 Info</title>\n<meta charset=\"UTF-8\">\n<style type=\"text/css\">\nbody { font-family: sans-serif; margin: 30px; }\nh2, h3 { color:#4070AA; }\np { line-height: 1.5; };\n</style>\n</head>\n\n<body>\n\n").arg(prmName);

      out << QString("<p>\n<h2>%1 @ %2 (%3)</h2>\n</p><br>\n\n")
              .arg(uPrmName).arg(geotracesCruise).arg(cruise);

      out << QString("<p>\n<h3>&#149; Parameter Description</h3>\n");
      out << paramSetPtr->paramFor(uPrmName).description << "\n</p><br>\n\n";

      out << QString("<p>\n<h3>&#149; Data Originators and Methods</h3>\n");
      for (i=0; i<n; ++i)
        {
          extPrmName=extPrmNames.at(idxList.at(i));
          mi=docuByExtPrmNamePtr->value(extPrmName);
          di=datasetInfosPtr->value(extPrmName);
          methodsUrl=mi.at(1); methodsId=methodsIdFromUrl(methodsUrl);
          piNames=di.at(datasetInfosPtr->idxDataGenerator).split(" | ");

          out << QString("<p>%1<br><br>\n")
            .arg(sortedNameList(piNames,false,piInfosByNamePtr).join(" | "));
          //out << QString();
          out << fmtA.arg(methodsUrl)
            .arg("Link to detailed originator and methods information");
          out << " | \n";
          out << fmtA.arg(cruiseInfoUrl).arg("Link to cruise information");
          out << "</p>\n";
        }
      out << "</p><br>\n";

      out << QString("<p>\n<h3>&#149; Processing Information</h3>\n");
      out << ((n>1) ? proc2 : proc1) << "\n</p><br>\n\n";

      out << QString("<p>\n<h3>&#149; References</h3>\n");
      out << fmtA.arg(fmtPublicationUrl.arg(geotracesCruise).arg(uPrmName))
              .arg("Link to publications asociated with these data");
      out << "</p><br>\n\n";

      out << "</body>\n</html>\n";
    }
}
