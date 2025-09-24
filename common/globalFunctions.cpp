/****************************************************************************
 **
 ** Copyright (C) 2025 Reiner Schlitzer. All rights reserved.
 **
 ** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 ** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 **
 ****************************************************************************/

#include "globalFunctions.h"

#include <math.h>

#include <QCryptographicHash>
#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QFileInfoList>
#include <QTextStream>

#include "globalVars.h"
#include "InfoMap.h"
#include "common/constants.h"
#include "common/odv.h"

/**************************************************************************/
double adjustedLongitude(double lon)
/**************************************************************************/
/*!
  \brief Adjusts longitude \a lon to [0 - 360] range if within 10
  degrees of the date line.

  \return The adjusted longitude.
*/
{
  double d=lon+180.;
  if (d>0. && d<10.) lon+=360.;
  return lon;
}

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
bool appendRecordsFromFile(const QString& fn,const QString& srcFn)
/**************************************************************************/
/*!
  \brief Appends records from file \a srcFn at the end of file \a fn.

  \return \c true if successful, or \c false otherwise.

*/
{
  /* open the source file for reading. immediate error return if unsuccessful */
  QFile srcFi(srcFn);
  if (!srcFi.open(QIODevice::Text | QIODevice::ReadOnly)) return false;

  /* create UTF-8 input stream and read input stream into records string list */
  QStringList records; QTextStream in(&srcFi); in.setCodec("UTF-8");
  while (!in.atEnd())
    records << in.readLine();

  /* append records at the end of file fn */
  return appendRecords(fn,records);
}

/**************************************************************************/
QMap<char,QString> bottleFlagDescriptions()
/**************************************************************************/
/*!

  \brief \return A QMap containing the bottle flag desccriptions by
  bottle flag code.

*/
{
  QMap<char,QString> bfd;
  bfd.insert('0',"No problem reported");
  bfd.insert('1',"Filter burst");
  bfd.insert('2',"Leakage contamination");
  bfd.insert('3',"Bottle misfire");
  bfd.insert('4',"Bottles fired in incorrect order");
  bfd.insert('5',"Bottle leak");
  bfd.insert('6',"Partial sample loss");
  bfd.insert('7',"No sample");
  bfd.insert('8',"Questionable depth");
  bfd.insert('9',"Vent left open");
  return bfd;
}

/**************************************************************************/
double calDepthEOS80(double p,double lat)
/**************************************************************************/
/*!

  \brief Calculates depth from pressure using the Saunders and
  Fofonoff (1976) conversion function.

  Units:
  \verbatim
  pressure        p        decibars
  latitude        lat      degrees
  depth           depth    meters
  \endverbatim
*/
{
  /* Saunders and Fofonoff's method. Deep-Sea Res., 1976, 23,
     109-111. Formula refitted for 1980 equation of state. */
  double d,gr,x,a,b;
  x=sin(DEG2RAD*lat); x=x*x; a=5.2788e-3+2.36e-5*x;
  gr=9.780318*(1.0+a*x)+1.092e-6*p; a=-1.82e-15*p+2.279e-10;
  b=(a*p-2.2512e-5)*p; d=(b+9.72659)*p;
  return d/gr;
}

/**************************************************************************/
double calPressEOS80(double depth,double lat)
/**************************************************************************/
/*!

  \brief Calculates pressure from depth using the Saunders and
  Fofonoff (1981) conversion function.

  Units:
  \verbatim
  depth           depth    meters
  latitude        lat      degrees
  pressure        p        decibars
  \endverbatim

*/
{
  /* Saunder's formula with EOS80. Saunders,Peter M., Practical
     conversion of pressure to depth., JPO , April 1981. Check Value:
     pressure=7500.004 dbars; for: lat=30 deg., depth=7321.45 meters */
  double c,d,plat,pr;
  /* detect missing value situation */
  if (depth==ODV::missDOUBLE) return ODV::missDOUBLE;
  /* do the calculation */
  plat=fabs(DEG2RAD*lat); d=sin(plat); c=1.-(5.92e-3+5.25e-3*d*d);
  /*  pr=(c-sqrt(c*c-(8.84e-6*depth)))/4.42e-6; */
  pr=(c-sqrt(c*c-(8.84e-6*depth)))*226244.3;
  return pr;
}

