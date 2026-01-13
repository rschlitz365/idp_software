/****************************************************************************
 **
 ** Copyright (C) 2025 Reiner Schlitzer. All rights reserved.
 **
 ** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 ** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 **
 ****************************************************************************/

#include "common/globalVars.h"
#include "common/globalFunctions.h"

#include <QDir>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QMap>

#include "common/Replacer.h"

Replacer nameReplacer;

/**************************************************************************/
void extractScientists(const QJsonArray& scientistsArr,
                       QStringList *scientistOrcids,
                       QStringList *scientistNames,
                       QStringList *scientistEmails)
/**************************************************************************/
/*!

  \brief .

*/
{
  int i,n=scientistsArr.size(); QJsonObject scientist; QString orcid;
  scientistOrcids->clear(); scientistNames->clear(); scientistEmails->clear();
  for (i=0; i<n; ++i)
    {
      scientist=scientistsArr.at(i).toObject();
      orcid=scientist.value("orcid").toString().simplified();
      if (orcid.startsWith("XXXX-")) continue;

      scientistOrcids->append(orcid);
      scientistNames->append(nameReplacer.applyTo(scientist.value("name").toString().simplified()));
      scientistEmails->append(scientist.value("email").toString().simplified());
    }
}

/**************************************************************************/
QStringList namesFromOrcIDs(const QStringList& orcIds,
                            const QMap<QString,QString>& namesByOrcIds)
/**************************************************************************/
/*!

  \brief .

*/
{
  int i,n=orcIds.size(); QString orcId; QStringList sl;
  for (i=0; i<n; ++i)
    {
      orcId=orcIds.at(i);
      if (namesByOrcIds.contains(orcId)) sl << namesByOrcIds.value(orcId);
      else                               sl << "name unknown";
    }
  return sl;
}

/**************************************************************************/
QString jsonStrValue(const QJsonValue& jsonVal,
                     const QString& dfltStr=QString("null"))
/**************************************************************************/
/*!

  \brief .

*/
{
  return (jsonVal.isNull() ? dfltStr : jsonVal.toString()).simplified();
}

