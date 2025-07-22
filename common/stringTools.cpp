/****************************************************************************
 **
 ** Copyright (C) 1993-2025 Reiner Schlitzer. All rights reserved.
 **
 ** This file is part of Ocean Data View.
 **
 ** Class implementations of:
 **                           RMessageList
 **
 ** Global function implementations:
 **                           String manipulation functions
 **
 ** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 ** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 **
 ****************************************************************************/

/*! \file
  This is the include file for string manipulation functions.
*/
#include <QtGlobal>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>

#include "common/stringTools.h"

#include <ctype.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <QChar>
#include <QDate>
#if (QT_VERSION >= QT_VERSION_CHECK(5, 10, 0))
#include <QRandomGenerator>
#endif
#include <QRegExp>
#include <QString>
#include <QStringList>

#include "fileio/file.h"
#include "common/mathhelper.h"
#include "tools/unicode.h"
#include "tools/odv.h"	// missDOUBLE
#include "tools/systemTools.h"


/***************************************************************************
 ** NEW CLASS
 ***************************************************************************/

/*!
  \class RSeparator

  \brief Provides column separator functionality.

*/

/**************************************************************************/
QString RSeparator::quotedStr(const QString& str) const
/**************************************************************************/
/*!
  \brief Wraps string \a str with double quotes if \a str contains \c sepCh.

  \return The possibly double quoted string.
*/
{
  return (str.contains(sepCh)) ? QString("\"%1\"").arg(str) : str;
}

/**************************************************************************/
QString RSeparator::validatedStr(const QString& str) const
/**************************************************************************/
/*!
  \brief Replaces any occurence of \c sepCh with \c replCh.

  \return The modified string.
*/
{
  QString s(str);
  return s.replace(sepCh,replCh);
}



/***************************************************************************
 ** NEW CLASS
 ***************************************************************************/

/*!
  \class RMessageList

  \brief Provides message logging functionality and storage.

  Maintains a string list for message records and counts warning and
  error messages.

*/

/**************************************************************************/
RMessageList::RMessageList(const QString& logFilePath)
  : logFn(logFilePath)
    /**************************************************************************/
    /*!
      \brief Creates an empty message list.
    */
{
  clear();
}

/**************************************************************************/
RMessageList::~RMessageList()
/**************************************************************************/
/*!
 */
{
  /* save the messages to log file if non-empty (override mode) */
  if (size()>0 && !logFn.isEmpty()) appendRecords(logFn,*this,true);
}

/**************************************************************************/
void RMessageList::appendErrorMsg(const QString& errMsg)
/**************************************************************************/
{
  appendMsg(errMsg,ErrorMsg);
}

/**************************************************************************/
void RMessageList::appendInfoMsg(const QString& infoMsg)
/**************************************************************************/
{
  appendMsg(infoMsg,InfoMsg);
}

/**************************************************************************/
void RMessageList::appendMsg(const QString& msg,MsgType type)
/**************************************************************************/
/*!
  \brief Appends message \a msg using message type \a type.

  Also sets the warning and error flags if \a type is \c WarningMsg or
  \c ErrorMsg.

*/
{
  static const QString prefixes[]={"","INFO:","WARNING:","ERROR:"};

  append(prefixes[type]+"\t"+msg);

  if      (type==WarningMsg) ++numWarnings;
  else if (type==ErrorMsg)   ++numErrors;
}

/**************************************************************************/
void RMessageList::appendMessagesToFile(const QString& fn)
/**************************************************************************/
/*!

  \brief Appends all messages to the end of file \a fn.

*/
{
  if (!isEmpty()) appendRecords(fn,*this);
}

/**************************************************************************/
void RMessageList::appendWarningMsg(const QString& warnMsg)
/**************************************************************************/
{
  appendMsg(warnMsg,WarningMsg);
}

/**************************************************************************/
void RMessageList::clear()
/**************************************************************************/
/*!

  \brief Clears the messages and sets warning and error counts as well
  as the prefix start index to zero.

*/
{
  QStringList::clear(); numWarnings=numErrors=prefixStartIndex=0;
}

/**************************************************************************/
int RMessageList::count(const QString& subStr,Qt::CaseSensitivity cs)
/**************************************************************************/
/*!

  \brief Counts the number of messages that contain the sub-string \a
  subStr (using case sensitivity \a cs).

  \return The determined count.

*/
{
  int i,n=size(),N=0;
  for (i=0; i<n; ++i)
    { if (at(i).contains(subStr,cs)) ++N; }

  return N;
}

/**************************************************************************/
void RMessageList::prefixLastMessages(const QString& prefix)
/**************************************************************************/
/*!

  \brief Prefixes all messages in \a msgList from 0-based index \a
  prefixStartIndex to the end with \a prefix.

  Then sets \a prefixStartIndex to 1 past the last entry.

*/
{
  int i,n=size();

  for (i=prefixStartIndex; i<n; ++i)
    (*this)[i].prepend(prefix);

  prefixStartIndex=n;
}

/**************************************************************************/
void RMessageList::setPrefixStartIndex(int idx)
/**************************************************************************/
/*!

  \brief Sets the prefix start index to \a idx, or to the 0-based index
  of the next message if \a idx is \c -1 on entry (the default).

*/
{
  if (idx==-1) idx=size();
  prefixStartIndex=idx;
}


/**************************************************************************/
DECLSPEC
QString addEnclosingChars(const QString& s,const QChar& startChar,
                          const QChar& endChar)
/**************************************************************************/
/*!

  \brief Prepends\a startChar and appends \a endChar to \a s.

  \return The modified string.

  \sa stripEnclosingChars()

*/
{
  return QString("%1%2%3").arg(startChar).arg(s).arg(endChar);
}

/**************************************************************************/
DECLSPEC
int breakLine(char *szLine,const char *sepChar,char *szToken[],int maxToken)
/**************************************************************************/
/*!
  \brief Breaks \a szLine into tokens \a *szToken[] (max. no. of
  tokens \a maxToken) using \a sepChar as separation character.

  \return number of tokens found.
*/
{
  int n=0; char *psz;

  /* immediate return if szLine is empty */
  if (!strlen(szLine)) return 0;

  /* remove trailing newline character if present */
  if ((psz=strchr(szLine,(int) '\n'))) *psz='\0';
  if ((psz=strchr(szLine,(int) '\r'))) *psz='\0';

  if (!strcmp(sepChar," "))
    {
      psz=strtok(szLine," "); if (psz) { szToken[n]=psz; ++n; }
      while (psz)
        { psz=strtok(0," "); if (psz) { szToken[n]=psz; ++n; } }
    }
  else
    {
      szToken[n]=szLine; ++n;
      while ((psz=strstr(szToken[n-1],sepChar)))
        { *psz='\0'; if (n<maxToken) { szToken[n]=psz+1; ++n; } }
    }

  return n;
}

