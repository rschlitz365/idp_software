/****************************************************************************
**
** Copyright (C) 2025 Reiner Schlitzer. All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "Params.h"

#include <QDir>
#include <QFile>
#include <QTextStream>

#include "globalVars.h"
#include "globalFunctions.h"
#include "EventData.h"


/**************************************************************************/
QString ODVVarMap::concatenatedFullLabels(int strtIdx,int endIdx)
/**************************************************************************/
/*!

  \brief Constructs the string of tab-concatenated full variable labels.

  \return The constructed string.

*/
{
  QString s; if (endIdx==-1) endIdx=size()-1;
  for (int i=strtIdx; i<=endIdx; ++i)
    {
      if (i>0) s+="\t";
      s+=fullLabel(i+1);
    }
  return s;
}

/**************************************************************************/
QStringList ODVVarMap::definitionStyledLines(bool useDataVarStyle,
                                             InfoMap *keyVarsByDataVar)
/**************************************************************************/
/*!

  \brief Constructs the definition style lines for all entries.

  \return The constructed lines.

*/
{
  int i=0; InfoItem ii;
  QStringList sl,pl; QString valType,prmLbl,kvLbl;
  QMap<int,QString>::ConstIterator it;
  for (it=constBegin(); it!=constEnd(); ++it,++i)
    {
      pl=it.value().split(";"); valType=pl.at(2);
      if (valType=="TEXT") valType+=QString(":%1").arg(pl.at(3));

      if (useDataVarStyle)
        {
          prmLbl=Param::fullLabel(pl.at(0),pl.at(1));
          ii=keyVarsByDataVar->value(prmLbl);
          kvLbl=(ii.isEmpty()) ? QString() : ii.at(1);
          sl << fmtDvDef.arg(prmLbl).arg(valType).arg(pl.at(4))
            .arg(((i>0) ? "F" : "T")).arg(pl.at(8)).arg(kvLbl);
        }
      else
        {
          if (i>=5 && i<=13) continue;
          sl << fmtMvDef.arg(Param::fullLabel(pl.at(0),pl.at(1)))
            .arg(pl.at(7)).arg(valType).arg(pl.at(4)).arg(pl.at(8));
        }
    }
  return sl;
}

/**************************************************************************/
QString ODVVarMap::fullLabel(int varID)
/**************************************************************************/
/*!

  \brief Constructs the full variable label for 1-based variable id \a
  varID.

  \return The constructed variable label.

*/
{
  QStringList sl=value(varID).split(";");
  if (sl.isEmpty()) return QString();
  return Param::fullLabel(sl.at(0),sl.at(1));
}

/**************************************************************************/
void ODVVarMap::load(const QString& fn)
/**************************************************************************/
/*!

  \brief Loads all ODV variable entries from file \a fn.

*/
{
  QStringList sl=fileContents(fn),sp; int i,n=sl.size(),varID;
  for (i=0; i<n; ++i)
    {
      sp=sl.at(i).split(" = "); varID=sp.at(0).toInt();
      insert(varID,sp.at(1));
    }
}

/**************************************************************************/
QStringList ODVVarMap::odvFileStyledLines()
/**************************************************************************/
/*!

  \brief Constructs the .odv-file style lines for all entries.

  \return The constructed lines.

*/
{
  QStringList sl; QMap<int,QString>::ConstIterator it;
  for (it=constBegin(); it!=constEnd(); ++it)
    { sl << QString("%1 = %2").arg(it.key(),4,10,QChar('0')).arg(it.value()); }
  return sl;
}

/**************************************************************************/
QStringList ODVVarMap::tabStyledLines()
/**************************************************************************/
/*!

  \brief Constructs the tab style lines for all entries, consisting of
  id, name, units and description.

  \return The constructed lines.

*/
{
  QStringList sl,pl; QMap<int,QString>::ConstIterator it;
  for (it=constBegin(); it!=constEnd(); ++it)
    {
      pl=it.value().split(";");
      sl << QString("%1\t%2\t%3\t%4").arg(it.key())
        .arg(pl.at(0)).arg(pl.at(1)).arg(pl.at(8));
    }
  return sl;
}


/**************************************************************************/
/**************************************************************************/




/**************************************************************************/
IdpDataType Param::dataType(const QString& prmName)
/**************************************************************************/
/*!

  \brief \return The data type for sampling system \a prmSmplSytem.

*/
{
  return dataType(samplingSystem(prmName));
}

/**************************************************************************/
IdpDataType Param::dataType(ParamSamplingSystem prmSmplSytem)
/**************************************************************************/
/*!

  \brief \return The data type for sampling system \a prmSmplSytem.

*/
{
  switch (prmSmplSytem)
  {
    case BottleSS:
    case PumpSS:
    case FishSS:
    case BoatPumpSS:
    case UwaySS:
    case SubicePumpSS:
    case SensorSS:
      return SeawaterDT;
    case HivolSS:
    case LowvolSS:
    case CoarseImpactorSS:
    case FineImpactorSS:
      return AerosolsDT;
    case AutoSS:
    case ManSS:
      return PrecipitationDT;
    case GrabSS:
    case CorerSS:
    case MeltpondPumpSS:
      return CryosphereDT;
    default:
      return UnknownDT;
  }
}

/**************************************************************************/
QString Param::fullLabel()
/**************************************************************************/
/*!

  \return The full variable label of this variable.

*/
{
  return fullLabel(name,units);
}

/**************************************************************************/
QString Param::fullLabel(const QString& prmName,const QString& prmUnits)
/**************************************************************************/
/*!

  \return The full variable label composed of name and units labels \a
  prmName and prmUnits.

*/
{
  QString s=prmName;
  if (!prmUnits.isEmpty()) s+=QString(" [%1]").arg(prmUnits);
  return s;
}

