#ifndef PARAM_H
#define PARAM_H

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

// #include "globalDefines.h"
#include "Data.h"
#include "Datasets.h"
#include "InfoMap.h"
#include "Replacer.h"
#include "Stations.h"
#include "UnitConverter.h"

class ParamDB;


/**************************************************************************/
class ODVVarMap : public QMap<int,QString>
/**************************************************************************/
/*!

  \brief Class holding .odv-style properties of ODV variables (values)
  by 1-based variable ids (keys).

*/
{
public:
  QString concatenatedFullLabels(int strtIdx=0,int endIdx=-1);
  QStringList definitionStyledLines(bool useDataVarStyle,
                                    InfoMap *keyVarsByDataVar=NULL);
  QString fullLabel(int varID);
  void load(const QString& fn);
  QStringList odvFileStyledLines();
  QStringList tabStyledLines();
};


/**************************************************************************/
class Param
/**************************************************************************/
/*!

  \brief Struct holding the name, units and description of one
  pre-defined GEOTRACES variable.

*/
{
public:
  Param(const QString& prmName,const QString& prmUnits,
        const QString& prmDescription,int prmId=-1)
  {
    name=prmName; units=prmUnits; description=prmDescription; id=prmId;
    setDataType();
  }
  Param() { id=-1; }

  static IdpDataType dataType(const QString& prmName);
  static IdpDataType dataType(ParamSamplingSystem prmSmplSytem);
  QString fullLabel();
  static QString fullLabel(const QString& prmName,const QString& prmUnits);
  static QString nameLabel(const QString& prmLabel);
  static QString paramNameFromExtendedName(const QString& extPrmName,
                                           QString *barcode=NULL);
  static ParamSamplingSystem samplingSystem(const QString& prmName);
  static QString samplingSystemStr(const ParamSamplingSystem& prmSmplSytem);
  static QStringList samplingSystemSuffixes(IdpDataType dataType);
  void setDataType();
  static QString unifiedNameLabel(const QString& prmName,QString& samplingSuffix);
  static QString unitLabel(const QString& prmLabel);

  int id;              //!< 1-based parameter ID
  QString name;
  QString units;
  QString description;
  IdpDataType dType;
  ParamSamplingSystem smplSystem;
};


/**************************************************************************/
class ParamGroup
/**************************************************************************/
/*!

  \brief Struct defining a group of IDP parameters.

*/
{
public:
  ParamGroup(const QString& keyString,const QString& samplerName,
             const QString& prmCategory,const QList<Param>& paramList)
  { keyWord=keyString; sampler=samplerName; category=prmCategory; prmLst=paramList; }
  ParamGroup() { }

  bool belongsTo(IdpDataType dataType) const;
  bool isEmpty() { return prmLst.isEmpty(); }
  QList<int> paramIdList();
  QStringList paramIdListAsStrings();

  QString keyWord;
  QString sampler;
  QString category;
  QList<Param> prmLst; //!< List of member parameters
};


/**************************************************************************/
class ParamGroupList
/**************************************************************************/
/*!

  \brief Container holding the list of IDP var groups.

  Maintains the order of variables as in the variable list files.

*/
{
public:
  bool append(ParamGroup group);
  ParamGroup at(int idx);
  QStringList categoriesFor(IdpDataType dataType);
  bool contains(const QString &samplerName,const QString &prmCategory);
  QList<int> indexListForCategory(const QString &prmCategory);
  int indexOf(const QString &samplerName,const QString &prmCategory);
  ParamGroup paramGroupFor(const QString &samplerName,const QString &prmCategory);
  int size() { return prmGroups.size(); }

private:
  QList<ParamGroup> prmGroups;
};


/**************************************************************************/
class ParamDB
/**************************************************************************/
/*!

  \brief Container holding the entire IDP parameter information.

*/
{
 public:
  ParamDB(const QString paramListDir);

  int appendGroup(const QString& keyWord,const QString& samplerName,
                  const QString& prmCategory,QList<Param>& prmLst);
  int load(const QString& fn);
  void writeDiagnostics(const QString outDir);

  QString inpDir; //!< directory from which the parameter lists were read
  ParamGroupList prmGroupList; //!< list of parameter groups
  Replacer replacer;
};

