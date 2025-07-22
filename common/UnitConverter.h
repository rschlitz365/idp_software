#ifndef UNITCONVERTER_H
#define UNITCONVERTER_H

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


/**************************************************************************/
class UnitConverter
/**************************************************************************/
/*!

  \brief Container holding information for an arbitrary number of unit
  conversions.

  The following information is maintained: \a from and \a to units,
  conversion factor and offset and a descriptive text.

*/
{
public:
  UnitConverter() {  }
  UnitConverter(const QString& fn);

  int     append(const QString& varName,
		 const QString& fromUnits,const QString& toUnits,
		 const QString& factor,const QString& offset,
		 const QString& descriptiveText);
  QString description(int index);
  double  factor(int index) { return fac.at(index).toDouble(); }
  int     indexOf(const QString& varName,
		  const QString& fromUnits,const QString& toUnits);
  double  offset(int index) { return off.at(index).toDouble(); }
  int     size() { return from.size(); }
  int     writeSortedListToFile(const QString& fn);

private:
  QStringList var;
  QStringList from;
  QStringList to;
  QStringList fac;
  QStringList off;
  QStringList descr;
};


#endif   // UNITCONVERTER_H