/**************************************************************************/
QString Param::nameLabel(const QString& prmLabel)
/**************************************************************************/
/*!

  \brief Extracts the name label from the parameter label \a prmLabel.

  \return The extracted name label.

*/
{
  int i,j; QString r,s(prmLabel);

  /* identify and discard any variable label qualifier */
  if ((i=s.indexOf(":"))>-1 && !s.startsWith("STANDARD_DEV::")) s=s.left(i);

  /* find the last appearance of [ ] or ( ) */
  i=s.lastIndexOf(" ["); j=s.lastIndexOf(']');
  if (i==-1) { i=s.lastIndexOf(" ("); j=s.lastIndexOf(')'); }
  /* remove the last appearance of [ ] or ( ) from the string */
  if (j>-1) r=s.mid(j+1).trimmed();
  if (i>-1)
    {
      s=s.left(i).trimmed();
      if (!r.isEmpty()) s+=QString(" ")+r;
    }
  else
    s=s.trimmed();

  /* replace ; by , (; is used as separator in .odv files) */
  s.replace(QChar(';'),QChar(','));
  return s;
}

/**************************************************************************/
QString Param::paramNameFromExtendedName(const QString& extPrmName,
                                         QString *barcode)
/**************************************************************************/
/*!

  \brief Extracts the parameter name label from the extended parameter
  name \a extPrmName.

  If \a barcode is not NULL on entry, returns the barcode part of the
  extended name in \a barcode.

  \return The extracted name label.

*/
{
  QStringList sl=extPrmName.split("::");
  if (barcode) *barcode=sl.at(1);
  return sl.at(0);
}

/**************************************************************************/
ParamSamplingSystem Param::samplingSystem(const QString& prmName)
/**************************************************************************/
/*!

  \return The sampling system determined from the last part of the
  parameter name \a prmName.

*/
{
  if      (prmName.endsWith("_SENSOR"))          return SensorSS;
  else if (prmName.endsWith("_BOTTLE"))          return BottleSS;
  else if (prmName.endsWith("_BOAT_PUMP"))       return BoatPumpSS;
  else if (prmName.endsWith("_MELTPOND_PUMP"))   return MeltpondPumpSS;
  else if (prmName.endsWith("_SUBICE_PUMP"))     return SubicePumpSS;
  else if (prmName.endsWith("_PUMP"))            return PumpSS;
  else if (prmName.endsWith("_UWAY"))            return UwaySS;
  else if (prmName.endsWith("_FISH"))            return FishSS;
  else if (prmName.endsWith("_HIVOL"))           return HivolSS;
  else if (prmName.endsWith("_LOWVOL"))          return LowvolSS;
  else if (prmName.endsWith("_COARSE_IMPACTOR")) return CoarseImpactorSS;
  else if (prmName.endsWith("_FINE_IMPACTOR"))   return FineImpactorSS;
  else if (prmName.endsWith("_AUTO"))            return AutoSS;
  else if (prmName.endsWith("_MAN"))             return ManSS;
  else if (prmName.endsWith("_GRAB"))            return GrabSS;
  else if (prmName.endsWith("_CORER"))           return CorerSS;
  else                                           return UnknownSS;
}

/**************************************************************************/
QString Param::samplingSystemStr(const ParamSamplingSystem& prmSmplSytem)
/**************************************************************************/
/*!

  \brief \return The sampling system suffix string for \a prmSmplSytem.

*/
{
  switch (prmSmplSytem)
  {
    case SensorSS: return QString("_SENSOR");
    case BottleSS: return QString("_BOTTLE");
    case BoatPumpSS: return QString("_BOAT_PUMP");
    case MeltpondPumpSS: return QString("_MELTPOND_PUMP");
    case SubicePumpSS: return QString("_SUBICE_PUMP");
    case PumpSS: return QString("_PUMP");
    case UwaySS: return QString("_UWAY");
    case FishSS: return QString("_FISH");
    case HivolSS: return QString("_HIVOL");
    case LowvolSS: return QString("_LOWVOL");
    case CoarseImpactorSS: return QString("_COARSE_IMPACTOR");
    case FineImpactorSS: return QString("_FINE_IMPACTOR");
    case AutoSS: return QString("_AUTO");
    case ManSS: return QString("_MAN");
    case GrabSS: return QString("_GRAB");
    case CorerSS: return QString("_CORER");
    case UnifiedSS: return QString("_UNIFIED");
    default: return QString();
  }
}

/**************************************************************************/
QStringList Param::samplingSystemSuffixes(IdpDataType dataType)
/**************************************************************************/
/*!

  \return The list of sampling system suffixes for data type \a dataType.

*/
{
  switch (dataType)
  {
    case SeawaterDT:
      return QStringList()
        << "_SENSOR" << "_BOTTLE" << "_BOAT_PUMP"
        << "_SUBICE_PUMP" << "_PUMP" << "_UWAY" << "_FISH";
    case AerosolsDT:
      return QStringList()
        << "_HIVOL" << "_LOWVOL" << "_COARSE_IMPACTOR" << "_FINE_IMPACTOR";
    case PrecipitationDT:
      return QStringList() << "_AUTO" << "_MAN";
    case CryosphereDT:
      return QStringList() << "_GRAB" << "_CORER" << "_MELTPOND_PUMP";
    default:
      return QStringList();
  }
}

/**************************************************************************/
void Param::setDataType()
/**************************************************************************/
/*!

  \brief Sets the parameter's sample system and data type member variables.

*/
{
  smplSystem=samplingSystem(name); dType=dataType(smplSystem);
}

