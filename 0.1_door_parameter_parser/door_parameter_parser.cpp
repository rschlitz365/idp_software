/****************************************************************************
 **
 ** Copyright (C) 2025 Reiner Schlitzer. All rights reserved.
 **
 ** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 ** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 **
 ****************************************************************************/

#include "common/globalVars.h"

#include <QDir>
#include <QFile>
#include <QTextStream>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QMap>
#include <QPair>

/**************************************************************************/
bool appendRecords(const QString& fn,const QStringList& records,
                   bool deleteExistingFile)
/**************************************************************************/
/*!
  \brief Appends \a records at the end of file \a fn.

  If \a deleteExistingFile is \c true on entry and file \a fn exists,
  the file is deleted before creating an empty file and appending the
  records.

  \return \c true if successful, or \c false otherwise.
*/
{
  if (fn.isEmpty()) return false;

  /* if requested remove an existing file fn */
  if (deleteExistingFile) QFile::remove(fn);

  /* open the target file for appending. immediate error return if unsuccessful */
  QFile fi(fn); if (!fi.open(QIODevice::Text | QIODevice::Append)) return false;

  /* create an UTF-8 output stream */
  QTextStream out(&fi); out.setCodec("UTF-8");

  /* output the text */
  QStringListIterator it(records);
  while (it.hasNext())
    out << it.next() << Qt::endl;

  return true;
}

/**************************************************************************/
QStringList fileContents(const QString& filePath)
/**************************************************************************/
/*!
  \brief Extracts and returns contents of text file at \a filePath.

  \return The file's contents line by line or in case of errors an
  empty list.
*/
{
  QStringList sl; QFile fi(filePath);

  if (fi.exists() && fi.open(QFile::ReadOnly))
    {
      QTextStream in(&fi); in.setCodec("UTF-8");
      while (!in.atEnd())
        sl << in.readLine();
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
  return jsonVal.isNull() ? dfltStr : jsonVal.toString();
}


/**************************************************************************/
void processGroup(const QString &dir,const QString &fn,
                  const QString &lbl,const QJsonArray &jsonArr)
/**************************************************************************/
/*!

  \brief .

*/
{
  QStringList sl=QStringList()
   << "KEYWORD\tGROUP TITLE\tSUBGROUP\tPARAMETER\tUNITS\tPARAMETER DESCRIPTION"
   << "\t\t\t\t\t"
   << QString("%1\t\t\t\t\t").arg(lbl);
  int i,j,k,l,prmGroupCount=jsonArr.size(),groupCount,subGroupCount,prmCount; QString s;
  QJsonObject jObj,jGroupObj,jSubGroupObj,jPrmsObj;
  QJsonArray jGroupArr,jSubGroupArr,jPrmsArr;
  for (i=0; i<prmGroupCount; ++i)
  {
    jObj=jsonArr.at(i).toObject();
    if (jObj.value("name").toString()==lbl)
    {
      jGroupArr=jObj.value("subitems").toArray();
      groupCount=jGroupArr.size();
      for (j=0; j<groupCount; ++j)
      {
        jGroupObj=jGroupArr.at(j).toObject();
        sl << QString("\t%1\t\t\t\t").arg(jGroupObj.value("name").toString());
        jSubGroupArr=jGroupObj.value("subitems").toArray();
        subGroupCount=jSubGroupArr.size();
        for (k=0; k<subGroupCount; ++k)
        {
          jSubGroupObj=jSubGroupArr.at(k).toObject();
          sl << QString("\t\t%1\t\t\t").arg(jSubGroupObj.value("name").toString());
          jPrmsArr=jSubGroupObj.value("subitems").toArray();
          prmCount=jPrmsArr.size();
          for (l=0; l<prmCount; ++l)
          {
            jPrmsObj=jPrmsArr.at(l).toObject();
            sl << QString("\t\t\t%1\t%2\t%3")
                  .arg(jPrmsObj.value("name").toString())
                  .arg(jPrmsObj.value("unit").toString())
                  .arg(jPrmsObj.value("label").toString());
          }
        }
      }
    }
  }
  appendRecords(dir+fn,sl,true);
}

/**************************************************************************/
int main()
/**************************************************************************/
/*!

  \brief Reads the parameter definitions obtained from DOoR as json
  and creates <prm-grp>*_parameters.txt for all known parameter
  groups: AEROSOL, BIO_GEOTRACES, DISSOLVED_TEI,
  HYDROGRAPHY_AND_BIOGEOCHEMISTRY. LIGAND, PARTICULATE_TEI,
  POLAR, PRECIPITATION, SENSOR.

*/
{
  const QList<QPair<QString,QString> > prmGroups=QList<QPair<QString,QString> >()
    << QPair<QString,QString>(aerosolPrmFileName,"AEROSOLS")
    << QPair<QString,QString>(bioGeotracesPrmFileName,"BioGEOTRACES")
    << QPair<QString,QString>(dissolvedPrmFileName,"DISSOLVED TEIS")
    << QPair<QString,QString>(hydrographyPrmFileName,"HYDROGRAPHY AND BIOGEOCHEMISTRY")
    << QPair<QString,QString>(ligandPrmFileName,"LIGANDS")
    << QPair<QString,QString>(particlePrmFileName,"PARTICULATE TEIS")
    << QPair<QString,QString>(polarPrmFileName,"POLAR")
    << QPair<QString,QString>(precipitationPrmFileName,"PRECIPITATION")
    << QPair<QString,QString>(sensorPrmFileName,"SENSOR");

  QStringList sl=fileContents(idpPrmListInpDir+"parameters.json");
  QJsonParseError jsonErr; QJsonValue jsonVal; QString msg;
  QJsonDocument jsonDoc=QJsonDocument::fromJson(sl.at(0).toUtf8(),&jsonErr);
  if (jsonDoc.isNull()) { msg=jsonErr.errorString(); return 1; }
  QJsonObject jsonObj=jsonDoc.array().at(0).toObject();
  QJsonArray jsonPrmGroupArr=jsonObj.value("items").toArray();

  /* ensure that output directory exists */
  QDir().mkpath(idpPrmListIntermDir);

  /* loop over all parameter groups */
  int i,prmGroupCount=prmGroups.size();
  for (i=0; i<prmGroupCount; ++i)
    {
      processGroup(idpPrmListIntermDir,prmGroups.at(i).first,
                   prmGroups.at(i).second,jsonPrmGroupArr);
    }

  return 0;
}
