#ifndef INFOMAP_H
#define INFOMAP_H

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
class InfoItem : public QStringList
/**************************************************************************/
/*!

  \brief An InfoItem object holds a list of strings.

*/
{
public:
  InfoItem(const QString& line=QString(),QChar splitChar=QChar(','));
};



/**************************************************************************/
class InfoMap : public QMap<QString,InfoItem>
/**************************************************************************/
/*!

  \brief Container holding a QMap of InfoItem objects (values) using
  strings as keys.

  InfoMap objects also maintain the list of column labels as well as
  the column index used as keys in the QMap.

*/
{
public:
  InfoMap();
  InfoMap(const QString& fn,const QString& keyLabel,QChar splitChar);

  void append(const InfoMap& other);
  int columnIndexOf(const QString& columnLabel);
  int insertFile(const QString& fn,const QString& keyLabel,QChar splitChar);
  QStringList prependedColumnLabels(const QString& prefix);

  int keyColumnIndex;       //!< column index used as key in items
  QStringList columnLabels; //!< column header labels
};


#endif   // INFOMAP_H