/**************************************************************************/
DECLSPEC
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
DECLSPEC
bool cleanString(QString& str)
/**************************************************************************/
/*!
  \brief Removes ~-style control sequences from \a str.

  \return \c true if string was modified, and \c false otherwise.
*/
{
  bool modified=false,inMathMode=false; int i=0,iLen=str.size();
  QChar ch,tilde=QChar('~'),dollar=QChar('$'),hash=QChar('#'); QString s;

  while (i<iLen)
    {
      ch=str.at(i);
      if (ch==tilde)
        {
          if      (str.at(i+1)==dollar) inMathMode=true;
          else if (str.at(i+1)==hash) inMathMode=false;
          else if (str.at(i+1)==QChar('%')) s+="%0";
          i+=2; modified=true;
        }
      else
        {
          if (inMathMode && ch==QChar('m')) ch=QChar('u');
          s+=ch; ++i;
        }
    }
  str=s;

  return modified;
}

/**************************************************************************/
DECLSPEC
bool containsDateKeyWords(const QString& str)
/**************************************************************************/
/*!

  \brief Checks whether \a str contains one of the date label key
  words (case-insensitive match).

  Currently we are only searching for " since ".

  \return \c true if a date label key word is found, or \c false
  otherwise.

*/
{
  if (str.contains(" since ",Qt::CaseInsensitive)) return true;
  else                                             return false;
}

/**************************************************************************/
DECLSPEC
int containsString(const QStringList& sl,const QString& str,
                   int from,Qt::CaseSensitivity cs)
/**************************************************************************/
/*!

  \brief Determines the 0-based index in the string list \a sl that
  contains \a str, searching forward from index position \a from using
  case sensitivity \a cs.

  \return The determined index, or \c -1 if no match was found.

*/
{
  int i,n=sl.size();
  for (i=from; i<n; ++i)
    if (sl.at(i).contains(str,cs)) return i;

  return -1;
}

/**************************************************************************/
DECLSPEC
bool containsVocabCode(const QString& str)
/**************************************************************************/
/*!

  \brief Checks whether \a str contains a vocabulary code of the form
  SDN:[vocab]::[code].

  \return \c true if \a str contains a vocabulary code, and \c false
  otherwise.

*/
{
  return QRegExp("SDN:*::",Qt::CaseSensitive,QRegExp::Wildcard).indexIn(str)>-1;
}

/**************************************************************************/
DECLSPEC
bool containsURL(const QString& name,const QString& value)
/**************************************************************************/
/*!
  \brief Checks whether \a value contains a URL or DOI.

  \return \c true if \a value contains a URL or DOI, and \c false
  otherwise.

  \c true is returned in the following cases (case insensitive matches):
  - name contains \c doi
  - name == \c local_cdi_id
  - name == \c edmo_code
  - value contains \c doi:
  - value contains \c doi.
  - value contains \c http:
  - value contains \c https:
  - value contains \c ftp:

*/
{
  if (value.isEmpty()) return false;

  Qt::CaseSensitivity cs=Qt::CaseInsensitive;
  return
    name.contains("doi",cs) ||
    !name.compare("local_cdi_id",cs) ||
    !name.compare("edmo_code",cs) ||
    value.contains("doi:",cs) ||
    value.contains("doi.",cs) ||
    value.contains("http:",cs) ||
    value.contains("https:",cs) ||
    value.contains("ftp:",cs);
}

/**************************************************************************/
DECLSPEC
QString countInfoString(int count,int totalCount,const QString& suffix)
/**************************************************************************/
/*!

  \brief Returns relative count information as QString.

*/
{
  if (count==totalCount)
    return QString("All %1 %2").arg(count).arg(suffix);
  else
    return QString("%1 of %2 %3").arg(count).arg(totalCount).arg(suffix);
}

/**************************************************************************/
DECLSPEC
bool doubleVectorFromJsonArray(const QJsonArray& jsonArr,QVector<double>& dblVec,
                               const QString& errKeyWord,QString& rMsg)
/**************************************************************************/
/*!

  \brief Extracts double values from JSON array \a jsonArr and appends
  to \a dblVec.

  Appends an error message (using \a errKeyWord) to \a rMsg and
  returns \c false if an original value cannot be converted to double.

  \return \c true if successful, or \c false otherwise.

  \sa jsonArrayFromDoubleList()

*/
{
  int i,n=jsonArr.size(); double d,dMiss=-1.e10; dblVec.clear();
  for (i=0; i<n; ++i)
    {
      d=jsonArr.at(i).toDouble(dMiss);
      if (d!=dMiss)
        dblVec.append(d);
      else
        {
          rMsg+=jsonMsgEntry(QString("Invalid %1=%2.")
                             .arg(errKeyWord).arg(jsonArr.at(i).toString()));
          return false;
        }
    }
  return true;
}

/**************************************************************************/
DECLSPEC
void extractRHS(char *rhs,char* Text)
/**************************************************************************/
/*!
  \brief Copies the string to the right of \c = in \a Text to \a rhs.
  If \c = is not found in \a Text, rhs remains \a empty
*/
{
  char *psz;

  strcpy(rhs,""); psz=strchr(Text,'=');
  if (psz!=0) strcpy(rhs,psz+1);
  rmNL(rhs);
}

/**************************************************************************/
DECLSPEC
QString filePathToLocalUrl(const QString& filePath)
/**************************************************************************/
{
#if defined Q_OS_WIN
  return QString("file:///")+filePath;
#else
  return QString("file://")+filePath;
#endif
}

/**************************************************************************/
DECLSPEC
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
DECLSPEC
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
DECLSPEC
QString formattedInt(int i)
/**************************************************************************/
/*!

  \brief Constructs and returns the string representation of \a i with
  commas as thousands separator.

*/
{
  QString s=QString::number(i);
  for (int i=s.size()-3; i>0; i-=3) { s.insert(i,","); }
  return s;
}

/**************************************************************************/
DECLSPEC
QString formattedNumber(double d,int decCount,bool doChopTrailingZeros,
                        bool clearMissDouble)
