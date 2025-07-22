/****************************************************************************
**
** Copyright (C) 2025 Reiner Schlitzer. All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "InfoMap.h"

#include <QFile>
#include <QTextStream>

#include "globalFunctions.h"


/**************************************************************************/
InfoItem::InfoItem(const QString& line,QChar splitChar)
/**************************************************************************/
/*!

  \brief Creates an InfoItem object and assigns values from line \a
  line.

*/
{
  if (line.isEmpty()) return;
  append(splitString(line,splitChar));
}




/**************************************************************************/
InfoMap::InfoMap()
  : keyColumnIndex(-1)
/**************************************************************************/
/*!

  \brief Creates an empty CruiseInfoSet.

*/
{
}

/**************************************************************************/
InfoMap::InfoMap(const QString& fn,const QString& keyLabel,QChar splitChar)
/**************************************************************************/
/*!

  \brief Creates a CruiseInfoSet object and appends the cruise infos
  from file \a fn.

*/
{
  insertFile(fn,keyLabel,splitChar);
}

/**************************************************************************/
void InfoMap::append(const InfoMap& other)
/**************************************************************************/
/*!

  \brief Appends the items of \a other to this InfoMap.

*/
{
  /* immediate return if columnLabels or keyColumnIndex don't match */
  if (other.keyColumnIndex!=keyColumnIndex ||
      other.columnLabels!=columnLabels) return;

  /* insert InfoItems from other in this */
  QMap<QString,InfoItem>::ConstIterator it;
  for (it=other.constBegin(); it!=other.constEnd(); ++it)
    insert(it.key(),it.value());
}

/**************************************************************************/
int InfoMap::columnIndexOf(const QString& columnLabel)
/**************************************************************************/
/*!

  \brief \return The 0-based column index that has label \a columnLabel.

*/
{
  return (columnLabels.indexOf(columnLabel));
}

/**************************************************************************/
int InfoMap::insertFile(const QString& fn,const QString& keyLabel,QChar splitChar)
/**************************************************************************/
/*!

  \brief Inserts the info itemss from file \a fn.

  \return The number of info items after the insert.

*/
{
  /* load from file fn using UTF-8 encoding */
  QFile f(fn); if (!f.exists() || !f.open(QIODevice::ReadOnly)) return size();
  QTextStream in(&f); in.setCodec("UTF-8");
  int idx,linesRead=0; QString l,key; QStringList sl; InfoItem ii;

  /* read the column labels and determine the column index used as
    key. return if not found or different from previous file. */
  l=in.readLine(); ++linesRead; sl=splitString(l,splitChar);
  if (columnLabels.isEmpty())
    { columnLabels=sl; keyColumnIndex=sl.indexOf(keyLabel); }
  else
    if (sl!=columnLabels || (idx=sl.indexOf(keyLabel))!=keyColumnIndex) return size();
  if (keyColumnIndex==-1) return size();

  while (!in.atEnd())
    {
      l=in.readLine(); ++linesRead; ii=InfoItem(l,splitChar);
      key=ii.at(keyColumnIndex); insert(key,ii);
    }

  return size();
}

/**************************************************************************/
QStringList InfoMap::prependedColumnLabels(const QString& prefix)
/**************************************************************************/
/*!

  \brief \return The list of modified column labels with \a prefix
  prepanded on each label.

*/
{
  QStringList sl; int i,n=columnLabels.size();
  for (i=0; i<n; ++i)
    sl.append(prefix+columnLabels.at(i));
  return sl;
}