/**************************************************************************/
class ParamSet
/**************************************************************************/
/*!

  \brief Container holding parameter information for a given IDP data type.

*/
{
 public:
  // ParamSet(IdpDataType dataType,ParamDB *params,
  //          DataItemList *dataItemList=NULL,DatasetInfos *datasetInfos=NULL);
  ParamSet(IdpDataType dataType,ParamDB *params,
           DataItemList *dataItemList,DatasetInfos *datasetInfos,
           bool unifySamplingSystems=false);

  QString collectionDescription();
  QString collectionField();
  bool contains(int prmID) { return prms.contains(prmID); }
  IdpDataType dataType() { return type; }
  static QString dataTypeNameFromType(IdpDataType dataType);
  bool hasUnifiedPrms() { return unifiedPrms; }
  int largestKey() { return maxPrmID; }
  QStringList leadDataVarDefinitionStyledLines(InfoMap *keyVarsByDataVar)
  { return leadDataVars.definitionStyledLines(true,keyVarsByDataVar); }
  int leadDataVarCount() { return leadDataVars.size(); }
  QString leadDataVarHeader() { return leadDataVars.concatenatedFullLabels();}
  QStringList leadDataVarOdvFileStyledLines()
  { return leadDataVars.odvFileStyledLines(); }
  QStringList leadDataVarTabStyledLines()
  { return leadDataVars.tabStyledLines(); }
  QStringList metaVarDefinitionStyledLines();
  int metaVarCount() { return metaVars.size(); }
  QString metaVarHeader();
  QStringList metaVarOdvFileStyledLines();
  int paramCount() { return prms.count(); }
  QString paramDescription(int prmID);
  Param paramFor(const QString& prmName);
  QString paramFullLabel(int prmID);
  int paramIdFor(const QString& prmName);
  ParamGroupList* paramGroupListPtr() { return &prmGroupList; }
  QMap<int,Param>* paramMapPtr() { return &prms; }
  QString paramName(int prmID);
  QString paramUnits(int prmID);
  QString paramUnitsOf(const QString& prmName);
  void unifyParameters(IdpDataType dataType);
  void writeDataAsSpreadsheet(StationList *stationList,CruisesDB *cruisesDB,
                              InfoMap *docuByExtPrmName,InfoMap *bioGeotracesInfos,
                              InfoMap *piInfosByName,InfoMap *keyVarsByDataVar,
                              UnitConverter *unitConverter,
                              QMap<char,QString> *bottleFlagDescr,
                              const QString& dir,const QString& fn);
  void writeDescriptions(const QString& dir,const QString& fn);
  void writeParamLists(const QString& dir,const QString& fn);

private:
  int maxPrmID;             //!< Largest parameter ID (key in prms)
  IdpDataType type;         //!< Data type
  bool unifiedPrms;         //!< Flag indicating whether parameters are unified or not

  ParamGroupList prmGroupList; //!< parameter groups for the specific data type
  QMap<int,Param> prms;        //!< <prmID, Param> container
  QMap<QString,QString> prmUnitsByName; //!< <name, units> container

  ODVVarMap metaVars;     //!< ODV meta variables for this data type
  ODVVarMap leadDataVars; //!< ODV lead data variables for this data type

  ParamDB *paramDBPtr;
  DataItemList *dataItemListPtr;
  DatasetInfos *datasetInfosPtr;
};


/**************************************************************************/
class ParamNameTranslator
/**************************************************************************/
/*!

  \brief Class managing to/from IDP2014/IDP2017 variable name
  translations.

*/
{
public:
  ParamNameTranslator();
  QString idp2017NameFor(const QString& IDP2025VarName);
  QString IDP2025NameFor(const QString& idp2017VarName);

private:
  QMap<QString,QString> dict; //!< IDP2017 (key) IDP2025 (value) variable names
};



#endif   // PARAM_H