/**************************************************************************/
int main()
/**************************************************************************/
/*!

  \brief The following is an example DOoR dataset entry showing all
  property names:

  {
  "barcode":"odqmts",
  "pi":"0000-0003-1655-297X",
  "parameter":"SILICATE_30_28_D_DELTA_BOTTLE",
  "name":"Janice L. Jones",
  "piEmail":"ja_jones@ucsb.edu",
  "authorisedResearcherOrcid":"0000-0003-3432-2297",
  "cruise":"KN199",
  "geotracesCruise":"GA03",
  "creationDate":null,
  "submissionDate":1579627791924,
  "approvalDate":1617120684334,
  "gdacDataSentDate":null,
  "intercalibrationReportSubmitted":true,
  "intercalibrationReportPath":null,
  "intercalibrationReportId":"5e27350fa3048c6dbb4c5eca",
  "intercalibrated":true,
  "bodcId":null,
  "status":"approved",
  "gdacStatus":"Processed",
  "gdacDatasetId":"1013367",
  "dataSentToReiner":null,
  "idpVersion":"IDP2017",
  "comment":"In IDP2017",
  "datasetFileName":null,
  "datasetFileUrl":null,
  "datagdac":[],
  "permissionToUseInIdp":"true",
  "dois":["10.1016/j.dsr2.2014.11.015"],
  "doiDataset":["10.26008/1912/bco-dmo.4070.1"],
  "scientists":[{"orcid":"0000-0003-3432-2297","email":"mark.brzezinski@lifesci.ucsb.edu","name":"Mark Brzezinski"},{"orcid":"0000-0003-1655-297X","email":"ja_jones@ucsb.edu","name":"Janice L. Jones"}],
  "compliant":false,
  "proofCheckImageFile":"SILICATE_30_28_D_DELTA_BOTTLE_odqmts.jpg",
  "proofCheckDataFile":"SILICATE_30_28_D_DELTA_BOTTLE_odqmts.csv",
  "proofCheckImageFileStat":0,
  "proofCheckDataFileStat":0,
  "creator":null
  }

*/
{
  const QString fmtExtPrmName="%1::%2";
  const QString fmtNoPi="No name for authorized PI OrcId=%1";
  const QStringList columnLbls=QStringList()
    << "GEOTRACES CRUISE"
    << "CRUISE"
    << "PARAMETER::BARCODE"
    << "IDP Version"
    << "GDAC DATASET ID"
    << "GDAC DATASET STATUS"
    << "PERMISSION"
    << "S&I STATUS"
    << "SUBMITTER"
    << "AUTORISED SCIENTIST"
    << "DATA GENERATOR(S)";

  // nameReplacer.append("Abigail JR Smith","Abigail Smith");
  // nameReplacer.append("Tristan J. Horner","Tristan Horner");
  // nameReplacer.append("Timothy C Kenna","Timothy Kenna");

  QStringList sl=fileContents(idpDataSetInpDir+"gdac_DataList.json"),vals,keys,itemVals;
  QJsonParseError jsonErr; QJsonValue jsonVal; QString msg;
  QJsonDocument jsonDoc=QJsonDocument::fromJson(sl.at(0).toUtf8(),&jsonErr);
  if (jsonDoc.isNull()) { msg=jsonErr.errorString(); return 1; }
  QJsonObject jsonDataset,jsonDocObj=jsonDoc.object();
  QJsonArray jsonDatasetArr=jsonDocObj.value("content").toArray();

  QStringList scientistOrcids,scientistNames,scientistEmails;
  QMap<QString,QString> namesByOrcIds,emailsByOrcIds,extPrmNamesByUnknownOrcIds;
  QString orcId,submitterOrcId,submitterName,submitterEmail;
  QString authorizedOrcId; int i,j,n,datasetCount=jsonDatasetArr.size();

  /* loop over all dataset entries and generate the name and email by OrcId maps */
  // namesByOrcIds.insert("0000-0003-2643-9567","Katrin Bluhm");
  // emailsByOrcIds.insert("0000-0003-2643-9567","katrin.bluhm@akvaplan.niva.no");
  for (i=0; i<datasetCount; ++i)
    {
      jsonDataset=jsonDatasetArr.at(i).toObject();

      submitterOrcId=jsonStrValue(jsonDataset.value("pi"),"");
      submitterName=nameReplacer.applyTo(jsonStrValue(jsonDataset.value("name"),""));
      submitterEmail=jsonStrValue(jsonDataset.value("piEmail"),"");
      if (!submitterOrcId.isEmpty())
        {
          if (!submitterName.isEmpty())
            namesByOrcIds.insert(submitterOrcId,submitterName);
          if (!submitterEmail.isEmpty())
            emailsByOrcIds.insert(submitterOrcId,submitterEmail);
        }

      extractScientists(jsonDataset.value("scientists").toArray(),
                        &scientistOrcids,&scientistNames,&scientistEmails);
      for (j=0; j<scientistOrcids.size(); ++j)
        {
          orcId=scientistOrcids.at(j);
          if (!namesByOrcIds.contains(orcId))
            namesByOrcIds.insert(orcId,scientistNames.at(j));
          if (!emailsByOrcIds.contains(orcId))
            emailsByOrcIds.insert(orcId,scientistEmails.at(j));
        }
    }

  /* ensure that output directory exists */
  QDir().mkpath(idpDataSetIntermDir);

  /* create file with name and email by OrcId */
  keys=namesByOrcIds.keys(); n=keys.size(); sl.clear();
  sl << "ORCID\tNAME\tEMAIL";
  for (i=0; i<n; ++i)
    {
      sl << QString("%1\t%2\t%3").arg(keys.at(i))
        .arg(namesByOrcIds.value(keys.at(i))).arg(emailsByOrcIds.value(keys.at(i)));
    }
  appendRecords(idpDataSetIntermDir+"orcid_list.txt",sl,true);

  /* loop over all dataset entries again and create output records. */
  QMap<QString,QStringList> datasetInfos; QString prmName,barcode,piPermission;
  for (i=0; i<datasetCount; ++i)
    {
      keys.clear(); vals.clear();
      jsonDataset=jsonDatasetArr.at(i).toObject();

      submitterOrcId=jsonStrValue(jsonDataset.value("pi"),"");
      authorizedOrcId=jsonDataset.value("authorisedResearcherOrcid").toString().simplified();

      /* if no authorized scientist but submitter exists: use
         submitter as authorized scientist */
      if (authorizedOrcId.isEmpty() && !submitterOrcId.isEmpty())
        authorizedOrcId=submitterOrcId;

      extractScientists(jsonDataset.value("scientists").toArray(),
                        &scientistOrcids,&scientistNames,&scientistEmails);

      /* if not already present, prepend the authorized scientist to
         the OrcID list of data generators */
      if (scientistOrcids.indexOf(authorizedOrcId)==-1)
          scientistOrcids.prepend(authorizedOrcId);
      scientistNames=namesFromOrcIDs(scientistOrcids,namesByOrcIds);

      prmName=jsonDataset.value("parameter").toString().simplified();
      barcode=jsonDataset.value("barcode").toString().simplified();
      piPermission=jsonStrValue(jsonDataset.value("permissionToUseInIdp")).simplified();
      if      (piPermission=="true") piPermission="approved";
      else if (piPermission=="false") piPermission="not approved";
      else if (piPermission=="undefined") piPermission="pending";

      //debug start
      // int dmy;
      // if (barcode=="hkqyhq" && prmName=="DIC_13_12_D_DELTA_BOTTLE")
      // QString dmy=scientistNames.join(" | ");;
      // if (dmy.size()<5)
      //   dmy=1;
      //debug end

      keys << jsonStrValue(jsonDataset.value("geotracesCruise"),"");
      keys << jsonStrValue(jsonDataset.value("cruise"));
      keys << prmName+"::"+barcode;
      vals << jsonStrValue(jsonDataset.value("idpVersion"));
      vals << jsonStrValue(jsonDataset.value("gdacDatasetId"));
      vals << jsonStrValue(jsonDataset.value("gdacStatus"));
      vals << piPermission;
      vals << jsonStrValue(jsonDataset.value("status"));
      vals << (namesByOrcIds.contains(submitterOrcId) ?
               namesByOrcIds.value(submitterOrcId) : "_no submitter_");
      vals << (namesByOrcIds.contains(authorizedOrcId) ?
               namesByOrcIds.value(authorizedOrcId) : "_unknown authorized scientist_");
      vals << scientistNames.join(" | ");

      datasetInfos.insert(keys.join(":"),keys+vals);

      if (!namesByOrcIds.contains(authorizedOrcId))
        extPrmNamesByUnknownOrcIds.insert(authorizedOrcId,
                                          fmtExtPrmName.arg(prmName).arg(barcode));
    }

  sl.clear(); sl << columnLbls.join("\t");
  QMap<QString,QStringList>::ConstIterator it;
  for (it=datasetInfos.constBegin(); it!=datasetInfos.constEnd(); ++it)
    { sl << it.value().join("\t"); }
  appendRecords(idpDataSetIntermDir+"gdac_DataList_essentials.txt",sl,true);

  sl.clear();
  QMap<QString,QString>::ConstIterator its;
  for (its=extPrmNamesByUnknownOrcIds.constBegin();
       its!=extPrmNamesByUnknownOrcIds.constEnd(); ++its)
    { sl << QString("%1\t%2").arg(its.key()).arg(its.value()); }

  QDir().mkpath(idpErrorsDir);
  appendRecords(idpErrorsDir+"Unnamed_OrcIds.txt",sl,true);

  return 0;
}
