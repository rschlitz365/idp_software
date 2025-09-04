/****************************************************************************
**
** Copyright (C) 2025 Reiner Schlitzer. All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <QDir>

#include "common/globalVars.h"
#include "common/globalFunctions.h"
#include "common/Cruises.h"
#include "common/Data.h"
#include "common/Events.h"
#include "common/InfoMap.h"
#include "common/Params.h"
#include "common/RRandomVar.h"
#include "common/RMemArea.h"
#include "common/UnitConverter.h"

#include "common/RConfig.h"
#include "common/odv.h"
#include "common/odvDate.h"
#include "common/RDateTime.h"
#include "common/systemTools.h"

/**************************************************************************/
int main()
/**************************************************************************/
/*!

  \brief Loads all inputs and performs various test for the IDP2025 creation.

*/
{
  const QString dataDir=idpDataInpDir+"DISCRETE_DATA/";
  const QString docOutDir=idpRootDir+"documents/idp/";

  QString dir,outDir,fn; QStringList sl,slP;

  /* load the bottle flag descriptions */
  QMap<char,QString> bottleFlagDescr=bottleFlagDescriptions();

  /* load the unit conversion information from file */
  UnitConverter unitConverter(idpInputDir+"unit_conversions/unit_conversions.txt");

  /* load the bioGEOTRACES information */
  InfoMap bioGeotracesInfos(idpInputDir+"biogeotraces/BioGEOTRACES_Omics_IDP2025.txt",
                            "BODC Bottle Number",tab);

  /* load the bottle and cell data documentation information from file */
  InfoMap docuByExtPrmName(dataDir+"BOTTLE_DATA_DOCUMENTATION.csv",
                           "PARAMETER",comma);
  docuByExtPrmName.insertFile(dataDir+"CELL_DATA_DOCUMENTATION.csv",
                              "PARAMETER",comma);

  /* load the cruise information from file */
  CruisesDB cruisesDB(dataDir+"CRUISES.csv","CRUISE",comma);

  /* load the event information from file */
  EventsDB::diagnoseEventCorrections();
  EventsDB eventsDB(dataDir+"EVENTS.csv","BODC_EVENT_NUMBER",comma);
  eventsDB.insertFile(idpInputDir+"data/_corrections/EVENTS_corrected.csv",
                      "BODC_EVENT_NUMBER",comma);
  eventsDB.autoCorrectStationLabels();


  /* load the DOoR dataset information from file */
  QStringList ignoredDatasets=fileContents(idpDataSetInpDir+"datasets_ignore.txt");
  DatasetInfos datasetInfos(idpDataSetInpDir+"gdac_DataList_essentials.txt",
                            "PARAMETER::BARCODE",tab,&ignoredDatasets);


  /* write the cruises information file */
  appendRecords(docOutDir+"IDP2025_Cruises.txt",
                datasetInfos.toCruisesStringList(&cruisesDB),true);


  /* load all data records and set the accepted status */
  DataItemsDB dataItemsDB(dataDir+"BOTTLE_DATA.csv",comma,&datasetInfos,&eventsDB);
  dataItemsDB.appendFile(dataDir+"CELL_DATA.csv",comma);
  dataItemsDB.writeDiagnostics(&cruisesDB);

  /* load the data records for all dataTypes */
  DataItemList seawaterDataItems(SeawaterDT,&dataItemsDB,&datasetInfos);
  DataItemList aerosolDataItems(AerosolsDT,&dataItemsDB,&datasetInfos);
  DataItemList precipDataItems(PrecipitationDT,&dataItemsDB,&datasetInfos);
  DataItemList cryosphDataItems(CryosphereDT,&dataItemsDB,&datasetInfos);



  /* construct the station lists for all dataTypes */
  StationList seawaterStats=
    eventsDB.collateStations(seawaterDataItems.acceptedEventNumbers.keys(),
                             15.,5.,&eventsDB);
  seawaterStats.writeSpreadsheetFile(dir,"IDP2025_Seawater_Stations.txt",&eventsDB);

  StationList aerosolStats=
    eventsDB.collateStations(aerosolDataItems.acceptedEventNumbers.keys(),
                             15.,1.,&eventsDB);
  aerosolStats.writeSpreadsheetFile(dir,"IDP2025_Aerosols_Stations.txt",&eventsDB);

  StationList precipStats=
    eventsDB.collateStations(precipDataItems.acceptedEventNumbers.keys(),
                             15.,1.,&eventsDB);
  precipStats.writeSpreadsheetFile(dir,"IDP2025_Precipitation_Stations.txt",&eventsDB);

  StationList cryosphStats=
    eventsDB.collateStations(cryosphDataItems.acceptedEventNumbers.keys(),
                             15.,1.,&eventsDB);
  cryosphStats.writeSpreadsheetFile(dir,"IDP2025_Cryosphere_Stations.txt",&eventsDB);


  dir=idpOutputDir+"datasets/"; QDir().mkpath(dir);

  /* create the IDP2025 contributor documents */
  InfoMap scientistInfoByName(idpDataSetInpDir+"orcid_list.txt","NAME",tab);
  QStringList scientistNames=datasetInfos.acceptedPrmsByContribNames.keys();
  QStringList sortedNamesFL=sortedNameList(scientistNames,false);
  QStringList sortedNamesLF=sortedNameList(scientistNames,true);
  int i,n=scientistNames.size(); QMap<QString,int> prmNameMap;
  QString scientistNameFL,scientistNameLF; InfoItem ii;
  QStringList unidentifiedNames;
  for (i=0; i<n; ++i)
    {
      scientistNameFL=sortedNamesFL.at(i);
      scientistNameLF=sortedNamesLF.at(i);
      if (!scientistInfoByName.contains(scientistNameFL))
        { unidentifiedNames.append(scientistNameFL); continue; }

      ii=scientistInfoByName.value(scientistNameFL);
      prmNameMap=datasetInfos.acceptedPrmsByContribNames.value(scientistNameFL);
      sl << QString("%1\t%2\t%3").arg(scientistNameLF).arg(ii.at(0)).arg(ii.at(2));
      slP << QString();
      slP << QString("%1\t%2").arg(scientistNameLF).arg(prmNameMap.keys().join(" | "));
    }
  appendRecords(dir+"Contributing_Scientists.txt",sl,true);
  appendRecords(dir+"Contributing_Scientists_with_Parameters.txt",slP,true);
  appendRecords(dir+"Unidentified_Contributing_Scientist_Names.txt",unidentifiedNames,true);

  QMap<QString,QMap<QString,int> >::ConstIterator it; sl.clear();
  for (it=datasetInfos.acceptedContribNamesByPrms.constBegin();
       it!=datasetInfos.acceptedContribNamesByPrms.constEnd(); ++it)
    {
      sl << QString("%1\t%2").arg(it.key()).arg(it.value().keys().join(" | "));
    }
  appendRecords(dir+"Contributing_Scientists_by_Parameters.txt",sl,true);


  fn=dataDir+"BOTTLE_DATA.csv";

  dir=idpOutputDir+"parameters/"; QDir().mkpath(dir);

  /* load all IDP parameter definitions */
  ParamDB params(idpPrmListInpDir);

  /* setup the IDP parameter sets for all dataTypes taking into
     account S&I approvals and PI permissions */
  ParamSet seawaterPrms(SeawaterDT,&params,&seawaterDataItems,&datasetInfos);
  seawaterPrms.writeParamLists(dir,"IDP2025_Parameters_Seawater");
  ParamSet aerosolPrms(AerosolsDT,&params,&aerosolDataItems,&datasetInfos);
  aerosolPrms.writeParamLists(dir,"IDP2025_Parameters_Aerosols");
  ParamSet precipPrms(PrecipitationDT,&params,&precipDataItems,&datasetInfos);
  precipPrms.writeParamLists(dir,"IDP2025_Parameters_Precipitation");
  ParamSet cryosphPrms(CryosphereDT,&params,&cryosphDataItems,&datasetInfos);
  cryosphPrms.writeParamLists(dir,"IDP2025_Parameters_Cryosphere");


  /* validate units in the data items against units of parameters */
  seawaterDataItems.validateUnits(&seawaterPrms);
  aerosolDataItems.validateUnits(&precipPrms);
  precipDataItems.validateUnits(&aerosolPrms);
  cryosphDataItems.validateUnits(&cryosphPrms);

  return 0;
}