/**************************************************************************/
/*!

  \brief Constructs and returns a string representation of \a d.

  An empty string is returned if \a d is equal to \c ODV::missDOUBLE
  and \a clearMissDouble is true (the default).

  \a decCount is the number of significant digits to be used.

*/
{
  if (d==ODV::missDOUBLE && clearMissDouble) return QString();

  double a=fabs(d),b=floor(a);
  if ((a-b)<1.e-8 && doChopTrailingZeros) decCount=0;
  QString n=(decCount>0 && a!=0. && (a<1.e-5 || a>1.e6)) ?
    QString::number(d,'g',decCount+3) : QString("%1").arg(d,0,'f',decCount);
  if (doChopTrailingZeros) chopTrailingZeros(n);
  return n;
}

/**************************************************************************/
quint64 hashFor(const QString& string)
/**************************************************************************/
/*!

  \brief Returns the 64 bit Fowler/Noll/Vo FNV-1a hash for string \a string.

*/
{
  return hashFor(string.toUtf8().constData());
}

/**************************************************************************/
quint64 hashFor(const char *str)
/**************************************************************************/
/*!

  \brief Returns the 64 bit Fowler/Noll/Vo FNV-1a hash for string \a str.

  This code is based on C code downloaded from \c
  http://www.isthe.com/chongo/tech/comp/fnv/index.html#FNV-source on
  2014-03-28.

*/
{
#define FNV1A_64_INIT ((quint64)0xcbf29ce484222325ULL)
#define FNV_64_PRIME ((quint64)0x100000001b3ULL)

  quint64 hash=FNV1A_64_INIT;
  unsigned char *s=(unsigned char*)str;	/* unsigned string */

  /* loop over all bytes of the string */
  while (*s)
    {
      /* xor the bottom with the current byte */
      hash^=(quint64)*s++;

      /* multiply by the 64 bit FNV magic prime mod 2^64 */
      hash*=FNV_64_PRIME;
    }

  /* return the hash value */
  return hash;
}

/**************************************************************************/
DECLSPEC
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
DECLSPEC
QList<int> indexListOfStr(const QStringList& sl,const QString& str)
/**************************************************************************/
/*!

  \brief Determines the list of 0-based indexes of all occurrences of \a
  str in the string list \a sl.

  \return The constructed index list. The list is empty if no match
  was found.

*/
{
  int idx,strtIdx=0; QList<int> idxLst;
  do
    {
      idx=sl.indexOf(str,strtIdx);
      if (idx>-1) { idxLst.append(idx); strtIdx=idx+1; }
    }
  while(idx>-1);

  return idxLst;
}

/**************************************************************************/
DECLSPEC
int indexOfStr(const QStringList& sl,const QString& str,
               int from,Qt::CaseSensitivity cs)
/**************************************************************************/
/*!

  \brief Determines the 0-based index of the first occurrence of \a
  str in the string list \a sl, searching forward from index position
  \a from using case sensitivity \a cs.

  \return The determined index, or \c -1 if no match was found.

*/
{
  int i,n=sl.size();
  for (i=from; i<n; ++i)
    if (!str.compare(sl.at(i),cs)) return i;

  return -1;
}

/**************************************************************************/
DECLSPEC
int indexOfContainsStr(const QStringList& sl,const QString& str,
                       int from,Qt::CaseSensitivity cs)
/**************************************************************************/
/*!

  \brief Determines the 0-based index of the first entry in \a sl containing
  \a str, searching forward from index position \a from using case
  sensitivity \a cs.

  \return The determined index, or \c -1 if no match was found.

*/
{
  int i,n=sl.size();
  for (i=from; i<n; ++i)
    if (sl.at(i).contains(str,cs)) return i;

  return -1;
}

/**************************************************************************/
DECLSPEC
int indexOfEndsWithStr(const QStringList& sl,const QString& str,
                       int from,Qt::CaseSensitivity cs)
/**************************************************************************/
/*!

  \brief Determines the 0-based index of the first entry in \a sl ending
  with \a str, searching forward from index position \a from using case
  sensitivity \a cs.

  \return The determined index, or \c -1 if no match was found.

*/
{
  int i,n=sl.size();
  for (i=from; i<n; ++i)
    if (sl.at(i).endsWith(str,cs)) return i;

  return -1;
}

/**************************************************************************/
DECLSPEC
int indexOfStartsWithStr(const QStringList& sl,const QString& str,
                         int from,Qt::CaseSensitivity cs)
/**************************************************************************/
/*!

  \brief Determines the 0-based index of the first entry in \a sl starting
  with \a str, searching forward from index position \a from using case
  sensitivity \a cs.

  \return The determined index, or \c -1 if no match was found.

*/
{
  int i,n=sl.size();
  for (i=from; i<n; ++i)
    if (sl.at(i).startsWith(str,cs)) return i;

  return -1;
}

/**************************************************************************/
DECLSPEC
bool intListFromJsonArray(const QJsonArray& jsonArr,QList<int>& intList,
                          int addOffset,int lowerLimit,int upperLimit,
                          const QString& errKeyWord,QString& rMsg)
/**************************************************************************/
/*!

  \brief Extracts integer values from JSON array \a jsonArr, adds \a
  addOffset to the extracted values and appends to \a intList.

  Appends an error message (using \a errKeyWord) to \a rMsg and returns
  \c false if an original value outside the [\a lowerLimit,\a
  upperLimit] interval is encountered. If \a upperLimit is \c -1 on
  entry, no upper limit check is performed.

  \return \c true if successful, or \c false otherwise.

  \sa jsonArrayFromIntList()

*/
{
  int i,id,n=jsonArr.size(); intList.clear();
  for (i=0; i<n; ++i)
    {
      id=jsonArr.at(i).toInt(-1);
      if (id>=lowerLimit && (upperLimit==-1 || id<=upperLimit))
        intList.append(id+addOffset);
      else
        {
          rMsg+=jsonMsgEntry(QString("Invalid %1=%2.").arg(errKeyWord).arg(id));
          return false;
        }
    }
  return true;
}

/**************************************************************************/
DECLSPEC
QList<int> intListFromStr(const QString& str,const QString& splitStr)
/**************************************************************************/
/*!

  \brief Splits using \a splitStr and extracts integer values from
  string \a str.

  \return The list of extracted integer values.

  \sa joinedIntList()

*/
{
  QList<int> iLst; QStringList sl=str.split(splitStr); int i,n=sl.size();

  for (i=0; i<n; ++i)
    { iLst.append(sl.at(i).toInt()); }

  return iLst;
}

/**************************************************************************/
DECLSPEC
bool isLetter(int asciiCode)
/**************************************************************************/
/*!

  \brief Checks whether \a asciiCode represents a letter.

  \return \c true if \a asciiCode represents a letter, or \c false
  otherwise.

  \sa isNumber()

*/
{
  return (isLowerCaseLetter(asciiCode) || isUpperCaseLetter(asciiCode));
}