/**************************************************************************/
QString Param::unifiedNameLabel(const QString& prmName,QString& samplingSuffix)
/**************************************************************************/
/*!

  \brief Extracts the part of the parameter name left of the sampling
  suffix.

  \return The extracted pure name label. The sampling suffix is returned
  in \a samplingSuffix.

  \note \c _SENSOR parameter names are returned unchanged.

*/
{
  ParamSamplingSystem samplingSystem=Param::samplingSystem(prmName);
  samplingSuffix=Param::samplingSystemStr(samplingSystem);
  QString n=prmName;
  if (samplingSystem!=SensorSS)
    { int i=n.indexOf(samplingSuffix); if (i>-1) n=n.left(i); }
  return n;
}

/**************************************************************************/
QString Param::unitLabel(const QString& prmLabel)
/**************************************************************************/
/*!

  \brief Extracts the unit label from the parameter label \a prmLabel.

  \return The extracted unit label.

*/
{
  int i,j; QString s(prmLabel);

  /* identify and discard any variable label qualifier */
  if ((i=s.indexOf(":"))>-1 && !s.startsWith("STANDARD_DEV::")) s=s.left(i);

  /* find the last appearance of [ ] or ( ) and extract enclosed label */
  i=s.lastIndexOf(" ["); j=s.lastIndexOf(']');
  if (i==-1) { i=s.lastIndexOf(" ("); j=s.lastIndexOf(')'); }
  s=(i>-1 && j>-1 && j>(i+2)) ? s.mid(i+2,j-i-2).trimmed() : QString();

  /* replace ; by , (; is used as separator in .odv files) */
  s.replace(QChar(';'),QChar(','));
  return s;
}



/**************************************************************************/
/**************************************************************************/


/**************************************************************************/
bool ParamGroup::belongsTo(IdpDataType dataType) const
/**************************************************************************/
/*!

  \brief \return \c true if this group belongs to the data type \a
  dataType, or \c false otherwise.

*/
{
  bool b=false;

  switch (dataType)
    {
    case SeawaterDT:
      b=category.startsWith("Seawater");
      break;
    case AerosolsDT:
      b=category.startsWith("Aerosols");
      break;
    case PrecipitationDT:
      b=sampler.startsWith("Rain Man") || sampler.startsWith("Rain Auto") ||
        sampler.startsWith("Snow Auto") || sampler.startsWith("Snow Man");
      break;
    case CryosphereDT:
      b=category.startsWith("Ice") || category.startsWith("Meltpond") ||
        sampler.startsWith("Snow Grab");
      break;
    }

  return b;
}

/**************************************************************************/
QList<int> ParamGroup::paramIdList()
/**************************************************************************/
/*!

  \brief \return The list of parameter ids as integers.

*/
{
  int i,n=prmLst.size(); QList<int> ids;
  for (i=0; i<n; ++i)
    ids.append(prmLst.at(i).id);
  return ids;
}

/**************************************************************************/
QStringList ParamGroup::paramIdListAsStrings()
/**************************************************************************/
/*!

  \brief \return The list of parameter ids as strings.

*/
{
  int i,n=prmLst.size(); QStringList ids;
  for (i=0; i<n; ++i)
    ids.append(QString::number(prmLst.at(i).id));
  return ids;
}

/**************************************************************************/
/**************************************************************************/


/**************************************************************************/
bool ParamGroupList::append(ParamGroup group)
/**************************************************************************/
/*!

  \brief Appends the parameter group \a group if this group does
  not already exist.

  \return \c true if the group was appended, or \c false otherwise.

*/
{
  bool b=!contains(group.sampler,group.category) && !group.isEmpty();
  if (b) prmGroups.append(group);
  return b;
}

/**************************************************************************/
ParamGroup ParamGroupList::at(int idx)
/**************************************************************************/
/*!

  \brief \return The parameter group at index \a idx.

*/
{
  return prmGroups.at(idx);
}

/**************************************************************************/
QStringList ParamGroupList::categoriesFor(IdpDataType dataType)
/**************************************************************************/
/*!

  \brief \return The list of category labels for data type \a
  dataType.

*/
{
  QStringList sl; int i,n=prmGroups.size();

  for (i=0; i<n; ++i)
    {
      if (prmGroups.at(i).belongsTo(dataType) &&
          !sl.contains(prmGroups.at(i).category))
        sl.append(prmGroups.at(i).category);
    }

  return sl;
}

/**************************************************************************/
bool ParamGroupList::contains(const QString &samplerName,
                              const QString &prmCategory)
/**************************************************************************/
/*!

  \brief Checks whether a parameter group with group name \a samplerName
  and subgroup name \a prmCategory exist.

  \return \c true if a matching group was found, or \c false otherwise.

*/
{
  return (indexOf(samplerName,prmCategory)>-1);
}

/**************************************************************************/
int ParamGroupList::indexOf(const QString &samplerName,
                            const QString &prmCategory)
/**************************************************************************/
/*!

  \brief Checks whether a parameter group with group name \a samplerName
  and subgroup name \a prmCategory exist.

  \return The 0-based index of the first matching group, or \c -1 if
  no such group was found.

*/
{
  int i,n=prmGroups.size();
  for (i=0; i<n; ++i)
    if (prmGroups.at(i).sampler==samplerName &&
        prmGroups.at(i).category==prmCategory) return i;

  return -1;
}

/**************************************************************************/
QList<int> ParamGroupList::indexListForCategory(const QString &prmCategory)
/**************************************************************************/
/*!

  \brief Constructs the index list (zero-based index values) of parameter
  groups with category \a prmCategory.

  \return The constructed index list.

*/
{
  int i,n=prmGroups.size(); QList<int> il;
  for (i=0; i<n; ++i)
    if (prmGroups.at(i).category==prmCategory) il.append(i);

  return il;
}

/**************************************************************************/
ParamGroup ParamGroupList::paramGroupFor(const QString &samplerName,
                                         const QString &prmCategory)
