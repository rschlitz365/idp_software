/****************************************************************************
**
** Copyright (C) 2025 Reiner Schlitzer. All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "Datasets.h"

#include <QDir>

#include "globalVars.h"
#include "globalFunctions.h"
#include "Cruises.h"



/**************************************************************************/
DatasetInfos::DatasetInfos(const QString& fn,
                           const QString& keyLabel,QChar splitChar,
                           QStringList *ignoredDatasets)
: InfoMap(fn,keyLabel,splitChar),ignoredDatasetsPtr(ignoredDatasets)
/**************************************************************************/
/*!

\brief Creates a DatasetInfos object and appends the DOoR dataset
entries from file \a fn.

*/
{
  /* determine various column indexes */
  idxCruise=columnIndexOf("CRUISE");
  idxGeotracesCruise=columnIndexOf("GEOTRACES CRUISE");
  idxPrmBarcode=columnIndexOf("PARAMETER::BARCODE");
  idxSiApproval=columnIndexOf("S&I STATUS");
  idxPiPermission=columnIndexOf("PERMISSION");
  idxDataGenerator=columnIndexOf("DATA GENERATOR(S)");
  idxAuthorisedScientist=columnIndexOf("AUTORISED SCIENTIST");
  idxIdpVersion=columnIndexOf("IDP Version");

  QMap<QString,InfoItem>::ConstIterator it; InfoItem ii;
  QString extPrmName,prmName,cruise,gtCruise,resolvedPrmName,contribName;
  bool isSensor,siApproved,piApproved,piPending,isRemoved,isAccepted;
  QStringList sl,siYpiP,siNpiY,slNN; int i,n;
  QMap<QString,int> prmNameMap,contribNameMap;
  siYpiP.append(columnLabels.join(tab)); siNpiY.append(columnLabels.join(tab));
  for (it=constBegin(); it!=constEnd(); ++it)
  {
    extPrmName=it.key(); ii=it.value();
    cruise=ii.at(idxCruise);
    prmName=extPrmName.split("::").at(0);
    resolvedPrmName=prmName+" @ "+cruise;
    gtCruise=ii.at(idxGeotracesCruise); ;
    if ((i=gtCruise.indexOf(" "))>-1) gtCruise=gtCruise.left(i);

    isSensor=extPrmName.contains("_SENSOR");
    siApproved=ii.at(idxSiApproval).startsWith("approved",Qt::CaseInsensitive);
    piApproved=ii.at(idxPiPermission).startsWith("approved",Qt::CaseInsensitive);
    piPending=ii.at(idxPiPermission).startsWith("pending",Qt::CaseInsensitive);
    // siApproved=!ii.at(idxSiApproval).compare("approved",Qt::CaseInsensitive);
    // piApproved=!ii.at(idxPiPermission).compare("approved",Qt::CaseInsensitive);
    // piPending=!ii.at(idxPiPermission).compare("pending",Qt::CaseInsensitive);
    isRemoved=isRemovedDataset(cruise,prmName);
    isAccepted=(!isRemoved && (isSensor || (siApproved && piApproved)));

    if (siApproved) extPrmNamesSiApproved.insert(extPrmName,1);
    if (piApproved) extPrmNamesPiApproved.insert(extPrmName,1);
    if (isAccepted)
      {
        prmNamesAccepted.insert(prmName,1);
        sl=dataGeneratorNameList(ii.at(idxDataGenerator)," | "); n=sl.size();
        for (i=0; i<n; ++i)
          {
            contribName=sl.at(i);

            prmNameMap=acceptedPrmsByContribNames.value(contribName);
            prmNameMap.insert(resolvedPrmName,1);
            acceptedPrmsByContribNames.insert(contribName,prmNameMap);

            contribNameMap=acceptedContribNamesByPrms.value(resolvedPrmName);
            contribNameMap.insert(contribName,1);
            acceptedContribNamesByPrms.insert(resolvedPrmName,contribNameMap);
          }

        sectsByCruiseName.insert(cruise,gtCruise);
      }

    /* for diagnostics */
    if      (!isSensor && siApproved && piPending)
      siYpiP.append(value(extPrmName).join(tab));
    else if (!isSensor && !siApproved && piApproved)
      siNpiY.append(value(extPrmName).join(tab));
  }

  const QString dir=idpOutputDir+"dataset_lists/"; QDir().mkpath(dir);

  /* for diagnostics */
  appendRecords(dir+"SiApproved_PiPending.txt",siYpiP,true);
  appendRecords(dir+"NotSiApproved_PiApproved.txt",siNpiY,true);

  QMap<QString,QString>::ConstIterator its; sl.clear();
  for (its=sectsByCruiseName.constBegin();
       its!=sectsByCruiseName.constEnd(); ++its)
  {
    gtCruise=its.value();
    if (gtCruise.isEmpty())
      slNN.append(QString("%1\t%2").arg(its.key()).arg(gtCruise));
    else
      sl.append(QString("%1\t%2").arg(its.key()).arg(gtCruise));
  }
  appendRecords(dir+"Sections_By_Cruise.txt",sl,true);
  appendRecords(dir+"Sections-no-name_By_Cruise.txt",slNN,true);
}

