#ifndef RTABLE_H
#define RTABLE_H

/****************************************************************************
**
** Copyright (C) 2025 Reiner Schlitzer. All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <QMap>
#include <QString>
#include <QStringList>


/**************************************************************************/
class RTableRow : public QStringList
/**************************************************************************/
/*!

  \brief An RTableRow object holds a list of strings.

*/
{
public:
  RTableRow(const QString& line=QString(),QChar splitChar=QChar(','));
  RTableRow(const QStringList& tokens);
  RTableRow(int tokenCount);

  QList<int> indexOfList(const QStringList &strs);
  bool validateSize(int tokenCount);
  QString valueAt(int idx,bool *ok=NULL) const;
};



/**************************************************************************/
class RTable : public QMultiMap<QString,RTableRow>
/**************************************************************************/
/*!

  \brief Container holding a QMap of RTableRow objects using
  strings as keys.

  RTable objects also maintain the list of column labels as well as
  the column indexes used as keys in the QMap.

*/
{
public:
  RTable();
  RTable(const QString& fnIn,const QStringList& keyColumnLabels,
         QChar splitChar,int keyPartLength);
  RTable(const QStringList& columnLabels,
         const QStringList& keyColumnLabels,int keyPartLength);
  RTable(const QString& fnIn,const QString& keyColumnLabel,QChar splitChar);
  RTable(const QStringList& columnLabels,const QString& keyColumnLabel);

  void append(const RTable& other);
  int columnIndexOf(const QString& columnLabel);
  RTableRow emptyRow() const
  { return RTableRow(header.size()); }
  QString firstTokenValue(const QString& key,
                          const QString& columnLabel) const;
  RTableRow headerRow() const
  { return header; }
  int insertFile(const QString& fnIn,const QStringList& keyColumnLabels,
                 QChar splitChar,int keyPartLength);
  int insertFile(const QString& fnIn,const QString& keyColumnLabel,
                 QChar splitChar);
  int insertRow(const RTableRow& row);
  QString keyFor(const RTableRow& row);
  QStringList prefixedColumnLabels(const QString& prefix);
  bool saveAsSpreadsheet(const QString& fnOut,QChar sepChar);
  void setHeaderRow(RTableRow row)
  { header=row; }
  QStringList tokenValues(const QString& key,
                          int columnIdx,bool &ok) const;
  QStringList tokenValues(const QString& key,
                          const QString& columnLabel,bool &ok) const;

protected:
  int keyPartLen; //!< length of individual key parts (only used for multi-column keys)
  QList<int> keyColIdxs; //!< column indexes used for key
  RTableRow header;   //!< header row containing the column  labels
  // QStringList header;   //!< column header labels
};


#endif   // RTABLE_H