/**************************************************************************/
QString chopPrefix(const QString& str,const QStringList& prefixes,
                   const QString& prefixExtra)
/**************************************************************************/
/*!

  \brief Checks whether \a str starts with one of the prefixes in \a
  prefixes appended by \a prefixExtra (search is in order of
  appearance in \a prefixes) and on first match removes the matched
  prefix and \a prefixExtra.

  \return The modified string. The original string is returned if no
  matching prefix is found.

*/
{
  int i,n=prefixes.size(),extraLength=prefixExtra.size();
  for (i=0; i<n; ++i)
    {
      if (str.startsWith(prefixes.at(i)+prefixExtra))
        return str.mid(prefixes.at(i).size()+extraLength);
    }
  return str;
}

/**************************************************************************/
bool chopSuffix(QString& str,const QStringList& suffixes)
/**************************************************************************/
/*!

  \brief Checks whether \a str ends with one of the suffixes in \a
  suffixes (search is in order of appearance in \a suffixes) and on
  first match removes the matched suffix (in-place change).

  \return \c true if \a str was changed, or \c false otherwise.
*/
{
  int i,n=suffixes.size();
  for (i=0; i<n; ++i)
    {
      if (str.endsWith(suffixes.at(i)))
        { str.chop(suffixes.at(i).size()); return true; }
    }
  return false;
}

/**************************************************************************/
bool chopTrailingZeros(QString& str)
/**************************************************************************/
/*!

  \brief Removes trailing zeroes right of the decimal point in \a str.

  \return \c true if string was modified, and \c false otherwise.
*/
{
  int i=str.indexOf(QChar('.')); if (i==-1) return false;

  bool modified=false; QChar zero=QChar('0');
  while (str.size()>i && str.endsWith(zero))
    { str.chop(1); modified=true; }

  return modified;
}

/**************************************************************************/
char combinedSdnQualityFlag(const QList<char>& sdnFlags)
/**************************************************************************/
/*!

  \brief .

  \return .
*/
{
  int i,n=sdnFlags.size(),idx,m=0; char sdnQf,odvQf='0';

  for (i=0; i<n; ++i)
    {
      sdnQf=sdnFlags.at(i); if (sdnQf=='9') continue;

      ++m; idx=sdnQFlags.indexOf(sdnQf);
      odvQf=qMax(odvQf,mappedOdvQFlags.at(idx));
    }

  return (m==0) ? '9' : mappedSdnQFlags.at(odvQFlags.indexOf(odvQf));
}

/**************************************************************************/
int copyDir(const QString& src,const QString& trg,const QString& filter)
/**************************************************************************/
/*!
  \brief Copies all files in directory \a src that match one of the
  file filters in \a filters to directory \a trg.

  If the target directory does not exist, \a trg as well as all
  necessary parent directories will be created.
  The target file attributes will be set to \a attribute.

  \note This function does not copy sub-directories contained in \a
  src.

  \return The number of files copied.
*/
{
  int n=0; QString tFn; QDir tDir(trg),sDir(src);
  QStringList sl(sDir.entryList(QStringList(filter),
                                QDir::Files | QDir::NoDotAndDotDot |
                                QDir::Readable | QDir::Hidden));
  QStringList::ConstIterator it=sl.constBegin();

  if (!tDir.exists()) tDir.mkpath(trg);

  QFile::Permissions attribute=QFile::ReadUser | QFile::WriteUser
    | QFile::ReadGroup | QFile::ReadOther;
  for (; it!=sl.constEnd(); ++it)
    {
      tFn=trg+*it; QFile::remove(tFn); // copy does not overwrite
      if (QFile::copy(src+*it,tFn))
        { ++n; QFile::setPermissions(tFn,attribute); }
    }

  return n;
}

/**************************************************************************/
QString currentDateTimeAsFileNamePart()
/**************************************************************************/
/*!

  \brief Constructs and returns an ISO 8601-like string versions of
  the current date and time that can be used as part of a directory or
  file path.

*/
{
  return QDateTime::currentDateTime().toString(Qt::ISODate)
    .replace(QChar(':'),QChar('-'));
}