/**************************************************************************/
/*!

  \brief \return The parameter group with group name \a samplerName and
  subgroup name \a prmCategory, or an empty group if no such group is
  found.

*/
{
  int i=indexOf(samplerName,prmCategory);
  return (i>-1) ? prmGroups.at(i) : ParamGroup();
}



/**************************************************************************/
/**************************************************************************/



/**************************************************************************/
ParamDB::ParamDB(const QString paramListDir)
  : inpDir(paramListDir)
/**************************************************************************/
/*!

  \brief Constructs a ParamDB object and loads all parameter
  definitions from directory \a paramListDir.

  The parameters are organized as list of parameter groups by
  parameter category and sampler name.

*/
{
  /* string replacements for keywords and group/subgroup names in parameter files */
  replacer.append("Bottles","Bottle");
  replacer.append("Pumps","Pump");
  replacer.append("Towed fish","Towed Fish");
  replacer.append("Boat-pump","Boat Pump");
  replacer.append("Meltpond-pump","Meltpond Pump");
  replacer.append("Ship's underway","Ship's Underway");
  replacer.append("Subice-pump","Subice Pump");
  replacer.append("Rain-auto","Rain Auto");
  replacer.append("Rain-man","Rain Man");
  replacer.append("Snow-auto","Snow Auto");
  replacer.append("Snow-man","Snow Man");
  replacer.append("Snow-grab","Snow Grab");
  replacer.append("Aerosols-hivol","Aerosols Hivol");
  replacer.append("Aerosols-lowvol","Aerosols Lowvol");
  replacer.append("Aerosols-size_fractionated","Aerosols Size Fractionated");
  replacer.append("Ice-corer","Ice Corer");
  replacer.append("Ice-grab","Ice Grab");
  replacer.append("and inert gases","and Inert Gases");
  replacer.append("and water isotopes","and Water Isotopes");

  /* load all parameter lists */
  load(inpDir+"HYDROGRAPHY_AND_BIOGEOCHEMISTRY_parameters.txt");
  load(inpDir+"DISSOLVED_TEI_parameters.txt");
  load(inpDir+"PARTICULATE_TEI_parameters.txt");
  load(inpDir+"BIO_GEOTRACES_parameters.txt");
  load(inpDir+"AEROSOL_parameters.txt");
  load(inpDir+"PRECIPITATION_parameters.txt");
  load(inpDir+"SENSOR_parameters.txt");
  load(inpDir+"POLAR_parameters.txt");
  load(inpDir+"LIGAND_parameters.txt");
}

/**************************************************************************/
int ParamDB::appendGroup(const QString& keyWord,const QString& samplerName,
                         const QString& prmCategory,QList<Param>& prmLst)
/**************************************************************************/
/*!

  \brief Appends GEOTRACES parameter group \a prmLst to list of groups.

  \return The number of variable groups in this container after append.

*/
{
  prmGroupList.append(ParamGroup(keyWord,samplerName,prmCategory,prmLst));
  return prmGroupList.size();
}

/**************************************************************************/
int ParamDB::load(const QString& fn)
/**************************************************************************/
/*!

  \brief Appends the GEOTRACES parameter definitions from file \a fn
  to the global parameter group list.

  \return The number of parameter groups after append.

*/
{
  QFile f(fn);
  if (!f.exists() || !f.open(QIODevice::ReadOnly)) return prmGroupList.size();

  QTextStream in(&f); in.setCodec("UTF-8");
  QString l,name,units,descr; QStringList sl; int lineCount=0,colCount;

  QString currKeyWord,currSampler,currCategory; QList<Param> currPrms;

  while (!in.atEnd())
    {
      /* read next line */
      l=in.readLine(); ++lineCount;

      /* skip if header line or line is empty */
      if (lineCount==1 || l.trimmed().isEmpty()) continue;

      /* split into tokens; extract group identifiers */
      sl=l.split("\t"); colCount=sl.size();
      QString keyWord,sampler,category;
      if (colCount>=1) keyWord=sl.at(0).trimmed();
      if (colCount>=2) sampler=sl.at(1).trimmed();
      if (colCount>=3) category=sl.at(2).trimmed();

      /* process the line... */
      if      (!keyWord.isEmpty())
        {
          /* ...set the keyword */
          appendGroup(currKeyWord,currSampler,currCategory,currPrms);
          currKeyWord=replacer.applyTo(keyWord);
          currSampler=currCategory=QString(); currPrms.clear();
        }
      else if (!sampler.isEmpty())
        {
          /* ...set the group name */
          appendGroup(currKeyWord,currSampler,currCategory,currPrms);
          currSampler=replacer.applyTo(sampler);
          currCategory=QString(); currPrms.clear();
        }
      else if (!category.isEmpty())
        {
          /* ...set the subgroup name */
          appendGroup(currKeyWord,currSampler,currCategory,currPrms);
          currCategory=replacer.applyTo(category); currPrms.clear();
        }
      else if (colCount>=6)
        {
          /* ...extract and append idp variable */
          name=sl.at(3).trimmed(); units=sl.at(4).trimmed();
          descr=sl.at(5).trimmed();
          stripEnclosingChars(descr,'"','"');
          descr.replace(QChar(';'),QChar(','));
          descr.replace("\"\"","\"");
          if (!name.isEmpty())
            currPrms.append(Param(name,units,descr));
        }
    }

  /* append the current group when file end is reached */
  appendGroup(currKeyWord,currSampler,currCategory,currPrms);

  return prmGroupList.size();
}