/**************************************************************************/
DECLSPEC
bool isLetterOrNumber(int asciiCode)
/**************************************************************************/
/*!

  \brief Checks whether \a asciiCode represents a letter or a number.

  \return \c true if \a asciiCode represents a letter or a number, or
  \c false otherwise.

  \sa isNumber()

*/
{
  return (isLetter(asciiCode) || isNumber(asciiCode));
}

/**************************************************************************/
DECLSPEC
bool isLowerCaseLetter(int asciiCode)
/**************************************************************************/
/*!

  \brief Checks whether \a asciiCode represents a lower case letter.

  \return \c true if \a asciiCode represents a lower case letter, or
  \c false otherwise.

  \sa isNumber()

*/
{
  return (asciiCode>96 && asciiCode<123);
}

/**************************************************************************/
DECLSPEC
bool isNumber(int asciiCode)
/**************************************************************************/
/*!

  \brief Checks whether \a asciiCode represents a number.

  \return \c true if \a asciiCode represents a number, or \c false
  otherwise.

  \sa isLetter()

*/
{
  return (asciiCode>47 && asciiCode<58);
}

/**************************************************************************/
DECLSPEC
bool isUpperCaseLetter(int asciiCode)
/**************************************************************************/
/*!

  \brief Checks whether \a asciiCode represents an upper case letter.

  \return \c true if \a asciiCode represents an upper case letter, or
  \c false otherwise.

  \sa isLetter()

*/
{
  return (asciiCode>64 && asciiCode<91);
}

/**************************************************************************/
DECLSPEC
QString joinedFormattedIntList(const QList<int>& lst,const QString &separator,int addOffset)
/**************************************************************************/
/*!

  \brief Joins all values in \a lst (value of \a addOffset added)
  using separator \a separator.

  \return The constructed joined string.

  \sa intListFromStr()

*/
{
  if (lst.isEmpty()) return QString();

  int i,n=lst.size(); const QString quote="\"";
  QString s=quote+formattedInt(lst.at(0)+addOffset)+quote;
  for (i=1; i<n; ++i)
    { s+=(separator+quote+formattedInt(lst.at(i)+addOffset)+quote); }
  return s;
}

/**************************************************************************/
DECLSPEC
QString joinedIntList(const QList<int>& lst,const QString &separator,int addOffset)
/**************************************************************************/
/*!

  \brief Joins all values in \a lst (value of \a addOffset added)
  using separator \a separator.

  \return The constructed joined string.

  \sa intListFromStr()

*/
{
  if (lst.isEmpty()) return QString();

  int i,n=lst.size();
  QString s=QString::number(lst.at(0)+addOffset);
  for (i=1; i<n; ++i)
    { s+=(separator+QString::number(lst.at(i)+addOffset)); }
  return s;
}

/**************************************************************************/
DECLSPEC
void jsonAppendToArray(QString& arr,const QString& item)
/**************************************************************************/
/*!

  \brief Appends item \a item to JSON array \a arr.

*/
{
  if (item.isEmpty()) return;
  if (!arr.isEmpty()) arr+=",";
  arr+=item;
}

/**************************************************************************/
DECLSPEC
QString jsonArrayFromDoubleList(const QList<double>& lst)
/**************************************************************************/
/*!

  \brief Constructs JSON array from values in \a lst.

  \return The constructed JSON array.

  \sa doubleVectorFromJsonArray()

*/
{
  int i,n=lst.size(); QString s="[";
  for (i=0; i<n; ++i)
    {
      if (i>0) s+=",";
      s+=QString::number(lst.at(i));
    }
  return s+"]";
}

/**************************************************************************/
DECLSPEC
QString jsonArrayFromDoubleVals(double *dVals,int count)
/**************************************************************************/
/*!

  \brief Constructs JSON array from values in \a dVals.

  \return The constructed JSON array.

*/
{
  int i; QString s="[";
  for (i=0; i<count; ++i)
    {
      if (i>0) s+=",";
      s+=QString::number(dVals[i]);
    }
  return s+"]";
}

/**************************************************************************/
DECLSPEC
QString jsonArrayFromIntList(const QList<int>& lst,int addOffset,
                             bool asFormattedInt)
/**************************************************************************/
/*!

  \brief Constructs JSON array from values in \a lst.

  \return The constructed JSON array.

  \sa intListFromJsonArray()

*/
{
  QString s=asFormattedInt ?
    joinedFormattedIntList(lst,",",addOffset) : joinedIntList(lst,",",addOffset);
  return "["+s+"]";
}

/**************************************************************************/
DECLSPEC
QString jsonArrayFromIntVals(int *iVals,int count,int addOffset)
/**************************************************************************/
/*!

  \brief Constructs JSON array from values in \a iVals.

  \return The constructed JSON array.

*/
{
  int i; QString s="[";
  for (i=0; i<count; ++i)
    {
      if (i>0) s+=",";
      s+=QString::number(iVals[i]+addOffset);
    }
  return s+"]";
}

/**************************************************************************/
DECLSPEC
QString jsonArrayFromStrList(const QStringList& lst)
/**************************************************************************/
/*!

  \brief Constructs JSON array string from values in \a lst.

  \return The constructed JSON array string.

*/
{
  // int i,n=lst.size(); QString s="[";
  // for (i=0; i<n; ++i)
  //   {
  //     if (i>0) s+=",";
  //     s+=QString("\"%1\"").arg(jsonValidatedStr(lst.at(i)));
  //   }
  // return s+"]";
  int i,n=lst.size(); QJsonArray jsonArr; QJsonDocument jsonDoc;
  for (i=0; i<n; ++i) jsonArr.append(QJsonValue(lst.at(i)));
  jsonDoc.setArray(jsonArr);
  return QString(jsonDoc.toJson(QJsonDocument::Compact));
}

/**************************************************************************/
DECLSPEC
QString jsonDoubleArrayEntry(const QString& name,const QList<double>& lst)
/**************************************************************************/
/*!

  \brief Constructs and returns a JSON double array entry with name \a name
  and array containing values in \a lst.

*/
{
  return QString("\"%1\":%2").arg(name).arg(jsonArrayFromDoubleList(lst));
}

/**************************************************************************/
DECLSPEC
QString jsonDoubleEntry(const QString& name,double value)
/**************************************************************************/
/*!

  \brief Constructs and returns a JSON double entry with name \a name
  and value \a value.

*/
{
  return QString("\"%1\":%2").arg(name).arg(value);
}