/**************************************************************************/
QStringList dataGeneratorNameList(const QString& str,const QString& splitStr)
/**************************************************************************/
/*!

  \brief Constructs and returns the list of data generator names based on
  \a str using \a splitStr as split string.

*/
{
  QStringList sl=str.split(splitStr),dgnLst; int i,n=sl.size(); QString s;
  for (i=0; i<n; ++i)
  {
    s=sl.at(i).trimmed();
    if (s.endsWith(" | ")) { s.chop(3); s=s.trimmed(); }
    if (s.endsWith(" |"))  { s.chop(2); s=s.trimmed(); }
    if (s.endsWith("| "))  { s.chop(2); s=s.trimmed(); }
    if (!s.isEmpty()) dgnLst << s;
  }
  return dgnLst;
}

/**************************************************************************/
QString dateStampedFileName(const QString& fn,const QString& ext)
/**************************************************************************/
/*!

  \brief Constructs and returns a date-stamped file name of the form
  \a fn + "_" + ISO 8601 date/time + "."+ \a ext.

*/
{
  return fn+"_"+currentDateTimeAsFileNamePart()+"."+ext;
}

/**************************************************************************/
void decomposeName(const QString fullName,QString& firstName,QString& lastName)
/**************************************************************************/
/*!
  \brief Decomposes full name \a fullName into first and last names.

  Example: Sebastian M. Vivancos yields \a firstName "Sebastian M."
  and \a lastName "Vivancos".
*/
{
  int i=fullName.lastIndexOf(" ");
  if (i==-1)
    { firstName=QString(); lastName=fullName; }
  else
    { firstName=fullName.left(i); lastName=fullName.mid(i+1); }
}

/**************************************************************************/
void decomposePath(const QString filePath,QString& dirPath,QString& fn)
/**************************************************************************/
/*!
  \brief Breaks full path-name \a filePath into drive/directory and
  name/ext components.

  Example:
  - \a filePath = \c c:/tmp/xyz.tar.gz
  - \a dirPath  = \c c:/tmp/
  - \a fn       = \c xyz.tar.gz
*/
{
  QFileInfo fi(filePath); dirPath=fi.path();
  if (!dirPath.endsWith("/") && !dirPath.endsWith("\\")) dirPath+="/";
  fn=fi.fileName();
}

/**************************************************************************/
double distance(double lon1,double lat1,double lon2,double lat2)
/**************************************************************************/
/*!
  \brief Returns distance (in km) between two points with lon/lat
  coordinates \a lon1 / \a lat1 and \a lon2 / \a lat2.
*/
{
  int i,n; const double fac=111.194929,dstep=1.;
  double dlon=lon2-lon1,dlat=lat2-lat1,dx,dy,N,I,dist=0.;

  if (dlon==0.) return fabs(fac*dlat);
  else
    {
      n=(int) (fabs(dlat)/dstep)+1; N=n; dx=dlon/N; dy=dlat/N;
      for (i=1; i<=n; ++i)
        {
          I=i; dlon=fac*cos(DEG2RAD*(lat1+(I-0.5)*dy))*dx;
          dlat=fac*dy; dist+=sqrt(dlat*dlat+dlon*dlon);
        }
      return dist;
    }
}

/**************************************************************************/
QMap<QString,QString> eGeotracesVarDescriptions(RConfig& varsCf)
/**************************************************************************/
/*!

  \brief Iterates over all variable groups in \a varsCf and constructs
  the set of eGEOTRACES variable descriptions.

  Keys are variable names as they appear in JavaScript code and values
  are the descriptions.

  \note The sampling system suffixes are removed in the variable names.

  \return The set of eGEOTRACES variable descriptions.

*/
{
  QMap<QString,QString> vars; QStringList sl,groupNames=varsCf.groupNames();
  RConfig::ConfigGroup::Iterator it;
  foreach (QString groupName,groupNames)
    {
      varsCf.setGroup(groupName);
      for (it=(*(varsCf.groupIterator())).begin();
           it!=(*(varsCf.groupIterator())).end(); ++it)
        { sl=it.value().split("; "); vars.insert(sl.at(0),sl.at(2)); }
    }

  return vars;
}