/**************************************************************************/
void ParamDB::writeDiagnostics(const QString outDir)
/**************************************************************************/
/*!

  \brief Write diagnostics information to files.

*/
{
  /* construct list of subgroup labels for all data types and save to files.
    these are template files; the actual order is adjusted manually. */
  const QString fmt="_category_priorities_%1.txt";
  appendRecords(outDir+fmt.arg(ParamSet::dataTypeNameFromType(SeawaterDT)),
    prmGroupList.categoriesFor(SeawaterDT),true);
  appendRecords(outDir+fmt.arg(ParamSet::dataTypeNameFromType(AerosolsDT)),
    prmGroupList.categoriesFor(AerosolsDT),true);
  appendRecords(outDir+fmt.arg(ParamSet::dataTypeNameFromType(PrecipitationDT)),
    prmGroupList.categoriesFor(PrecipitationDT),true);
  appendRecords(outDir+fmt.arg(ParamSet::dataTypeNameFromType(CryosphereDT)),
    prmGroupList.categoriesFor(CryosphereDT),true);
}

/**************************************************************************/
/**************************************************************************/




/**************************************************************************/
ParamSet::ParamSet(IdpDataType dataType,ParamDB *params,
                   DataItemList *dataItemList,DatasetInfos *datasetInfos,
                   bool unifySamplingSystems)
  : type(dataType),paramDBPtr(params),unifiedPrms(unifySamplingSystems),
    dataItemListPtr(dataItemList),datasetInfosPtr(datasetInfos)
/**************************************************************************/
/*!

  \brief Constructs a ParamSet object for data type \a dataType and
  retrieves all parameter definitions for this data type from \a
  params.

  If \a dataItemList is not \c NULL on entry, takes into account
  whether data values exist for a given parameter or not.

  If \a datasetInfos is not \c NULL on entry, S&I approval and PI
  permission status is taken into account.

*/
{
  const QStringList methodPriorities= QStringList()
    << "Bottle" << "Pump" << "Towed Fish" << "Ship's Underway"
    << "Boat Pump" << "Subice Pump"
    << "Rain Auto" << "Rain Man" << "Snow Auto" << "Snow Man" << "Snow Grab"
    << "Aerosols Hivol" << "Aerosols Lowvol" << "Aerosols Size Fractionated"
    << "Ice Corer" << "Ice Grab"
    << "Meltpond Pump";

  /* load ODV meta and lead data variable information */
  const QString dir=idpInputDir+"odv_variables/";
  metaVars.load(dir+"MetaVarList.txt");
  leadDataVars.load(dir+QString("LeadDataVarList_%1.txt")
                    .arg(dataTypeNameFromType(type)));

  /* load category priority list for this data type */
  const QString fmt="_category_priorities_%1.txt";
  QStringList categoryPriorities=
    fileContents(idpInputDir+"parameters/"+fmt.arg(dataTypeNameFromType(type)));

  int i,j,k,m=methodPriorities.size(),n=categoryPriorities.size();
  QString method,category; ParamGroup group;
  Param prm; QList<Param> prmLst; bool hasData,isApproved;
  maxPrmID=leadDataVarCount();
  for (i=0; i<n; ++i)
    {
      category=categoryPriorities.at(i);
      for (j=0; j<m; ++j)
        {
          method=methodPriorities.at(j);
          group=paramDBPtr->prmGroupList.paramGroupFor(method,category);
          if (group.isEmpty()) continue;

          for (k=0; k<group.prmLst.size(); ++k)
            {
              prm=group.prmLst.at(k);
              /* checks for data availability, approvals and permissions go here... */
              hasData=!dataItemListPtr || dataItemListPtr->hasDataFor(prm.name);
              isApproved=!datasetInfosPtr ||
                datasetInfosPtr->hasApprovalsForParamName(prm.name);
              if (hasData && isApproved)
                {
                  ++maxPrmID; prm.id=maxPrmID;
                  prms.insert(maxPrmID,prm); prmLst.append(prm);
                  prmUnitsByName.insert(prm.name,prm.units);
                }
            }
          if (!prmLst.isEmpty())
            prmGroupList.append(ParamGroup(group.keyWord,group.sampler,
                                group.category,prmLst));
          prmLst.clear();
        }
    }

    if (unifySamplingSystems) unifyParameters(dataType);
}

/**************************************************************************/
QString ParamSet::collectionDescription()
/**************************************************************************/
/*!

  \return The collection description for this data type.

*/
{
  switch (type)
    {
    case SeawaterDT:
      return "GEOTRACES IDP2025 seawater discrete sample hydrographic and tracer data";
    case AerosolsDT:
      return "GEOTRACES IDP2025 aerosol data";
    case PrecipitationDT:
      return "GEOTRACES IDP2025 precipitation data";
    case CryosphereDT:
      return "GEOTRACES IDP2025 snow and ice data";
    }
  return QString();
}

/**************************************************************************/
QString ParamSet::collectionField()
/**************************************************************************/
/*!

  \return The collection field for this data type.

*/
{
  switch (type)
    {
    case SeawaterDT:
      return "Ocean";
    case AerosolsDT:
      return "Atmosphere";
    case PrecipitationDT:
      return "Atmosphere";
    case CryosphereDT:
      return "GeneralField";
    }
  return QString();
}

/**************************************************************************/
QString ParamSet::dataTypeNameFromType(IdpDataType dataType)
/**************************************************************************/
/*!

  \brief \return The IDP data type name for data type \a dataType.

*/
{
  switch (dataType)
    {
    case SeawaterDT:
      return "Seawater";
    case AerosolsDT:
      return "Aerosols";
    case PrecipitationDT:
      return "Precipitation";
    case CryosphereDT:
      return "Cryosphere";
    default:
      return "Unknown";
    }
}

/**************************************************************************/
QStringList ParamSet::metaVarDefinitionStyledLines()
/**************************************************************************/
/*!

  \return The list of meta variable <MetaVariable> lines.

*/
{
  return metaVars.definitionStyledLines(false);
}

/**************************************************************************/
QString ParamSet::metaVarHeader()
/**************************************************************************/
/*!

  \return The tab-concatenated full meta variable labels as in a ODV
  spreadsheet data file.

*/
{
  return "Cruise\tStation\tType\tyyyy-mm-ddThh:mm:ss.sss\tLongitude [degrees_east]\tLatitude [degrees_north]"+metaVars.concatenatedFullLabels(14);
}