/**************************************************************************/
DECLSPEC
QString jsonFormattedIntArrayEntry(const QString& name,
                                   const QList<int>& lst,int addOffset)
/**************************************************************************/
/*!

  \brief Constructs and returns a JSON formateed integer array entry with
  name \a name and array containing integer values in \a lst.

*/
{
  return QString("\"%1\":%2").arg(name).arg(jsonArrayFromIntList(lst,addOffset,true));
}

/**************************************************************************/
DECLSPEC
QString jsonIntArrayEntry(const QString& name,const QList<int>& lst,int addOffset)
/**************************************************************************/
/*!

  \brief Constructs and returns a JSON integer array entry with name \a name
  and array containing values in \a lst.

*/
{
  return QString("\"%1\":%2").arg(name).arg(jsonArrayFromIntList(lst,addOffset));
}

/**************************************************************************/
DECLSPEC
QString jsonIntEntry(const QString& name,int value)
/**************************************************************************/
/*!

  \brief Constructs and returns a JSON int entry with name \a name
  and value \a value.

*/
{
  return QString("\"%1\":%2").arg(name).arg(value);
}

/**************************************************************************/
DECLSPEC
QString jsonMsgEntry(const QString& msgValue)
/**************************************************************************/
/*!

  \brief Constructs and returns a JSON string entry with name \a msg
  and value \a msgValue.

*/
{
  return jsonStrEntry("msg",msgValue);
}

/**************************************************************************/
DECLSPEC
QString jsonPolygon(const QString& id,const QList<int> &xC,const QList<int> &yC)
/**************************************************************************/
/*!

  \brief Constructs a JSON-formatted polygon object containing item \c
  id with value \a id, and items \c x_coords and \c y_coords
  containing the polygon coordinates.

  \return The constructed JSON string.

*/
{
  return QString("{\"id\":\"%1\",\"x_coords\":%2,\"y_coords\":%3}")
    .arg(id).arg(jsonArrayFromIntList(xC)).arg(jsonArrayFromIntList(yC));
}

/**************************************************************************/
DECLSPEC
QString jsonStrArrayEntry(const QString& name,const QStringList& lst)
/**************************************************************************/
/*!

  \brief Constructs and returns a JSON string array entry with name \a name
  and array containing values in \a lst.

*/
{
  return QString("\"%1\":%2").arg(name).arg(jsonArrayFromStrList(lst));
}

/**************************************************************************/
DECLSPEC
QString jsonStrEntry(const QString& name,const QString& value)
/**************************************************************************/
/*!

  \brief Constructs and returns a JSON string entry with name \a name
  and value \a value.

*/
{
  return QString("\"%1\":\"%2\"").arg(name).arg(jsonValidatedStr(value));
  // QJsonObject jsonObj; QJsonDocument jsonDoc;
  // jsonObj.insert(name,QJsonValue(value)); jsonDoc.setObject(jsonObj);
  // return QString(jsonDoc.toJson(QJsonDocument::Compact));
}

/**************************************************************************/
DECLSPEC
QString jsonValidatedStr(const QString& str)
/**************************************************************************/
/*!

  \brief Validates string \a str by replacing TABs with single spaces
  and escaping double quotes.

*/
{
  QString s=str; s.replace("\\","\\\\"); s.replace("\"","\\\"");
  s.replace("\t"," "); s.replace("\n"," "); s.replace("\r"," ");
  return s;
}

/**************************************************************************/
DECLSPEC
QString leftPaddedStr(const QString& str,int width)
/**************************************************************************/
/*!

  \brief Prepends spaces to \a str until its length is \a width.

  \return The modified string.

*/
{
  static QString spStr="                                                     ";
  return spStr.left(qMax(0,width-str.size()))+str;
}

/**************************************************************************/
DECLSPEC
QString logMessage(const QString& action,const QString& msg,const QString& user)
/**************************************************************************/
/*!

  \brief Constructs and returns ODV-style log message.

  The log message consists of current date/time and user/host
  information concatenated with \a action and \a msg. Items are TAB
  separated.

  Uses the system user name if \a user is empty on entry (the default).

*/
{
  QString usr=(user.isEmpty()) ? userName() : user;
  return QString("%1\t%2@%3\t%4\t%5")
    .arg(QDateTime::currentDateTime().toString(Qt::ISODate))
    .arg(usr).arg(hostName()).arg(action).arg(msg);
}

/**************************************************************************/
DECLSPEC
QString mediaTypeFromFileExt(const QString& fileExt)
/**************************************************************************/
/*!

  \brief Returns the media type for file extension \a fileExt, or an
  empty string if the media type can not be determined.

*/
{
  QString ext=fileExt;
  if (ext.isEmpty()) return QString();

  if      (ext=="js")   ext="javascript";
  else if (ext=="htm")  ext="html";
  else if (ext=="jpg")  ext="jpeg";
  else if (ext=="txt")  ext="plain";
  else if (ext=="xgob") ext="xml";

  QString prefix="application/";
  if (ext=="css" || ext=="html" || ext=="xml" || ext=="csv" || ext=="plain")
    prefix="text/";
  else if (ext=="gif" || ext=="jpeg" || ext=="png" || ext=="tif")
    prefix="image/";

  return prefix+ext;
}

/**************************************************************************/
DECLSPEC
QString mediaTypeFromFilePath(const QString& filePath)
/**************************************************************************/
/*!

  \brief Returns the media type for file \a filePath, or an empty
  string if the media type can not be determined.

*/
{
  QString ext=QFileInfo(filePath).suffix().toLower();
  return mediaTypeFromFileExt(ext);
}

/**************************************************************************/
DECLSPEC
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
DECLSPEC
QString percentStr(int nPart,int nAll)
/**************************************************************************/
/*!
  \brief Returns the percentage value of \a nPart in \a nAll as a
  string, or "-" if \a nPart or \a nAll or both are zero.
*/
{
  return (nPart && nAll) ?
    QString::number((int) (nPart*100./nAll)) : QString("-");
}

/**************************************************************************/
DECLSPEC
QString powerTenLabel(int exponent)
/**************************************************************************/
/*!
  \brief Returns the label representing the "ten to the power \a
  exponent" value.
*/
{
  QString s("10"),e=QString::number(exponent);
  int i,n=e.size();

  for (i=0; i<n; ++i)
    s+=QString("~^%1").arg(e.at(i));

  return s;
}

