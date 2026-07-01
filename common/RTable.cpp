/****************************************************************************
**
** Copyright (C) 2025 Reiner Schlitzer. All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "RTable.h"

#include <QFile>
#include <QTextStream>

#include "globalFunctions.h"


/**************************************************************************/
RTableRow::RTableRow(const QString& line,QChar splitChar)
/**************************************************************************/
/*!

  \brief Creates an RTableRow object and assigns tokens from line \a
  line.

*/
{
  if (line.isEmpty()) return;
  append(splitString(line,splitChar));
}

/**************************************************************************/
RTableRow::RTableRow(const QStringList& tokens)
/**************************************************************************/
/*!

  \brief Creates an RTableRow object and assigns tokens from \a vals.

*/
{
  append(tokens);
}

/**************************************************************************/
RTableRow::RTableRow(int tokenCount)
/**************************************************************************/
/*!

  \brief Creates an RTableRow object containing \a tokenCount empty strings.

*/
{
  for (int i=0; i<tokenCount; ++i)
    append(QString());
}

/**************************************************************************/
QList<int> RTableRow::indexOfList(const QStringList &strs)
/**************************************************************************/
/*!

  \brief Determines the indexes in this object for all strings in \a strs.

  \return The list of determined indexes.

*/
{
  int i,n=strs.size(); QList<int> idxs;
  for (i=0; i<n; ++i)
    idxs.append(indexOf(strs.at(i)));
  return idxs;
}

/**************************************************************************/
bool RTableRow::validateSize(int tokenCount)
/**************************************************************************/
/*!

  \brief Trims this object to at most \a tokenCount tokens.

  Exessive tokens are discarded.

  \return \c true if the size is equal to \a tokenCount, or \c false
  the size is less than \a tokenCount.

*/
{
  while (size()>tokenCount)
    removeLast();

  return (size()==tokenCount);
}

/**************************************************************************/
QString RTableRow::valueAt(int idx,bool *ok) const
/**************************************************************************/
/*!

  \brief Retrieve the value at index \a idx.

  \return The retrieved string.

  The returned string is empty if \a idx is invalid and \a ok (if not \c NULL)
  is set to \c false. \a ok = \c true indicates a successful retrieval.
*/
{
  QString s; if (ok) *ok=false;
  if (idx>=0 && idx<size()) { s=at(idx); if (ok) *ok=true; }
  return s;
}



/**************************************************************************/
RTable::RTable()
  : keyPartLen(-1)
/**************************************************************************/
/*!

  \brief Creates an empty RTable object.

*/
{
}

/**************************************************************************/
RTable::RTable(const QString& fnIn,const QStringList& keyColumnLabels,
               QChar splitChar,int keyPartLength)
  : keyPartLen(keyPartLength)
/**************************************************************************/
/*!

  \brief Creates a RTable object and inserts the rows from file \a fnIn.

  \a fnIn must contain the column labels in the first line, all other
  lines are considered data lines. Data lines with fewer tokens than
  column labels are ignored. Excessive tokens in a data line are discarded.
  \a splitChar is used to break lines into tokens.

*/
{
  insertFile(fnIn,keyColumnLabels,splitChar,keyPartLength);
}

/**************************************************************************/
RTable::RTable(const QStringList& columnLabels,
               const QStringList& keyColumnLabels,
               int keyPartLength)
  : keyPartLen(keyPartLength)
/**************************************************************************/
/*!

  \brief Creates a RTable object using column labels \a columnLabels
  and key label \a keyColLabel.

  There are no RTableRow elements initially. These can be inserted
  any time using the \a insertRow() member function.

*/
{
  header.append(columnLabels);
  keyColIdxs=header.indexOfList(keyColumnLabels);
  if (keyColIdxs.contains(-1))
    { keyPartLen=-1; keyColIdxs.clear(); header.clear(); }
}

/**************************************************************************/
RTable::RTable(const QString& fnIn,const QString& keyColumnLabel,QChar splitChar)
  : keyPartLen(-1)
/**************************************************************************/
/*!

  \brief Creates a RTable object and inserts the rows from file \a fnIn.

  \a fnIn must contain the column labels in the first line, all other
  lines are considered data lines. Data lines with fewer tokens than
  column labels are ignored. Excessive tokens in a data line are discarded.
  \a splitChar is used to break lines into tokens.

*/
{
  insertFile(fnIn,keyColumnLabel,splitChar);
}

