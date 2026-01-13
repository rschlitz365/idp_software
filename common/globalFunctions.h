#ifndef GLOBALFUNCTIONS_H
#define GLOBALFUNCTIONS_H

/****************************************************************************
**
** Copyright (C) 2025 Reiner Schlitzer. All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <QString>
#include <QStringList>

#include "globalDefines.h"
#include "common/RConfig.h"

class InfoMap;

/**global functions*******************************************************/
bool appendRecord(const QString& fn,const QString& record,
                  bool deleteExistingFile=false);
bool appendRecords(const QString& fn,const QStringList& records,
                   bool deleteExistingFile=false);
bool appendRecordsFromFile(const QString& fn,const QString& srcFn);
QMap<char,QString> bottleFlagDescriptions();
double calDepthEOS80(double p,double lat);
double calPressEOS80(double depth,double lat);
QString chopPrefix(const QString& str,const QStringList& prefixes,
                   const QString& prefixExtra);
bool chopSuffix(QString& str,const QStringList& suffixes);
bool chopTrailingZeros(QString& str);
char combinedSdnQualityFlag(const QList<char>& sdnFlags);
int copyDir(const QString& src,const QString& trg,const QString& filter);
QString currentDateTimeAsFileNamePart();
QStringList dataGeneratorNameList(const QString& str,const QString& splitStr);
QString dateStampedFileName(const QString& fn,const QString& ext=QString("txt"));
void decomposeName(const QString fullName,QString& firstName,QString& lastName);
void decomposePath(const QString filePath,QString& dirPath,QString& fn);
void decomposePathEx(const QString& filePath,
                     QString& dirPath,QString& fn,QString& fileExt);
double distance(double lon1,double lat1,double lon2,double lat2);
QMap<QString,QString> eGeotracesVarDescriptions(RConfig& varsCf);
QMap<QString,QString> eGeotracesVars(RConfig& varsCf);
double extractedDouble(const QString& valStr);
int extractedInt(const QString& valStr);
QStringList fileContents(const QString& filePath);
QString fileSignature(const QString& filePath);
QString firstDiffIndicatorStr(int idx);
QString firstDiffIndicatorStr(const QString& str,const QString& strC);
QString formattedNumber(double d,int decCount,bool doChopTrailingZeros=false,
                        bool clearMissDouble=true);
void generateBaseNameFileList(const QString dir,const QString fSpec,QStringList& sl);
void generateFileList(const QString rootDir,const QString fSpec,
                      bool doRecurse,QStringList& sl);
int indexOfFirstDiff(const QString& str,const QString& strC);
int indexOfContains(const QString& str,const QStringList& sl,
                    int from=0,Qt::CaseSensitivity cs=Qt::CaseInsensitive);
int indexOfContainsStr(const QStringList& sl,const QString& str,
                       int from=0,Qt::CaseSensitivity cs=Qt::CaseInsensitive);
int indexOfSampleDevice(const QList<QPair<QString,int> >& ssLst,const QString& smplDev);
int indexOfStartsWith(const QString& str,const QStringList& sl,
                      int from=0,Qt::CaseSensitivity cs=Qt::CaseInsensitive);
void indexx(int n,double arrin[],int indx[]);
QList<int> intListFromStr(const QString& str,const QString& splitStr);
QString md5Hash(const QString& filePath);
double meanOf(double d1,double d2);
double medianVal(double *vals,int count,double valMiss,double *dWrk,int *iWrk);
double myround(double val,int decim);
char* mystrlwr(char *szB);
QString referenceURL(const QString& geotracesId,const QString& prmName);
QString relativePathFromAbsolute(const QString &absPath,const QString &baseDir);
QStringList sortedNameList(const QStringList& names,bool lastNameFirstName,
                           InfoMap *piInfosByName=NULL,
                           QStringList *nonOrcIdNames=NULL);
QStringList splitString(const QString& str,QChar sepChar,
                        const QString& textQuote=QString("\""));
void stripEnclosingChars(QString& s,const QChar& startChar,const QChar& endChar);
QString toLastNameFirstName(const QString& firstName,const QString& lastName,
                            const QString& separator=QString(", "));
QString toLastNameFirstName(const QString& fullName,
                            const QString& separator=QString(", "));

/**************************************************************************/
template <typename T>
void initArray(T *t,int n,T value)
/**************************************************************************/
/*!
  \brief Initializes \a n elements of \a t with value \a value,
  starting at first position.
*/
{
  for (int i=0; i<n; ++i) t[i]=value;
}

#endif   // GLOBALFUNCTIONS_H