/**************************************************************************/
DECLSPEC
QString powerTenLabel(const QString& num)
/**************************************************************************/
/*!

  \brief Converts \a num to "power of ten" representation if exponent
  format (e.g., 1.5e-09) is detected.

  \return The converted string, if exponent format (e.g., 1.5e-09) is
  detected, or the original string otherwise.

*/
{
  /* try to find the 'e', immediate return if unsuccessful */
  int i=num.indexOf('e',0,Qt::CaseInsensitive);
  if (i==-1) return num;

  /* try to extract the exponent, immediate return if unsuccessful */
  bool ok; int e=num.mid(i+1).toInt(&ok);
  if (!ok) return num;

  /* construct and return the "power of ten" representation */
  return num.left(i)+UCC_MIDDLEDOT+powerTenLabel(e);
}

/**************************************************************************/
DECLSPEC
QString pureLocalCdiId(const QString& lCdiId,int *version)
/**************************************************************************/
/*!

  \brief Extracts the pure local CDI Id from \a lCdiId by removing any
  trailing version string.

  \return The extracted pure local CDI Id.

*/
{
  int i=lCdiId.lastIndexOf("/v",-1,Qt::CaseInsensitive),v; bool ok;
  if (version!=NULL)
    {
      *version=-1;
      if (i>-1) { v=lCdiId.mid(i+2).toInt(&ok); if (ok) *version=v; }
    }
  return lCdiId.left(i);
}

/**************************************************************************/
DECLSPEC
QString	randomString()
/**************************************************************************/
/*!

  \brief Constructs and returns the hex representation of a random
  unsigned 32bit integer.

*/
{
#if (QT_VERSION >= QT_VERSION_CHECK(5, 10, 0))
  return QString("%1").arg(QRandomGenerator::global()->generate(),0,16);
#else
  qsrand((uint) QTime::currentTime().msec());
  return QString("%1").arg(qrand(),0,16);
#endif
}

/**************************************************************************/
DECLSPEC
double readValue(char* szB,int len,double missVal)
/**************************************************************************/
/*!
  \brief Reads and returns real number from the first len characters
  of \a szB.

  \a missVal is returned if \a szB can not be interpreted as number or
  if it is empty.
*/
{
  float v=missVal; char szA[256];
  memcpy(szA,szB,len); *(szA+len)='\0';
  return (sscanf(szA,"%g",&v)) ? (double) v:missVal;
}

/**************************************************************************/
DECLSPEC
void replaceInString(char* szStr,const char *szPre,const char *szPost,int maxlen)
/**************************************************************************/
/*!
  \brief Replaces in \a szStr all occurences of \a szPre with \a
  szPost. \a maxlen is maximal length of \a szStr.
*/
{
  int l=0,dl,l0=(int) strlen(szPre),lp=(int) strlen(szPost);
  char *psz,*ps,*pS;

  /* if szPre is not in szStr: return immediately */
  if (!strstr(szStr,szPre)) return;

  /* there is at least one occurance of szPre: process */
  if ((psz=(char*) malloc(maxlen)))
    {
      strcpy(psz,szStr); strcpy(szStr,""); l=0; pS=psz;
      while ((ps=strstr(pS,szPre)))
        {
          *ps='\0'; dl=(int) strlen(pS)+lp;
          if ((l+dl)<maxlen)
            { strcat(szStr,pS); strcat(szStr,szPost); l+=dl; }
          pS=ps+l0;
        }
      dl=(int) strlen(pS); if (dl && (l+dl)<maxlen) strcat(szStr,pS);
      free(psz);
    }
}

/**************************************************************************/
DECLSPEC
void rmNL(char* szL)
/**************************************************************************/
/*!
  \brief Removes newline character in \a szL.
*/
{
  char *psz=strchr(szL,(int) '\n');
  if (psz!=0) *psz='\0';
  psz=strchr(szL,(int) '\r');
  if (psz!=0) *psz='\0';
}

/**************************************************************************/
DECLSPEC
QStringList splitString(QString &str,QChar sepChar,const QString& textQuote)
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
  if (sepChar.isNull()) return QStringList(str);
  if (sepChar==QChar(' ')) str=str.simplified();

  /* perform simple split if textQuote is empty */
  if (textQuote.isEmpty()) return splitStringSimple(str,sepChar);

  /* split the line only at sepChar occurences outside textQuote quotes */
  int i=0;     // 0-based running index in str
  int start=0; // 0-based start index in str of current token
  int k=str.indexOf(textQuote); // 0-based start index of next textQuote

  int n=str.size(),m=textQuote.size();
  QStringList sl; QString s; bool inQuote=false;
  while (i<n)
    {
      if (i==k)
        {
          /* textQuote found at index i */
          inQuote=!inQuote; i+=m; k=str.indexOf(textQuote,i);
        }
      else
        {
          if (!inQuote && str.at(i)==sepChar)
            {
              /* sepChar found at index i outside quotation: extract
                 token and append to sl */
              sl.append(str.mid(start,i-start));
              start=i+1;
            }
          ++i;
        }
    }
  if (i>start || i==(str.lastIndexOf(sepChar)+1))
    sl.append(str.mid(start,n-start));

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
DECLSPEC
QStringList splitStringSimple(QString &str,QChar sepChar)
/**************************************************************************/
/*!

  \brief Splits the string \a str into tokens using separation
  character \a sepChar.

  \return The QStringList of tokens after splitting.

*/
{
  if (sepChar.isNull()) return QStringList(str);

  QStringList sl=str.split(sepChar); int i,n=sl.size();
  for (i=0; i<n; ++i) { sl[i]=sl.at(i).trimmed(); }
  return sl;
}

/**************************************************************************/
DECLSPEC
void sprintNumber(char *szNum,double val,int len,int dec)
/**************************************************************************/
/*!
  \brief Provides formatted number of value \a val.

  Rounds \a val to \a dec decimal places and formats the rounded value
  as a string of length \a len with \a dec significant digits
  (right-justified, returned in \a szNum).

  If the value does not fit into a string of length \a len, \a szNum
  will still be of length \a len with the right-most characters being
  "+++". If \a len==-1, the rounded value will be returned in \a szNum
  as left-justified number.

  Note: \a szNum must be at least \a MAXNUMLEN characters long.
*/
{
  const int MAXNUMLEN=30,MAXNUMDEC=8;
  int i,il=(len>0)?len:MAXNUMLEN,ieb,iet,idp=MAXNUMLEN-MAXNUMDEC;
  char szB[MAXNUMLEN+1],szFmt[11];
  double uLim=pow(10.,(double) (il-dec-2)),aVal=fabs(val);

  /* set the format string and initialize szNum to blanks */
  sprintf(szFmt,"%%%d.%df",MAXNUMLEN,MAXNUMDEC);
  for (i=0; i<MAXNUMLEN; ++i) *(szNum+i)=' ';
  *(szNum+il)='\0';

  /* now format the numerical values */
  if (val!=ODV::missDOUBLE && aVal<uLim)
    {
      sprintf(szB,szFmt,myround(val,dec));
      iet=(dec)?idp+dec:idp-1;
      if (len>0) { ieb=iet-il; memcpy(szNum,szB+ieb,il); }
      else       { *(szB+iet)='\0'; strcpy(szNum,tStrip(szB)); }
    }
  else if (val!=ODV::missDOUBLE && aVal>=uLim)
    {
      memcpy((szNum+il-3),"+++",3);
    }
  else if (val==ODV::missDOUBLE && len==-1)
    {
      strcpy(szNum,"no data");
    }
}