/**************************************************************************/
QMap<QString,QString> eGeotracesVars(RConfig& varsCf)
/**************************************************************************/
/*!

  \brief Iterates over all variable groups in \a varsCf and constructs
  the set of eGEOTRACES variables.

  Keys are variable names as they appear in JavaScript code and values
  are the labels as they appear in the combo-boxes on the eGEOTRACES
  web pages.

  \note The sampling system suffixes are removed in the variable names.

  \return The set of eGEOTRACES variables.

*/
{
  QMap<QString,QString> vars; QStringList sl,groupNames=varsCf.groupNames();
  RConfig::ConfigGroup::Iterator it;
  foreach (QString groupName,groupNames)
    {
      varsCf.setGroup(groupName);
      for (it=(*(varsCf.groupIterator())).begin();
           it!=(*(varsCf.groupIterator())).end(); ++it)
        { sl=it.value().split("; "); vars.insert(sl.at(0),sl.at(1)); }
    }

  return vars;
}

/**************************************************************************/
double extractedDouble(const QString& valStr)
/**************************************************************************/
/*!
  \brief Extracts a double value from \a valStr.

  \return The extracted value, or \c ODV::missDOUBLE if extraction failed.
*/
{
  bool ok; double d=valStr.toDouble(&ok);
  return ok ? d : ODV::missDOUBLE;
}