/**************************************************************************/
RTable::RTable(const QStringList& columnLabels,const QString& keyColumnLabel)
  : keyPartLen(-1)
/**************************************************************************/
/*!

  \brief Creates a RTable object using column labels \a columnLabels
  and key label \a keyColLabel.

  There are no RTableRow elements initially. These can be inserted
  any time using the \a insertRow() member function.

*/
{
  header.append(columnLabels);
  keyColIdxs.append(header.indexOf(keyColumnLabel));
  if (keyColIdxs.contains(-1))
    { keyPartLen=-1; keyColIdxs.clear(); header.clear(); }
}

/**************************************************************************/
void RTable::append(const RTable& other)
/**************************************************************************/
/*!

  \brief Appends the rows of \a other to this RTable.

  \note \a other must have identical column labels and key column index.

*/
{
  /* immediate return if header or keyColIdxs don't match */
  if (other.keyPartLen!=keyPartLen ||
      other.keyColIdxs!=keyColIdxs ||
      other.header!=header) return;

  /* insert RTableRows from other in this */
  QMultiMap<QString,RTableRow>::ConstIterator it;
  QList<RTableRow> vals; QString key; int i,n;
  for (it=other.constBegin(); it!=other.constEnd(); ++it)
    {
      key=it.key(); vals=other.values(key); n=vals.size();
      for (i=0; i<n; ++i)
        insert(key,vals.at(i));
    }
}

/**************************************************************************/
int RTable::columnIndexOf(const QString& columnLabel)
/**************************************************************************/
/*!

  \brief Retrieves the 0-based column index that has label \a columnLabel.

  \return The retrieved index, or \c -1 if \a columnLabel is not found.

*/
{
  return (header.indexOf(columnLabel));
}

/**************************************************************************/
QString RTable::firstTokenValue(const QString& key,
                                const QString& columnLabel) const
/**************************************************************************/
/*!

  \brief Retrieves the first token for key \a key and column label \a columnLabel.

  If \a key and/or \a columnIdx are invalid the returned string list is empty.

  \return The retrieved token.
*/
{
  bool ok; QStringList sl=tokenValues(key,columnLabel,ok);
  return (sl.isEmpty()) ? QString() : sl.at(0);
}

/**************************************************************************/
int RTable::insertFile(const QString& fnIn,const QStringList& keyColumnLabels,
                       QChar splitChar,int keyPartLength)
/**************************************************************************/
/*!

  \brief Inserts the rows from file \a fnIn.

  \a fnIn must contain the column labels in the first line, all other
  lines are considered data lines. Data lines with fewer tokens than
  column labels are ignored. Excessive tokens in a data line are discarded.
  \a splitChar is used to break lines into tokens.

  \return The number of rows after the insert.

  \note If this object is not empty on entry, \a fnIn must have identical
  column labels and the key column label \a keyColumnLabels must be identical
  to the ones already in use. The \a keyPartLength parameter is ignored
  if this object is not empty on entry
*/
{
  /* load from file fnIn using UTF-8 encoding */
  QFile f(fnIn); if (!f.exists() || !f.open(QIODevice::ReadOnly)) return size();
  QTextStream in(&f); in.setCodec("UTF-8");
  int linesRead=0; QList<int> kcIdxs; QString l,key; RTableRow row;

  /* read the column labels and determine the column indexes used for the
    keys. return if not found or different from previous file. */
  l=in.readLine(); ++linesRead;
  row=RTableRow(l,splitChar); kcIdxs=row.indexOfList(keyColumnLabels);
  if (kcIdxs.contains(-1)) return size();

  if (header.isEmpty())
    { keyPartLen=keyPartLength; keyColIdxs=kcIdxs; header=row; }
  else
    { if (kcIdxs!=keyColIdxs || row!=header) return size(); }

  // int colCount=header.size();
  while (!in.atEnd())
    {
      l=in.readLine(); ++linesRead; row=RTableRow(l,splitChar);
      // if (row.validateSize(colCount) && !(key=keyFor(row)).isEmpty())
      if (!(key=keyFor(row)).isEmpty())
        insert(key,row);
    }

  return size();
}