/**************************************************************************/
DECLSPEC
void stripEnclosingChars(QString& s,const QChar& startChar,const QChar& endChar)
/**************************************************************************/
/*!

  \brief Strips first and last characters of \a s if these are equal
  to \a startChar and \a endChar.

  Also replaces all double-quotes '""' to single quotes '"'.

  \sa addEnclosingChars()

*/
{
  QString l=s.trimmed();
  if (l.startsWith(startChar) && l.endsWith(endChar))
    { s=l; s.chop(1); s=s.mid(1); s.replace("\"\"","\""); }
}

/**************************************************************************/
DECLSPEC
QString substitutedString(const QString& t,const QHash<QString,QString>& defs)
/**************************************************************************/
/*!

  \brief Performs in \a t key to value replacements of all entries in
  \a defs and returns the modified string.

*/
{
  QString s=t; QHash<QString,QString>::ConstIterator it=defs.constBegin();
  while (it!=defs.constEnd()) { s.replace(it.key(),it.value()); ++it; }
  return s;
}

/**************************************************************************/
DECLSPEC
QString subString(const QString& str,int startIdx,int endIdx)
/**************************************************************************/
/*!

  \brief Extracts and returns the substring from \a str starting at
  zero-based index \a startIdx and ending at index \a endIdx.

  \return The extracted substring.
*/
{
  return str.mid(startIdx,endIdx-startIdx+1);
}

/**************************************************************************/
DECLSPEC
double toDouble(const QString& t,const double dflt)
/**************************************************************************/
/*!
  \brief Converts the string \a t into a double

  \return The converted double value or \a dflt if the conversion failed.
*/
{
  bool ok; double d=t.toDouble(&ok);
  return (!ok || !qIsFinite(d)) ? dflt:d;
}

/**************************************************************************/
DECLSPEC
QList<double> toDoubleList(const QStringList& sl,const double dflt)
/**************************************************************************/
/*!
  \brief Converts the strings in \a sl into double values.

  \a dflt is used if a conversion fails.

  \return The list of converted double values.
*/
{
  QList<double> dl; bool ok; double d; int i,n=sl.size();
  for (i=0; i<n; ++i)
    {
      d=sl.at(i).toDouble(&ok);
      if (!ok || !qIsFinite(d)) d=dflt;
      dl.append(d);
    }

  return dl;
}

/**************************************************************************/
DECLSPEC
QList<int> toIntList(const QStringList& sl,const int dflt)
/**************************************************************************/
/*!
  \brief Converts the strings in \a sl into integer values.

  \a dflt is used if a conversion fails.

  \return The list of converted integer values.
*/
{
  QList<int> il; bool ok; int iVal; int i,n=sl.size();
  for (i=0; i<n; ++i)
    {
      iVal=sl.at(i).toInt(&ok); if (!ok) iVal=dflt;
      il.append(iVal);
    }

  return il;
}

/**************************************************************************/
DECLSPEC
QChar toQChar(QChar ch,int fontType)
/**************************************************************************/
/*!
  \brief Returns Unicode QChar for character \a ch in normal (\a
  fontType==0) or Symbol font (\a fontType==1).
*/
{
  static quint8 gr[255]={
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x30,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,//50
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x91,0x92,0xa7,0x94,0x95,0xa6,
    0x93,0x97,0x99,0xd1,0x9a,0x9b,0x9c,0x9d,0x9f,0xa0,
    0x98,0xa1,0xa3,0xa4,0xa5,0xdb,0xa9,0x9e,0xa8,0x96,
    0x00,0x00,0x00,0x00,0x00,0x00,0xb1,0xb2,0xc7,0xb4,//100
    0xb5,0xd5,0xb3,0xb7,0xb9,0xc6,0xba,0xbb,0xbc,0xbd,
    0xbf,0xc0,0xb8,0xc1,0xc3,0xc4,0xc5,0xd6,0xc9,0xbe,
    0xc8,0xb6,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,//150
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,//200
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,//250
    0x00,0x00,0x00,0x00,0x00};
  static quint8 p[255]={
    0x03,0x03,0x03,0x03,0x03,0x03,0x03,0x03,0x03,0x03,
    0x03,0x03,0x03,0x03,0x03,0x03,0x03,0x03,0x03,0x03,
    0x03,0x03,0x03,0x03,0x03,0x03,0x03,0x03,0x03,0x03,
    0x03,0x03,0x03,0x03,0x03,0x03,0x20,0x03,0x03,0x03,
    0x03,0x03,0x03,0x03,0x03,0x03,0x03,0x03,0x03,0x03,//50
    0x03,0x03,0x03,0x03,0x03,0x03,0x03,0x03,0x03,0x03,
    0x03,0x03,0x03,0x03,0x03,0x03,0x03,0x03,0x03,0x03,
    0x03,0x03,0x03,0x03,0x03,0x03,0x03,0x03,0x03,0x03,
    0x03,0x03,0x03,0x03,0x03,0x03,0x03,0x03,0x03,0x03,
    0x03,0x03,0x03,0x03,0x03,0x03,0x03,0x03,0x03,0x03,//100
    0x03,0x03,0x03,0x03,0x03,0x03,0x03,0x03,0x03,0x03,
    0x03,0x03,0x03,0x03,0x03,0x03,0x03,0x03,0x03,0x03,
    0x03,0x03,0x03,0x03,0x03,0x03,0x03,0x03,0x03,0x03,
    0x03,0x03,0x03,0x03,0x03,0x03,0x03,0x03,0x03,0x03,
    0x03,0x03,0x03,0x03,0x03,0x03,0x03,0x03,0x03,0x03,//150
    0x03,0x03,0x03,0x03,0x03,0x03,0x03,0x03,0x03,0x03,
    0x03,0x03,0x03,0x03,0x03,0x03,0x03,0x03,0x03,0x03,
    0x03,0x03,0x03,0x03,0x03,0x03,0x03,0x03,0x03,0x03,
    0x03,0x03,0x03,0x03,0x03,0x03,0x03,0x03,0x03,0x03,
    0x03,0x03,0x03,0x03,0x03,0x03,0x03,0x03,0x03,0x03,//200
    0x03,0x03,0x03,0x03,0x03,0x03,0x03,0x03,0x03,0x03,
    0x03,0x03,0x03,0x03,0x03,0x03,0x03,0x03,0x03,0x03,
    0x03,0x03,0x03,0x03,0x03,0x03,0x03,0x03,0x03,0x03,
    0x03,0x03,0x03,0x03,0x03,0x03,0x03,0x03,0x03,0x03,
    0x03,0x03,0x03,0x03,0x03,0x03,0x03,0x03,0x03,0x03,//250
    0x03,0x03,0x03,0x03,0x03};

  QChar qc(ch); quint8 b;

#if !defined NO_XFT
  int ia=(int) ch.toLatin1()-1;
  if (fontType==1 && ia>-1 && (b=gr[ia])>0) { qc=QChar(b,p[ia]); }
#endif

  return qc;
}