/**************************************************************************/
QStringList ParamSet::metaVarOdvFileStyledLines()
/**************************************************************************/
/*!

  \return The list of meta variable lines styled as  in .odv files.

*/
{
  return metaVars.odvFileStyledLines();
}

/**************************************************************************/
QString ParamSet::paramDescription(int prmID)
/**************************************************************************/
/*!

  \return The description of the variable with key \a prmID, or an
  empty string if there is no such variable.

*/
{
  return prms.contains(prmID) ? prms.value(prmID).description : QString();
}

/**************************************************************************/
Param ParamSet::paramFor(const QString& prmName)
/**************************************************************************/
/*!

  \return The IDP parameter with name label \a prmName, or an empty \c
  Param if there is no such parameter.

*/
{
  return prms.value(paramIdFor(prmName));
}

/**************************************************************************/
QString ParamSet::paramFullLabel(int prmID)
/**************************************************************************/
/*!

  \return The full variable label of the variable with key \a prmID, or
  an empty string if there is no such variable.

*/
{
  if (prms.contains(prmID))
    {
      Param prm=prms.value(prmID);
      return Param::fullLabel(prm.name,prm.units);
    }
  return QString();
}

/**************************************************************************/
int ParamSet::paramIdFor(const QString& prmName)
/**************************************************************************/
/*!

  \return The IDP parameter id of the parameter with name label \a
  prmName, or \c -1 if there is no such parameter.

*/
{
  QMap<int,Param>::ConstIterator it=prms.constBegin();

  while (it!=prms.constEnd())
    {
      if (it.value().name==prmName) return it.key();
      ++it;
    }
  return -1;
}

/**************************************************************************/
QString ParamSet::paramName(int prmID)
/**************************************************************************/
/*!

  \return The name label of the parameter with id \a prmID, or an
  empty string if there is no such variable.

*/
{
  return prms.contains(prmID) ? prms.value(prmID).name : QString();
}

/**************************************************************************/
QString ParamSet::paramUnits(int prmID)
/**************************************************************************/
/*!

  \return The unit label of the parameter with id \a prmID, or an
  empty string if there is no such variable.

*/
{
  return prms.contains(prmID) ? prms.value(prmID).units : QString();
}

/**************************************************************************/
QString ParamSet::paramUnitsOf(const QString& prmName)
/**************************************************************************/
/*!

  \return The unit label of the parameter with name \a prmName, or
  "unknown_units" if there is no such parameter.

*/
{
  return prmUnitsByName.contains(prmName) ?
    prmUnitsByName.value(prmName) : "unknown_units";
}

/**************************************************************************/
void ParamSet::unifyParameters(IdpDataType dataType)
/**************************************************************************/
/*!

  \brief .

*/
{
  ParamGroupList uPrmGroupList; QMap<int,Param> uPrmsById;
  QMap<QString,QString> uPrmUnitsByName; QStringList uPrmNames;

  int i,j,k,unitMismatchCount=0;

  QStringList unifySS=Param::samplingSystemSuffixes(dataType);
  if ((i=unifySS.indexOf(Param::samplingSystemStr(SensorSS)))>-1)
    unifySS.removeAt(i);

  InfoMap descrByUVar(idpPrmListInpDir
                      +"_UNIFIED_PARAMETER_DESCRIPTIONS.txt",
                      "Parameter Name",tab);

  /* loop over all existing parameters and unify */
  QMap<int,Param>::ConstIterator it;
  Param prm; QString purePrmName,ssSuffix; int prmId=prms.keys().at(0)-1;
  for (it=prms.constBegin(); it!=prms.constEnd(); ++it)
    {
      prm=it.value(); purePrmName=Param::unifiedNameLabel(prm.name,ssSuffix);
      if (unifySS.contains(ssSuffix))
        {
          if (!uPrmNames.contains(purePrmName))
            {
              prm.id=(++prmId); prm.name=purePrmName;
              if (descrByUVar.contains(prm.name))
                prm.description=descrByUVar.value(prm.name).at(1);
              uPrmsById.insert(prm.id,prm);
              uPrmNames.append(purePrmName);
              uPrmUnitsByName.insert(prm.name,prm.units);
            }
          else
            {
              if (prm.units!=uPrmUnitsByName.value(purePrmName))
                ++unitMismatchCount;
            }
        }
      else
        {
          if (descrByUVar.contains(prm.name))
            prm.description=descrByUVar.value(prm.name).at(1);
          prm.id=(++prmId); uPrmsById.insert(prm.id,prm);
          uPrmUnitsByName.insert(prm.name,prm.units);
        }
    }
  prms=uPrmsById; prmUnitsByName=uPrmUnitsByName;

  /* loop over all existing parameter groups and unify */
  const QStringList categories=prmGroupList.categoriesFor(dataType);
  const QString samplingStr=Param::samplingSystemStr(UnifiedSS);
  QList<int> idxs; QMap<int,Param> prmMap;
  QString category,prmName,suffix; QList<Param> prmLst; ParamGroup prmGroup;
  for (i=0; i<categories.size(); ++i)
    {
      category=categories.at(i); prmMap.clear();
      idxs=prmGroupList.indexListForCategory(category);
      for (j=0; j<idxs.size(); ++j)
        {
          prmGroup=prmGroupList.at(idxs.at(j)); prmLst=prmGroup.prmLst;
          for (k=0; k<prmLst.size(); ++k)
            {
              prmName=Param::unifiedNameLabel(prmLst.at(k).name,suffix);
              prm=paramFor(prmName);
              if (prm.id>-1) prmMap.insert(prm.id,prm);
            }
        }
      uPrmGroupList.append(ParamGroup(prmGroup.keyWord,samplingStr,category,
                                      prmMap.values()));
    }
  prmGroupList=uPrmGroupList;
}

