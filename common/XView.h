#ifndef XVIEW_H
#define XVIEW_H

/****************************************************************************
**
** Copyright (C) 2025 Reiner Schlitzer. All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <QList>
#include <QMap>
#include <QString>

#include "globalDefines.h"


/**************************************************************************/
class CruiseInventory
/**************************************************************************/
/*!

  \brief Class holding the data value count information for one cruise.

*/
{
public:
  CruiseInventory(const QString& cruiseName=QString(),int totalSmplCount=0)
    : cruise(cruiseName),smplCount(totalSmplCount) { }
  void append(const QString& prmName,int valCount)
  { countsByPrmName.insert(prmName,valCount); }
  int size() { return countsByPrmName.size(); }
  
  QString cruise;
  int smplCount;
  QMap<QString,int> countsByPrmName;
};


#endif   // XVIEW_H
