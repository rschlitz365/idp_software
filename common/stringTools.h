#ifndef STRINGTOOLS_H
#define STRINGTOOLS_H

/****************************************************************************
 **
** Copyright (C) 1993-2025 Reiner Schlitzer. All rights reserved.
**
** This file is part of Ocean Data View.
**
** Class definitions of:
**                       RMessageList
**
** Global functions:
**                       String manipulation functions
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

/*! \file
  This file implements string manipulation functions.
*/

#include "common/declspec.h"

#include <QHash>
#include <QJsonArray>
#include <QStringList>
#include <QVector>

/*! Message Types */
enum MsgType { NormalMsg, InfoMsg, WarningMsg, ErrorMsg };


/**************************************************************************/
class RSeparator
/**************************************************************************/
{
public:
  enum SeparatorId { Tab, Semicolon, Comma, Space, Slash };

  RSeparator(SeparatorId sepId,QChar sepChar,
            QChar replaceChar,const QString& nameStr)
  { id=sepId; sepCh=sepChar; replCh=replaceChar; name=nameStr; }

  QString quotedStr(const QString& str) const;
  QString validatedStr(const QString& str) const;

  SeparatorId id;
  QChar sepCh;
  QChar replCh;
  QString name;
};

/**************************************************************************/
class DECLSPEC RMessageList : public QStringList
/**************************************************************************/
{
public:
  RMessageList(const QString& logFilePath=QString());
  ~RMessageList();

  void appendErrorMsg(const QString& errMsg);
  void appendInfoMsg(const QString& infoMsg);
  void appendMsg(const QString& msg,MsgType type=NormalMsg);
  void appendMessagesToFile(const QString& fn);
  void appendWarningMsg(const QString& warnMsg);
  void clear();
  int count(const QString& subStr,Qt::CaseSensitivity cs=Qt::CaseInsensitive);
  int errorCount() { return numErrors; }
  bool hasWarningsOrErrors() { return numErrors>0 || numWarnings>0; }
  void prefixLastMessages(const QString& prefix);
  void setPrefixStartIndex(int idx=-1);
  int warningCount() { return numWarnings; }

protected:
  QString logFn;       //!< Log file path
  int numWarnings;     //!< The number of warning messages in \a msgList
  int numErrors;       //!< The number of error messages in \a msgList
  int prefixStartIndex;
  //!< Start index (0-based) into \a msgList for next prefixLastMessages() call
};

DECLSPEC
QString	addEnclosingChars(const QString& s,const QChar& startChar,
                          const QChar& endChar);
DECLSPEC
int	breakLine(char *szLine,const char *sepChar,char *szToken[],int maxToken);
DECLSPEC
bool chopTrailingZeros(QString& str);
DECLSPEC
bool cleanString(QString& str);
DECLSPEC
bool containsDateKeyWords(const QString& str);
DECLSPEC
int containsString(const QStringList& sl,const QString& str,
                   int from=0,Qt::CaseSensitivity cs=Qt::CaseInsensitive);
DECLSPEC
bool containsVocabCode(const QString& str);
DECLSPEC
bool containsURL(const QString& name,const QString& value);
DECLSPEC
QString	countInfoString(int count,int totalCount,const QString& suffix);
DECLSPEC
bool doubleVectorFromJsonArray(const QJsonArray& jsonArr,QVector<double>& dblVec,
                               const QString& errKeyWord,QString& rMsg);
DECLSPEC
void extractRHS(char *rhs,char* Text);
DECLSPEC
QString filePathToLocalUrl(const QString& filePath);
DECLSPEC
QString firstDiffIndicatorStr(int idx);
DECLSPEC
QString firstDiffIndicatorStr(const QString& str,const QString& strC);
DECLSPEC
QString formattedInt(int i);
QString formattedNumber(double d,int decCount,bool doChopTrailingZeros=false,
                        bool clearMissDouble=true);
DECLSPEC
quint64 hashFor(const QString& string);
DECLSPEC
quint64 hashFor(const char *str);
DECLSPEC
int indexOfFirstDiff(const QString& str,const QString& strC);
DECLSPEC
QList<int> indexListOfStr(const QStringList& sl,const QString& str);
DECLSPEC
int indexOfStr(const QStringList& sl,const QString& str,
               int from=0,Qt::CaseSensitivity cs=Qt::CaseInsensitive);
DECLSPEC
int indexOfContainsStr(const QStringList& sl,const QString& str,
                       int from=0,Qt::CaseSensitivity cs=Qt::CaseInsensitive);
DECLSPEC
int indexOfEndsWithStr(const QStringList& sl,const QString& str,
                       int from=0,Qt::CaseSensitivity cs=Qt::CaseInsensitive);
DECLSPEC
int indexOfStartsWithStr(const QStringList& sl,const QString& str,
                         int from=0,Qt::CaseSensitivity cs=Qt::CaseInsensitive);
