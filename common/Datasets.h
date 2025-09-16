#ifndef DATASETS_H
#define DATASETS_H

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

#include "globalDefines.h"
#include "InfoMap.h"

class CruisesDB;

/**************************************************************************/
class DatasetInfos : public InfoMap
/**************************************************************************/
/*!

  \brief Container holding the set of DOoR dataset entries.

*/
{
public:
  DatasetInfos(const QString& fn,const QString& keyLabel,QChar splitChar,
               QStringList *ignoredDatasets);

  QString geotracesCruiseNameFor(const QString& cruise);
  bool hasApprovalsForExtendedParamName(const QString& extPrmName);
  bool hasApprovalsForParamName(const QString& prmName);
  bool isRemovedDataset(const QString& cruise,const QString& prmName);
  QMap<QString,QString>* sectionsByCruisePtr() { return &sectsByCruiseName; }
  QStringList toCruisesStringList(CruisesDB *cruises);
  void writeContributingScientistsInfo(const InfoMap& piInfosByName);

  int idxCruise,idxGeotracesCruise,idxPrmBarcode,idxSiApproval,idxPiPermission;
  int idxDataGenerator,idxAuthorisedScientist,idxIdpVersion;
  QStringList *ignoredDatasetsPtr;
  QMap<QString,QMap<QString,int> > acceptedPrmsByContribNames;
  //!< accepted parameter names (value) by data contributor names (!removed and S&I approved and PI permitted)
  QMap<QString,QMap<QString,int> > acceptedContribNamesByPrms;
  //!< accepted contributor names (value) by parameter names (!removed and S&I approved and PI permitted)
  QMap<QString,int> prmNamesAccepted;
  //!< pure parameter names (!removed and S&I approved and PI permitted)
  QMap<QString,int> extPrmNamesSiApproved;
  //!< extended parameter names (S&I approved)
  QMap<QString,int> extPrmNamesPiApproved;
  //!< extended parameter names (PI permitted)
  QMap<QString,QString> sectsByCruiseName;
  //!< GEOTRACES section names (values) by cruise names (keys)
};

#endif   // DATASETS_H