/**************************************************************************/
int extractedInt(const QString& valStr)
/**************************************************************************/
/*!
  \brief Extracts a integer value from \a valStr.

  \return The extracted value, or \c ODV::missINT32 if extraction failed.
*/
{
  bool ok; int i=valStr.toInt(&ok);
  return ok ? i : ODV::missINT32;
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
QString fileSignature(const QString& filePath)
/**************************************************************************/
/*!

  \brief Constructs the signature for file \a filePath consisting of
  last modified date, byte size and hex encoded MD5 hash.

  \a filePath must be an absolute path.

  Example signature:

  last modified: 2015-10-16T12:35:42 | byte size: 45203110 | MD5: 0d93ab7be35c8715a16e7c0fea5aed5c

  \return The constructed signature. Individual entries are separated
  by ' | ' strings.

*/
{
  QFileInfo fi(filePath);
  return QString("last modified: %1 | byte size: %2 | MD5: %3")
    .arg(fi.lastModified().toString(Qt::ISODate))
    .arg(fi.size())
    .arg(md5Hash(filePath));
}

/**************************************************************************/
QString firstDiffIndicatorStr(int idx)
/**************************************************************************/
/*!

  \brief Constructs a string of length \idx+1 consisting of spaces except
  for a caret at index \idx.

  \return The constructed string.

  \sa indexOfFirstDiff()

*/
{
  QString s;
  if (idx>-1) { s.fill(QChar(' '),idx+1); s[idx]=QChar('^'); }
  return s;
}

/**************************************************************************/
QString firstDiffIndicatorStr(const QString& str,const QString& strC)
/**************************************************************************/
/*!

  \brief Constructs and returns the string representation of \a i with
  commas as thousands separator.

  \sa indexOfFirstDiff()
*/
{
  return firstDiffIndicatorStr(indexOfFirstDiff(str,strC));
}

/**************************************************************************/
QString formattedNumber(double d,int decCount,bool doChopTrailingZeros,
                        bool clearMissDouble)
/**************************************************************************/
/*!

  \brief Constructs and returns a string representation of \a d.

  \a decCount is the number of significant digits to be used.

*/
{
  if (d==ODV::missDOUBLE && clearMissDouble) return QString();

  double a=fabs(d),b=floor(a); if ((a-b)<1.e-8 && doChopTrailingZeros) decCount=0;
  QString n=(decCount>0 && a!=0. && (a<1.e-5 || a>1.e6)) ?
    QString::number(d,'g',decCount+3) : QString("%1").arg(d,0,'f',decCount);
  if (doChopTrailingZeros) chopTrailingZeros(n);
  return n;
}


/**************************************************************************/
void generateBaseNameFileList(const QString dir,const QString fSpec,
                              QStringList& sl)
/**************************************************************************/
/*!

  \brief Generates list of file basenames (no extension) in \a dir
  satisfying specification \a fSpec and appends the basenames to the
  list \a sl.

*/
{
  QFileInfoList l=QDir(dir).entryInfoList(QStringList(fSpec));
  QFileInfoList::ConstIterator it;
  for (it=l.constBegin(); it!=l.constEnd(); ++it)
    sl.append(it->completeBaseName());
}

/**************************************************************************/
void generateFileList(const QString rootDir,const QString fSpec,
                      bool doRecurse,QStringList& sl)
/**************************************************************************/
/*!

  \brief Generates list of files in \a rootDir satisfying
  specification \a fSpec and appends the absolute paths of all such
  files to the list \a sl.

  Search starts at \a rootDir and (if \a doRecurse==true) visits all
  sub-directories.

*/
{
  QDir dir(rootDir); QRegExp rx(fSpec,Qt::CaseInsensitive,QRegExp::Wildcard);
  dir.setFilter(QDir::Dirs | QDir::Files | QDir::NoSymLinks | QDir::Hidden);
  dir.setSorting(QDir::Name);

  QFileInfoList l=dir.entryInfoList(); QFileInfoList::ConstIterator it;

  for (it=l.constBegin(); it!=l.constEnd(); ++it)
    {
      QString fn=it->fileName();
      if (fn=="." || fn=="..")
        ;
      else
        {
          if (it->isDir() && it->isReadable() && doRecurse)
            { generateFileList(it->absoluteFilePath(),fSpec,doRecurse,sl); }
          else
            {
              //if (fn.contains(rx)) sl << it->absoluteFilePath();
              if (rx.exactMatch(fn)) sl << it->absoluteFilePath();
            }
        }
    }
}

/**************************************************************************/
int indexOfFirstDiff(const QString& str,const QString& strC)
/**************************************************************************/
/*!

  \brief Determines the 0-based index of the first character difference
  between \a str and \a strC.

  \return The determined index, or \c -1 if the two strings are identical.

  \sa firstDiffIndicatorStr()

*/
{
  int i,n=str.size(),nC=strC.size(),m=qMin(n,nC);
  for (i=0; i<m; ++i)
    if (str.at(i)!=strC.at(i)) return i;

  return (n==nC) ? -1 : m;
}

/**************************************************************************/
int indexOfSampleDevice(const QList<QPair<QString,int> >& ssLst,
                        const QString& smplDev)
/**************************************************************************/
/*!

  \brief Determines the 0-based index of the first occurance sampling
  device string \a smplDev as first part in the list \a ssLst.

  \return The determined index, or \c -1 if no match is found.

*/
{
  int i,n=ssLst.size();
  for (i=0; i<n; ++i)
    if (ssLst.at(i).first==smplDev) return i;

  return -1;
}

/**************************************************************************/
void indexx(int n,double arrin[],int indx[])
/**************************************************************************/
/*!
  \brief Indexes an array \a arrin of length \a n.

  Outputs the array \a indx such that \a arrin(indx(j)) is in
  ascending order for \a j=0,1,..\a n-1.  The input quantities \a n
  and \a arrin are not changed.

  from: Numerical Recipes
*/
{
  int l,j,ir,indxt,i; double q;

  for (j=1;j<=n;++j) indx[j-1]=j;
  if (n<2) { indx[0]=0; return; }
  l=(n >> 1) + 1; ir=n;

  for (;;)
    {
      if (l > 1)
        q=arrin[(indxt=indx[--l-1])-1];
      else
        {
          q=arrin[(indxt=indx[ir-1])-1];
          indx[ir-1]=indx[0];
          if (--ir == 1)
            {
              indx[0]=indxt;
              for (j=0;j<n;++j) indx[j]=indx[j]-1;
              return;
            }
        }

      i=l; j=l << 1;

      while (j <= ir)
        {
          if (j < ir && arrin[indx[j-1]-1] < arrin[indx[j]-1]) ++j;
          if (q < arrin[indx[j-1]-1])
            {
              indx[i-1]=indx[j-1];
              j += (i=j);
            }
          else j=ir+1;
        }
      indx[i-1]=indxt;
    }
}

/**************************************************************************/
QList<int> intListFromStr(const QString& str,const QString& splitStr)
/**************************************************************************/
/*!

  \brief Splits using \a splitStr and extracts integer values from
  string \a str.

  \return The list of extracted integer values.

*/
{
  QList<int> iLst; QStringList sl=str.split(splitStr); int i,n=sl.size();

  for (i=0; i<n; ++i)
    { iLst.append(sl.at(i).toInt()); }

  return iLst;
}

/**************************************************************************/
QString md5Hash(const QString& filePath)
/**************************************************************************/
/*!

  \brief Constructs the MD5 hash for file \a filePath.

  \a filePath must be an absolute path.

  \return The hex encoded MD5 hash or an empty string if the hash
  could not be obtained.

*/
{
  QFile fi(filePath);
  if (!fi.open(QIODevice::ReadOnly))
    return QString();
  else
    {
      QCryptographicHash md5(QCryptographicHash::Md5);
      if (md5.addData(&fi))
        return QString(md5.result().toHex());
      else
        return QString();
    }
}

/**************************************************************************/
double meanOf(double d1,double d2)
/**************************************************************************/
/*!

  \brief Calculates and returns the average of the two values \a d1
  values \a d2.

  Only values different from \a ODV::missDOUBLE are considered.  \a
  ODV::missDOUBLE is returned if no valid input value is found.

*/
{
  if      (d1!=ODV::missDOUBLE && d2!=ODV::missDOUBLE)
    return 0.5*(d1+d2);
  else if (d1!=ODV::missDOUBLE && d2==ODV::missDOUBLE)
    return d1;
  else if (d1==ODV::missDOUBLE && d2!=ODV::missDOUBLE)
    return d2;

  return ODV::missDOUBLE;
}

/**************************************************************************/
double medianVal(double *vals,int count,double valMiss,double *dWrk,int *iWrk)
/**************************************************************************/
/*!

  \brief Calculates and returns the median of \a count values \a
  vals.

  Only values different from \a valMiss are considered.  \a valMiss is
  returned if no valid input value is found.

  \note The workspace memory at \a dWrk and \a iWrk must be large
  enough to hold \a count \c double and \c int values, respectively.
*/
{

  int i,n=0,nc; double d;
  for (i=0; i<count; ++i)
    if (vals[i]!=valMiss) { dWrk[n]=vals[i]; iWrk[n]=n; ++n; }

  if (n)
    {
      indexx(n,dWrk,iWrk); nc=(n-1)/2; d=dWrk[iWrk[nc]];
      /* odd/even n distinction */
      return (n&1) ? d : 0.5*(d+dWrk[iWrk[nc+1]]);
    }
  else return valMiss;
}

/**************************************************************************/
double myround(double val,int decim)
/**************************************************************************/
/*!
  \brief Rounds double value \a val to \a decim decimal places.

  If fabs(\a val)/10<sup>\a decim</sup> < 1, \a val is left unchanged.
*/
{
  qint64 ival,sign; double fac,faci,sval,dval,dif;

  fac=pow(10.,(double) decim); faci=1./fac; sval=val*fac;
  /*   dval=fabs(val)*faci; */
  dval=fabs(val)*fac;

  if (fabs(sval)<1.e14 && sval!=0.)
    {
      if (dval>1.)
        {
          ival=(qint64) sval; dif=fabs(sval-((double) ival));
          sign=(qint64) (sval/fabs(sval));
          if (dif>=0.5) ival+=sign;
          val=((double) ival)*faci;
        }
      else val=myround(val,decim+1);
    }
  return val;
}

/**************************************************************************/
char* mystrlwr(char *szB)
/**************************************************************************/
/*!
  \brief In-place conversion of string \a szB to lower case.
*/
{
  char *pInStr=szB;

  while (*pInStr)
    {
      *pInStr = (char)tolower(*pInStr);
      ++pInStr;
    }
  return szB;
}

/**************************************************************************/
QString referenceURL(const QString& geotracesId,const QString& prmName)
/**************************************************************************/
/*!

  \brief Constructs and returns the full reference URL for GEOTRACES ID \a
  geotracesId and parameter name \a prmName.

*/
{
  static QString refURL="http://geotraces-biblio.sedoo.fr/search?campaign=%1&param=%2";
  return refURL.arg(geotracesId).arg(prmName);
}

/**************************************************************************/
QStringList sortedNameList(const QStringList& names,bool lastNameFirstName,
                           InfoMap *piInfosByName)
/**************************************************************************/
/*!

  \brief Constructs the alphabetically sorted name list from names in
  \a names.

  The returned names are in "lastName, firstName" format if \a
  lastNameFirstName is \c true.

  If \a piInfosByName is not \c NULL and an ORCID is found for a
  given name a href element is created containing a link to the
  person's ORCID page.

  \return The  list of names (or href elements) sorted by last names.

*/
{
  const QString fmt="<a href=\"https://orcid.org/%1/\">%2</a>";
  int i,n=names.size(); QString lastFirstName,name,str,orcId;
  QMap<QString,QString> namesMap;
  for (i=0; i<n; ++i)
    {
      name=names.at(i); lastFirstName=toLastNameFirstName(name);
      str=lastNameFirstName ? lastFirstName : name;
      if (piInfosByName && piInfosByName->contains(name))
        str=fmt.arg(piInfosByName->value(name).at(0)).arg(str);
      namesMap.insert(lastFirstName.toLower(),str);
    }
  return namesMap.values();
}

/**************************************************************************/
QStringList splitString(const QString& str,QChar sepChar,const QString& textQuote)
/**************************************************************************/
/*!

  \brief Splits the string \a str into tokens using separation
  character \a sepChar.

  \a textQuote is the string used to enclose text (default is ").
  Leading and terminating \a textQuote occurences are removed. Any
  \a sepChar occurences within \a textQuote quoted text are not split.

  \return The QStringList of tokens after splitting.  The unsplit
  string is returned if \a sepChar is invalid.

*/
{
  QString lStr=str;
  if (sepChar.isNull()) return QStringList(lStr);
  if (sepChar==QChar(' ')) lStr=lStr.simplified();

  /* split the line only at sepChar occurences outside textQuote quotes */
  int i=0;     // 0-based running index in lStr
  int start=0; // 0-based start index in lStr of current token
  int k=lStr.indexOf(textQuote); // 0-based start index of next textQuote

  int n=lStr.size(),m=textQuote.size();
  QStringList sl; QString s; QChar ch; bool inQuote=false;
  while (i<n)
    {
      if (i==k)
        {
          /* textQuote found at index i */
          inQuote=!inQuote; i+=m; k=lStr.indexOf(textQuote,i);
        }
      else
        {
          if (!inQuote && lStr.at(i)==sepChar)
            {
              /* sepChar found at index i outside quotation: extract
                 token and append to sl */
              sl.append(lStr.mid(start,i-start));
              start=i+1;
            }
          ++i;
        }
    }
  if (i>start || i==(lStr.lastIndexOf(sepChar)+1))
    sl.append(lStr.mid(start,n-start));

  /* remove white space and enclosing textQuote if at least one split occured */
  if ((n=sl.size())>1)
    {
      for (i=0; i<n; ++i)
        {
          sl[i]=sl[i].trimmed();
          if (sl[i].startsWith(textQuote) && sl[i].endsWith(textQuote))
            { sl[i].chop(m); sl[i]=sl[i].mid(m); }
        }
    }

  return sl;
}

/**************************************************************************/
void stripEnclosingChars(QString& s,const QChar& startChar,const QChar& endChar)
/**************************************************************************/
/*!

  \brief Strips first and last characters of \a s if these are equal
  to \a startChar and \a endChar.

*/
{
  if (s.startsWith(startChar) && s.endsWith(endChar))
    { s.chop(1); s=s.mid(1); }
}

/**************************************************************************/
QString toLastNameFirstName(const QString& firstName,const QString& lastName,
                            const QString& separator)
/**************************************************************************/
/*!

  \brief \return The name in last name first name order using
  separator \a separator.

*/
{
  QString n=lastName;
  if (!firstName.isEmpty()) n+=separator+firstName;
  return n;
}

/**************************************************************************/
QString toLastNameFirstName(const QString& fullName,
                            const QString& separator)
/**************************************************************************/
/*!

  \brief \return The name in last name first name order using
  separator \a separator.

*/
{
  QString firstName,lastName; decomposeName(fullName,firstName,lastName);
  return toLastNameFirstName(firstName,lastName,separator);
}