/**************************************************************************/
void ParamSet
::writeDataAsSpreadsheet(StationList *stationList,CruisesDB *cruisesDB,
                         InfoMap *docuByExtPrmName,InfoMap *bioGeotracesInfos,
                         InfoMap *piInfosByName,InfoMap *keyVarsByDataVar,
                         UnitConverter *unitConverter,
                         QMap<char,QString> *bottleFlagDescr,
                         const QString& dir,const QString& fn)
/**************************************************************************/
/*!

  \brief Writes the IDP data of this data type to file \a fn in
  directory \a dir.

*/
{
  const QString outFn=dir+fn;
  const QString infosDir=dir+"infos/"; QDir().mkpath(infosDir);

  appendRecords(outFn,EventData::spreadsheetHeaderLines(this,keyVarsByDataVar),true);
  int i,j,eventCount,stationCount=stationList->size(); Station station;
  /* loop over all stations and events */
  for (i=0; i<stationCount; ++i)
    {
      station=stationList->at(i); eventCount=station.size();
      for (j=0; j<eventCount; ++j)
        {
          EventData ed(&station,j,datasetInfosPtr,cruisesDB,this,
                       dataItemListPtr,docuByExtPrmName,
                       bioGeotracesInfos,piInfosByName,unitConverter,
                       bottleFlagDescr,infosDir);
          appendRecords(outFn,ed.spreadsheetDataLines());
        }
    }
}

/**************************************************************************/
void ParamSet::writeDescriptions(const QString& dir,const QString& fn)
/**************************************************************************/
/*!

  \brief Writes the parameter names and descriptions to file path \a fn.

*/
{
  QStringList sl; Param prm;
  sl << QString("Parameter Name\t Description");
  QMap<int,Param>::ConstIterator it;
  for (it=prms.constBegin(); it!=prms.constEnd(); ++it)
    {
      prm=it.value();
      sl << QString("%1\t%2").arg(prm.name).arg(prm.description);
    }
  QDir().mkpath(dir); appendRecords(dir+fn,sl,true);
}

/**************************************************************************/
void ParamSet::writeParamLists(const QString& dir,const QString& fn)
/**************************************************************************/
/*!

  \brief Writes the parameters of this object in various formats to
  files with base path \a fn.

*/
{
  const QString fmtGrpPrm=QString("%1 = %2; %3; %4");
  const QString fmtTabPrm=QString("%1\t%2\t%3\t%4");
  const QString fmtOdvPrm=QString("%1 = %2;%3;FLOAT;4;2;0;SEADATANET;BASIC;%4");
  const QString fmtGrpGrp=QString("[%1 %2 - %3]");
  const QString fmtOdvGrp=QString("%1 %2 - %3 = %4");

  QStringList odvLines,grpLines,tabLines,odvFooter; int groupID=2;

  odvFooter << QString() << "[Data Groups]"
    << "001 <TopLevelGroup> = 1, 2"
    << "002 Sample Metadata =  ...to be completed...";

  odvLines << fmtOdvHead.arg(collectionField()).arg(collectionDescription())
    .arg(metaVarCount()).arg(leadDataVarCount()+prms.size());
  odvLines << "\n[Meta Variables]" << metaVarOdvFileStyledLines();
  odvLines << "\n[Variables]" << leadDataVarOdvFileStyledLines();
  tabLines << leadDataVarTabStyledLines();

  QDir().mkpath(dir); //ensure output directory exists

  ParamGroup prmGroup; Param prm;
  int i,j,groupCount=prmGroupList.size(),prmCount;
  for (i=0; i<groupCount; ++i)
    {
      prmGroup=prmGroupList.at(i); prmCount=prmGroup.prmLst.size(); ++groupID;
      odvFooter << fmtOdvGrp.arg(groupID,3,10,QChar('0')).arg(prmGroup.category)
                    .arg(prmGroup.sampler).arg(prmGroup.paramIdListAsStrings().join(", "));
      grpLines << QString();
      grpLines << fmtGrpGrp.arg(groupID,3,10,QChar('0')).arg(prmGroup.category)
                    .arg(prmGroup.sampler);
      for (j=0; j<prmCount; ++j)
        {
          prm=prmGroup.prmLst.at(j);
          odvLines << fmtOdvPrm.arg(prm.id,4,10,QChar('0')).arg(prm.name)
            .arg(prm.units).arg(prm.description);
          grpLines << fmtGrpPrm.arg(prm.id,4,10,QChar('0')).arg(prm.name)
            .arg(prm.units).arg(prm.description);
          tabLines << fmtTabPrm.arg(prm.id).arg(prm.name)
            .arg(prm.units).arg(prm.description);
        }
    }

  appendRecords(dir+fn+".odv+",odvLines+odvFooter,true);
  appendRecords(dir+fn+"_grouped.txt",grpLines,true);
  appendRecords(dir+fn+"_for_document.txt",tabLines,true);
}



/**************************************************************************/
/**************************************************************************/