/**************************************************************************/
int RTable::insertFile(const QString& fnIn,const QString& keyColumnLabel,
                       QChar splitChar)
/**************************************************************************/
/*!

  \brief Inserts the rows from file \a fnIn.

  \a fnIn must contain the column labels in the first line, all other
  lines are considered data lines. Data lines with fewer tokens than
  column labels are ignored. Excessive tokens in a data line are discarded.
  \a splitChar is used to break lines into tokens.

  \return The number of rows after the insert.

  \note If this object is not empty on entry, \a fnIn must have identical
  column labels and the key column label \a keyColumnLabels must be identical
  to the ones already in use. The \a keyPartLength parameter is ignored
  if this object is not empty on entry
*/
{
  QStringList kcLbls; kcLbls.append(keyColumnLabel);
  return insertFile(fnIn,kcLbls,splitChar,-1);
}

/**************************************************************************/
int RTable::insertRow(const RTableRow& row)
/**************************************************************************/
/*!

  \brief Inserts the row \a row.

  \return The number of rows after the insert.

*/
{
  if (row.size()==header.size())
    insert(keyFor(row),row);

  return size();
}

/**************************************************************************/
QString RTable::keyFor(const RTableRow& row)
/**************************************************************************/
/*!

  \brief Constructs the key for row \a row .

  \return The constructed key.

*/
{
  const QString r="                                                    ";
  int i,n=keyColIdxs.size();

  if (n==1)
    return row.valueAt(keyColIdxs.at(0));
  else
    {
      QString key,s;
      for (i=0; i<n; ++i)
        {
          s=row.valueAt(keyColIdxs.at(i));
          key+=r.left(keyPartLen-s.size())+s;
        }

      return key;
  }
}

/**************************************************************************/
QStringList RTable::prefixedColumnLabels(const QString& prefix)
/**************************************************************************/
/*!

  \brief Prefixes each column label with prefix \a prefix.

  \return The list of modified column labels.

*/
{
  QStringList sl; int i,n=header.size();
  for (i=0; i<n; ++i)
    sl.append(prefix+header.at(i));
  return sl;
}

/**************************************************************************/
bool RTable::saveAsSpreadsheet(const QString& fnOut,QChar sepChar)
/**************************************************************************/
/*!

  \brief Saves the contents to UTF-8 encoded file \a fnOut using \a sepChar
  as column separation character.

  \return \c true if successful, or \c false otherwise.

*/
{
  /* open the target file for appending. immediate error return if unsuccessful */
  QFile fi(fnOut); QMultiMap<QString,RTableRow>::ConstIterator it;
  if (!fi.open(QIODevice::Text | QIODevice::WriteOnly)) return false;

  /* create an UTF-8 output stream */
  QTextStream out(&fi); out.setCodec("UTF-8");

  /* output the text */
  out << header.join(sepChar) << endl;;
  for (it=constBegin(); it!=constEnd(); ++it)
    { out << it.value().join(sepChar) << endl; }

  return true;
}

/**************************************************************************/
QStringList RTable::tokenValues(const QString& key,
                                int columnIdx,bool &ok) const
/**************************************************************************/
/*!

  \brief Retrieves the tokens for key \a key and column index
  \a columnIdx.

  If \a key and/or \a columnIdx are invalid the returned string list is
  empty and paramter \a ok is set to \c false. \a ok = \c true indicates
  success.

  \return The retrieved token list.
*/
{
  /* retrieve values for key and do error return none found */
  QList<RTableRow> rows=values(key); ok=false;
  if (rows.isEmpty()) return QStringList();

  QStringList sl; int i,n=rows.size(); bool b;
  ok=true;
  for (i=0; i<n; ++i)
    {
      sl << rows.at(i).valueAt(columnIdx,&b);
      if (!b) ok=false;
    }

  return sl;
}

/**************************************************************************/
QStringList RTable::tokenValues(const QString& key,
                                const QString& columnLabel,bool &ok) const
/**************************************************************************/
/*!

  \brief Retrieves the tokens for key \a key and column label \a columnLabel.

  If \a key and/or \a columnIdx are invalid the returned string list is
  empty and paramter \a ok is set to \c false. \a ok = \c true indicates
  success.

  \return The retrieved token list.
*/
{
  return tokenValues(key,header.indexOf(columnLabel),ok);
}