/**************************************************************************/
DECLSPEC
QString toQString(const QString& t)
/**************************************************************************/
/*!
  \brief Converts ~-style formatted text in \a t to Unicode and
  returns as QString.
*/
{
  int i=0,j,fT=0,len=t.length(),code; bool ok,isTilde,isHex; QString s;

  while (i<len)
    {
      isTilde=t.at(i)=='~';
      if      (isTilde && t.at(i+1)=='#')
        /* switch to normal font */
        { fT=0; i+=2; }
      else if (isTilde && t.at(i+1)=='$')
        /* switch to Symbol font */
        { fT=1; i+=2; }
      else if (isTilde && t.at(i+1)=='%')
        /* produce permille character */
        { s+=UCC_PERMILLE; i+=2; }
      else if (isTilde && (t.at(i+1)=='_' || t.at(i+1)=='^'))
        /* produce subscript or superscript character */
        { i+=2; s+=toQChar(t.at(i),fT); ++i; }
      // else if (isTilde && t.at(i+1)=='{' && (j=t.indexOf('}',i+2))>-1)
      // 	/* produce Unicode character */
      // 	{ code=t.mid(i+2,j-i-2).toInt(&ok,16); i=j+1; if (ok) s+=QChar(code); }
      else if (t.at(i)=='&' && t.at(i+1)=='#' && (j=t.indexOf(';',i+2))>-1)
        /* produce Unicode character */
        {
          isHex=t.at(i+2)=='x'; if (isHex) ++i;
          code=t.mid(i+2,j-i-2).toInt(&ok,isHex ? 16 : 10);
          i=j+1; if (ok) s+=QChar(code);
        }
      else
        /* produce normal character */
        { s+=toQChar(t.at(i),fT); ++i; }
    }

  return s;
}

/**************************************************************************/
DECLSPEC
QString toSubOrSuperscript(const QString& s,const QString& prefix)
/**************************************************************************/
/*!
  \brief Converts string \a s to sub or superscript (depending on \a
  prefix) and returns as QString.
*/
{
  QString t,w=s; w.remove("~^"); w.remove("~_");
  QChar c; int i=0,n=w.length();

  while (i<n)
    {
      c=w.at(i);
      if (c==QChar('~')) { t+=c; ++i; t+=w.at(i); }
      else               { t+=prefix+c; }
      ++i;
    }

  return t;
}

/**************************************************************************/
DECLSPEC
char* tStrip(char* Text)
/**************************************************************************/
/*!
  \brief Removes leading and trailing blanks from string \a Text.

  If \a Text represents a number with a decimal point, trailing 0s are
  removed as well.
*/
{
  int il,i,is=0; bool have_start=false,is_number=true; char *pszEOS,*pszPoint=0;

  il=(int) strlen(Text); if (il==0) return Text;

  for ( i=0; i<il; ++i)
    {
      if (Text[i]>32 && Text[i]<127)
        {
          if (!have_start) { is=i; have_start=true; }
          if (Text[i]<43 || Text[i]>57) is_number=false;
          if (Text[i]=='.') pszPoint=Text+i;
        }
    }

  pszEOS=Text+il-1;

  while (pszEOS>=Text &&
         (*pszEOS==' ' ||
          (is_number && pszPoint!=0 && pszEOS>pszPoint && *pszEOS=='0')))
    *pszEOS--='\0';

  /* next 2 lines commented out to keep decimal point. RS 2012-11-20 */
  // if (is_number && *pszEOS=='.')
  //   *pszEOS--='\0';

  return Text+is;
}

/**************************************************************************/
QString uniqueString(const QString &str,const QStringList &strList)
/**************************************************************************/
/*!

  \brief Generates a string based on \a str that does not exist in \a
  strList.

  Suffixes of the form _(n), with integers \a n beginning with \c 2,
  are appended to \a str if necessary.

  \return The generated unique string.

  \sa uniqueFileName()

*/
{
  /* immediate return if str is empty or not in strList */
  if (str.isEmpty() || !strList.contains(str)) return str;

  /* loop until non-existing str is found. */
  QString n; int suffix=1;
  do { n=str+QString("_(%1)").arg(++suffix); }
  while (strList.contains(n));

  return n;
}

/**************************************************************************/
DECLSPEC
QStringList vocabURLsFromCodes(const QString& str,const QString& rootURL)
/**************************************************************************/
/*!

  \brief Identifies all vocab codes in \a str of the form
  SDN:[vocab]::[code] and constructs respective URLs using the root
  URL \a rootURL as prefix.

  \return The list of constructed vocal URLs.

*/
{
  QStringList urls,sl; QString s; int pos=0,i,len;
  QRegExp rx("SDN:*::",Qt::CaseSensitive,QRegExp::Wildcard);

  while ((pos=rx.indexIn(str,pos))!=-1)
    {
      i=str.indexOf(QChar(' '),pos); len=(i>-1) ? i-pos : -1;
#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
      sl=str.mid(pos,len).split(QChar(':'),Qt::SkipEmptyParts);
#else
      sl=str.mid(pos,len).split(QChar(':'),QString::SkipEmptyParts);
#endif
      urls << QString("%1%2/current/%3/").arg(rootURL).arg(sl.at(1)).arg(sl.at(2));
      pos+=4;
    }


  return urls;
}