/**************************************************************************/
ParamNameTranslator::ParamNameTranslator()
/**************************************************************************/
/*!

  \brief Constructs a ParamNameTranslator object.

*/
{
  dict.insert("BRASSICASTEROL_13_LPT_DELTA_PUMP","");
  dict.insert("BRASSICASTEROL_13_SPT_DELTA_PUMP","");
  dict.insert("BRASSICASTEROL_LP_CONC_PUMP","");
  dict.insert("BRASSICASTEROL_SP_CONC_PUMP","");
  dict.insert("BSi_30_TP_DELTA_PUMP","bSi_30_28_TP_DELTA_PUMP");
  dict.insert("BSi_TP_CONC_PUMP","bSi_TP_CONC_PUMP");
  dict.insert("CFC-11","CFC-11_D_CONC_BOTTLE");
  dict.insert("CFC-12","CFC-12_D_CONC_BOTTLE");
  dict.insert("CFC113","CFC113_D_CONC_BOTTLE");
  dict.insert("CHLORA","CHLA_FLUOR_TP_CONC_BOTTLE");
  dict.insert("CHOLESTEROL_13_LPT_DELTA_PUMP","");
  dict.insert("CHOLESTEROL_13_SPT_DELTA_PUMP","");
  dict.insert("CHOLESTEROL_LP_CONC_PUMP","");
  dict.insert("CHOLESTEROL_SP_CONC_PUMP","");
  dict.insert("CTDPRS","PRESSURE");
  dict.insert("Cd_110_112_D_RATIO_BOTTLE","");
  dict.insert("Cd_114_D_EPSILON_BOTTLE","Cd_114_110_D_DELTA_BOTTLE");
  dict.insert("DIC","DIC_D_CONC_BOTTLE");
  dict.insert("DIC_13_D_DELTA_BOTTLE","DIC_13_12_D_DELTA_BOTTLE");
  dict.insert("DOC","DOC_D_CONC_BOTTLE");
  dict.insert("Fe_56_D_DELTA_BOTTLE","Fe_56_54_D_DELTA_BOTTLE");
  dict.insert("Fe_56_TP_DELTA_BOTTLE","Fe_56_54_TP_DELTA_BOTTLE");
  dict.insert("Fe_D_CONC_BOTTLE_FIA","");
  dict.insert("Filtration_Volumn","Filtration_Volume");
  dict.insert("H2O2_T_CONC_BOTTLE","H2O2_D_CONC_BOTTLE");
  dict.insert("H2O_18_D_DELTA_BOTTLE","H2O_18_16_D_DELTA_BOTTLE");
  dict.insert("H2O_2_D_DELTA_BOTTLE","H2O_2_1_D_DELTA_BOTTLE");
  dict.insert("He_3_D_DELTA_BOTTLE","He_3_4_D_DELTA_BOTTLE");
  dict.insert("He_4_D_CONC_BOTTLE","He_D_CONC_BOTTLE");
  dict.insert("Hf_176_D_EPSILON_BOTTLE","Hf_176_177_D_EPSILON_BOTTLE");
  dict.insert("IO3_D_CONC_BOTTLE","I_V_D_CONC_BOTTLE");
  dict.insert("NH4","NH4_D_CONC_BOTTLE");
  dict.insert("NITRAT","NITRATE_D_CONC_BOTTLE");
  dict.insert("NITRAT_NM","NITRATE_LL_D_CONC_BOTTLE");
  dict.insert("NITRIT","NITRITE_D_CONC_BOTTLE");
  dict.insert("NITRIT_NM","NITRITE_LL_D_CONC_BOTTLE");
  dict.insert("NO2+NO3","NO2+NO3_D_CONC_BOTTLE");
  dict.insert("NO3_15_D_DELTA_BOTTLE","NITRATE_15_14_D_DELTA_BOTTLE");
  dict.insert("NO3_15_TD_DELTA_BOTTLE","");
  dict.insert("NO3_18_TD_DELTA_BOTTLE","");
  dict.insert("Nd_143_D_EPSILON_BOTTLE","Nd_143_144_D_EPSILON_BOTTLE");
  dict.insert("OXYGEN","OXYGEN_D_CONC_BOTTLE");
  dict.insert("PHAEOPIGMENTS","PHAEO_FLUOR_TP_CONC_BOTTLE");
  dict.insert("PHSPHT","PHOSPHATE_D_CONC_BOTTLE");
  dict.insert("PHSPHT_NM","PHOSPHATE_LL_D_CONC_BOTTLE");
  dict.insert("PH_SWS","PH_SWS_BOTTLE");
  dict.insert("POC","POC_TP_CONC_BOTTLE");
  dict.insert("POC_13_LPT_DELTA_PUMP","POC_13_12_LPT_DELTA_PUMP");
  dict.insert("POC_13_SPT_DELTA_PUMP","POC_13_12_SPT_DELTA_PUMP");
  dict.insert("POC_LP_CONC_PUMP","POC_LPT_CONC_PUMP");
  dict.insert("POC_SP_CONC_PUMP","POC_SPT_CONC_PUMP");
  dict.insert("PON","PN_TP_CONC_BOTTLE");
  dict.insert("SALNTY","SALINITY_D_CONC_BOTTLE");
  dict.insert("SF6","SF6_D_CONC_BOTTLE");
  dict.insert("SILCAT","SILICATE_D_CONC_BOTTLE");
  dict.insert("SILICAT_30_D_DELTA_BOTTLE","SILICATE_30_28_D_DELTA_BOTTLE");
  dict.insert("TALK","TALK_D_CONC_BOTTLE");
  dict.insert("TN","TN_T_CONC_BOTTLE");
  dict.insert("TOC","TOC_T_CONC_BOTTLE");
  dict.insert("TRITUM","TRITIUM_D_CONC_BOTTLE");
}

/**************************************************************************/
QString ParamNameTranslator::idp2017NameFor(const QString& IDP2025VarName)
/**************************************************************************/
/*!

  \brief \return The IDP2017 variable name for IDP2025 variable name
  \a IDP2025VarName.

*/
{
  QString key=dict.key(IDP2025VarName,QString());
  return (key.isEmpty()) ? IDP2025VarName : key;
}

/**************************************************************************/
QString ParamNameTranslator::IDP2025NameFor(const QString& idp2017VarName)
/**************************************************************************/
/*!

  \brief \return The IDP2025 variable name for IDP2017 variable name
  \a idp2017VarName.

*/
{
  return dict.value(idp2017VarName,idp2017VarName);
}