/**************************************************************************/
QString DatasetInfos::geotracesCruiseNameFor(const QString& cruise)
/**************************************************************************/
/*!

\brief Retrieves the GEOTRACES cruise name for cruise \a cruise.

\return The retrieved GEOTRACES cruise name.

*/
{
  return sectsByCruiseName.value(cruise);
}

/**************************************************************************/
bool DatasetInfos::hasApprovalsForExtendedParamName(const QString& extPrmName)
/**************************************************************************/
/*!

\brief Checks whether extPrmName \a extPrmName has S&I approval and PI
permission, or represents a _SENSOR parameter name.

\return \c true if the requirements are matched, or \c false otherwise.

*/
{
  return extPrmName.contains("_SENSOR") ||
    (extPrmNamesSiApproved.contains(extPrmName) &&
     extPrmNamesPiApproved.contains(extPrmName));
}

/**************************************************************************/
bool DatasetInfos::hasApprovalsForParamName(const QString& prmName)
/**************************************************************************/
/*!

\brief Checks whether parameter with name \a prmName has S&I
approval and PI permission, or represents a _SENSOR parameter name.

\return \c true if the requirements are matched, or \c false otherwise.

*/
{
  return prmName.contains("_SENSOR") || prmNamesAccepted.contains(prmName);
}

/**************************************************************************/
bool DatasetInfos::isRemovedDataset(const QString& cruise,const QString& prmName)
/**************************************************************************/
/*!

\brief Checks whether parameter with name \a prmName at cruise \a
cruise has been removed, e.g., is included in list \a
ignoredDatasets.

\return \c true if the parameter is removed, or \c false otherwise.

*/
{
  int i,n=ignoredDatasetsPtr->size(); QString removedPrmName; QStringList sl;
  for (i=1; i<n; ++i)
    {
      sl=ignoredDatasetsPtr->at(i).split(","); removedPrmName=sl.at(1);
      if (cruise==sl.at(0) &&
        (removedPrmName=="*" || prmName==removedPrmName)) return true;
    }

  return false;
}

/**************************************************************************/
QStringList DatasetInfos::toCruisesStringList(CruisesDB *cruises)
/**************************************************************************/
/*!

\brief \return The list of cruise information strings.

*/
{
  QString cruise,gtCruise,s; QMultiMap<QString,QString> crInfos;
  QMap<QString,QString>::ConstIterator it; QStringList sl; InfoItem cr;
  for (it=sectsByCruiseName.constBegin(); it!=sectsByCruiseName.constEnd(); ++it)
    {
      cruise=it.key(); gtCruise=it.value(); cr=cruises->value(cruise);
      if (gtCruise.isEmpty() || cr.isEmpty()) continue;

      s=QString("%1\t%2\t%3\t%4\t%5\t%6 - %7\t%8").arg(gtCruise).arg(cruise)
        .arg(cr.at(cruises->idxAliases)).arg(cr.at(cruises->idxShipName))
        .arg(cr.at(cruises->idxGeotracesPi))
        .arg(cr.at(cruises->idxStartTimeDate).left(10))
        .arg(cr.at(cruises->idxEndTimeDate).left(10))
        .arg(cr.at(cruises->idxCountry));
      crInfos.insert(gtCruise,s);
    }
  QMultiMap<QString,QString>::ConstIterator itm;
  for (itm=crInfos.constBegin(); itm!=crInfos.constEnd(); ++itm)
    { sl << itm.value(); }

  return sl;
}