DECLSPEC
bool intListFromJsonArray(const QJsonArray& jsonArr,QList<int>& intList,
                          int addOffset,int lowerLimit,int upperLimit,
                          const QString& errKeyWord,QString& rMsg);
DECLSPEC
QList<int> intListFromStr(const QString& str,const QString& splitStr);
DECLSPEC
bool isLetter(int asciiCode);
DECLSPEC
bool isLetterOrNumber(int asciiCode);
DECLSPEC
bool isLowerCaseLetter(int asciiCode);
DECLSPEC
bool isNumber(int asciiCode);
DECLSPEC
bool isUpperCaseLetter(int asciiCode);
DECLSPEC
QString	joinedFormattedIntList(const QList<int>& lst,const QString &separator,int addOffset=0);
DECLSPEC
QString	joinedIntList(const QList<int>& lst,const QString &separator,int addOffset=0);
DECLSPEC
void jsonAppendToArray(QString& arr,const QString& item);
DECLSPEC
QString	jsonArrayFromDoubleList(const QList<double>& lst);
DECLSPEC
QString	jsonArrayFromDoubleVals(double *dVals,int count);
DECLSPEC
QString	jsonArrayFromDoubleList(const QList<int>& lst,int addOffset=0);
DECLSPEC
QString	jsonArrayFromIntList(const QList<int>& lst,int addOffset=0,
                            bool asFormattedInt=false);
DECLSPEC
QString	jsonArrayFromIntVals(int *iVals,int count,int addOffset=0);
DECLSPEC
QString	jsonArrayFromStrList(const QStringList& lst);
DECLSPEC
QString	jsonDoubleArrayEntry(const QString& name,const QList<double>& lst);
DECLSPEC
QString	jsonDoubleEntry(const QString& name,double value);
DECLSPEC
QString	jsonFormattedIntArrayEntry(const QString& name,const QList<int>& lst,int addOffset=0);
DECLSPEC
QString	jsonIntArrayEntry(const QString& name,const QList<int>& lst,int addOffset=0);
DECLSPEC
QString	jsonIntEntry(const QString& name,int value);
DECLSPEC
QString	jsonMsgEntry(const QString& msgValue);
DECLSPEC
QString	jsonPolygon(const QString& id,const QList<int> &xC,const QList<int> &yC);
DECLSPEC
QString	jsonStrArrayEntry(const QString& name,const QStringList& lst);
DECLSPEC
QString	jsonStrEntry(const QString& name,const QString& value);
DECLSPEC
QString jsonValidatedStr(const QString& str);
DECLSPEC
QString	leftPaddedStr(const QString& str,int width);
DECLSPEC
QString	logMessage(const QString& action,const QString& msg,
                   const QString& user=QString());
DECLSPEC
QString	mediaTypeFromFileExt(const QString& fileExt);
DECLSPEC
QString	mediaTypeFromFilePath(const QString& filePath);
DECLSPEC
char*	mystrlwr(char *szB);
DECLSPEC
QString	percentStr(int nPart,int nAll);
DECLSPEC
QString	powerTenLabel(int exponent);
DECLSPEC
QString	powerTenLabel(const QString& num);
DECLSPEC
QString pureLocalCdiId(const QString& lCdiId,int *version=NULL);
DECLSPEC
QString	randomString();
DECLSPEC
double readValue(char* szB,int len,double missVal);
DECLSPEC
void	replaceInString(char* szStr,
                      const char *szPre,const char *szPost,int maxlen);
DECLSPEC
void	rmNL(char* szL);
DECLSPEC
QStringList splitString(QString &str,QChar sepChar,
                        const QString& textQuote=QString("\""));
DECLSPEC
QStringList splitStringSimple(QString &str,QChar sepChar);
DECLSPEC
void sprintNumber(char *szNum,double val,int len,int dec);
DECLSPEC
void stripEnclosingChars(QString& s,const QChar& startChar,
                         const QChar& endChar);
DECLSPEC
QString	substitutedString(const QString& t,const QHash<QString,QString>& defs);
DECLSPEC
QString	subString(const QString& str,int startIdx,int endIdx);
DECLSPEC
double toDouble(const QString& t,const double dflt);
DECLSPEC
QList<double> toDoubleList(const QStringList& sl,const double dflt);
DECLSPEC
QList<int> toIntList(const QStringList& sl,const int dflt);
DECLSPEC
QChar	toQChar(QChar ch,int fontType);
DECLSPEC
QString	toQString(const QString& t);
DECLSPEC
QString	toSubOrSuperscript(const QString& s,const QString& prefix);
DECLSPEC
char*	tStrip(char* Text);
DECLSPEC
QString uniqueString(const QString &str,const QStringList &strList);
DECLSPEC
QStringList vocabURLsFromCodes(const QString& str,const QString& rootURL);

#endif	/* !STRINGTOOLS_H */
