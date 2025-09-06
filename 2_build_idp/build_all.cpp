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

  \brief Creates all IDP2025 discrete sample datasets.

*/
{
  // const bool unifyPrms=true;
  const QString dataDir=idpDataInpDir+"discrete/";
  QString inFn,outFn;

  /* ************* LOADING *************** */

  /* load the bottle flag descriptions */
  QMap<char,QString> bottleFlagDescr=bottleFlagDescriptions();

  /* load the unit conversion information from file */
  UnitConverter unitConverter(idpInputDir+"unit_conversions/unit_conversions.txt");

  /* load the bioGEOTRACES information */
  InfoMap bioGeotracesInfos(idpDataInpDir+"biogeotraces/BioGEOTRACES_Omics_IDP2025.txt",
                            "BODC Bottle Number",tab);

  /* load the bottle and cell data documentation information from file */
  InfoMap docuByExtPrmName(dataDir+"BOTTLE_DATA_DOCUMENTATION.csv","PARAMETER",comma);
  docuByExtPrmName.insertFile(dataDir+"CELL_DATA_DOCUMENTATION.csv","PARAMETER",comma);

  /* load the cruise information from file */
  CruisesDB cruisesDB(dataDir+"CRUISES.csv","CRUISE",comma);

  /* load the event information from file */
  EventsDB::diagnoseEventCorrections();
  EventsDB eventsDB(dataDir+"EVENTS.csv","BODC_EVENT_NUMBER",comma);
  eventsDB.insertFile(dataDir+"event_corrections/EVENTS_corrected.csv",
                      "BODC_EVENT_NUMBER",comma);
  eventsDB.autoCorrectStationLabels();

  /* load the PI information from file */
  InfoMap piInfosByName(idpDataSetInpDir+"orcid_list.txt","NAME",tab);

  /* load all IDP parameter definitions */
  ParamDB params(idpIntermDir+"parameters/");

  /* load the key variable associations from file, both unified and non-unified versions */
  InfoMap keyVarsByDataVar(idpPrmListInpDir+"_KEY_VARIABLES.txt","DATA VARIABLE",tab);
  InfoMap keyVarsByDataVarU(idpPrmListInpDir+"_UNIFIED_KEY_VARIABLES.txt","DATA VARIABLE",tab);

  /* load the DOoR dataset information from file */
  QStringList ignoredDatasets=fileContents(idpDataSetInpDir+"datasets_ignore.txt");
  DatasetInfos datasetInfos(idpIntermDir+"datasets/gdac_DataList_essentials.txt",
                            "PARAMETER::BARCODE",tab,&ignoredDatasets);

  /* load data records, ignore records without S&I approval or PI permission */
  DataItemsDB dataItemsDB(dataDir+"BOTTLE_DATA.csv",comma,&datasetInfos,&eventsDB);
  dataItemsDB.appendFile(dataDir+"CELL_DATA.csv",comma);
  dataItemsDB.aggregateSubSamples();


  /* ************* CryosphereDT *************** */

  /* load the data records for CryosphereDT */
  DataItemList cryosphDataItems(CryosphereDT,&dataItemsDB,&datasetInfos);

  /* construct the station list for CryosphereDT */
  StationList cryosphStations=
    eventsDB.collateStations(cryosphDataItems.acceptedEventNumbers.keys(),15.,1.,&eventsDB);
  cryosphStations.writeSpreadsheetFile(idpOutputDir+"stations/",
                                       "IDP2025_Cryosphere_Stations.txt",&eventsDB);

  /* setup the IDP parameter set for CryosphereDT */
  ParamSet cryosphPrms(CryosphereDT,&params,&cryosphDataItems,&datasetInfos);
  cryosphPrms.writeParamLists(idpOutputDir+"parameters/","IDP2025_Parameters_Cryosphere");

  /* collate meta data and data and write to ODV spreadsheet file */
  outFn="GEOTRACES_IDP2025_Cryosphere_Data.txt";
  cryosphPrms.writeDataAsSpreadsheet(&cryosphStations,&cruisesDB,
                                     &docuByExtPrmName,&bioGeotracesInfos,
                                     &piInfosByName,&keyVarsByDataVar,
                                     &unitConverter,&bottleFlagDescr,
                                     idpOutputDir+"data/cryosphere/",outFn);

  /* ************* PrecipitationDT *************** */

  /* load the data records for PrecipitationDT */
  DataItemList precipDataItems(PrecipitationDT,&dataItemsDB,&datasetInfos);

  /* construct the station list for PrecipitationDT */
  StationList precipStations=
    eventsDB.collateStations(precipDataItems.acceptedEventNumbers.keys(),15.,1.,&eventsDB);
  precipStations.writeSpreadsheetFile(idpOutputDir+"stations/",
                                      "IDP2025_Precipitation_Stations.txt",&eventsDB);

  /* setup the IDP parameter set for PrecipitationDT */
  ParamSet precipPrms(PrecipitationDT,&params,&precipDataItems,&datasetInfos);
  precipPrms.writeParamLists(idpOutputDir+"parameters/","IDP2025_Parameters_Precipitation");

  /* collate meta data and data and write to ODV spreadsheet file */
  outFn="GEOTRACES_IDP2025_Precipitation_Data.txt";
  precipPrms.writeDataAsSpreadsheet(&precipStations,&cruisesDB,
                                    &docuByExtPrmName,&bioGeotracesInfos,
                                    &piInfosByName,&keyVarsByDataVar,
                                    &unitConverter,&bottleFlagDescr,
                                    idpOutputDir+"data/precipitation/",outFn);

  /* ************* AerosolsDT *************** */

  /* load the data records for AerosolsDT */
  DataItemList aerosolDataItems(AerosolsDT,&dataItemsDB,&datasetInfos);

  /* construct the station list for AerosolsDT */
  StationList aerosolStations=
    eventsDB.collateStations(aerosolDataItems.acceptedEventNumbers.keys(),15.,1.,&eventsDB);
  aerosolStations.writeSpreadsheetFile(idpOutputDir+"stations/",
                                       "IDP2025_Aerosols_Stations.txt",&eventsDB);

  /* setup the IDP parameter set for AerosolsDT */
  ParamSet aerosolPrms(AerosolsDT,&params,&aerosolDataItems,&datasetInfos);
  aerosolPrms.writeParamLists(idpOutputDir+"parameters/","IDP2025_Parameters_Aerosols");

  /* collate meta data and data and write to ODV spreadsheet file */
  outFn="GEOTRACES_IDP2025_Aerosol_Data.txt";
  aerosolPrms.writeDataAsSpreadsheet(&aerosolStations,&cruisesDB,
                                     &docuByExtPrmName,&bioGeotracesInfos,
                                     &piInfosByName,&keyVarsByDataVar,
                                     &unitConverter,&bottleFlagDescr,
                                     idpOutputDir+"data/aerosols/",outFn);

  /* ************* SeawaterDT *************** */

  /* load the data records for SeawaterDT */
  DataItemList seawaterDataItems(SeawaterDT,&dataItemsDB,&datasetInfos);

  /* construct the station list for SeawaterDT */
  StationList seawaterStations=
    eventsDB.collateStations(seawaterDataItems.acceptedEventNumbers.keys(),15.,5.,&eventsDB);
  seawaterStations.writeSpreadsheetFile(idpOutputDir+"stations/",
                                        "IDP2025_Seawater_Stations.txt",&eventsDB);


  /* setup the IDP parameter set for SeawaterDT - non-unified parameters */
  ParamSet seawaterPrms(SeawaterDT,&params,&seawaterDataItems,&datasetInfos,false);
  // seawaterPrms.writeDescriptions(idpOutputDir+"diagnostics/seawater/",
  //                                "_UNIFIED_PARAMETER_DESCRIPTIONS.txt");
  seawaterPrms.writeParamLists(idpOutputDir+"parameters/","IDP2025_Parameters_Seawater");

  /* collate meta data and data and write to ODV spreadsheet file - non-unified parameters */
  outFn="GEOTRACES_IDP2025_Seawater_Discrete_Sample_Data.txt";
  seawaterPrms.writeDataAsSpreadsheet(&seawaterStations,&cruisesDB,
                                      &docuByExtPrmName,&bioGeotracesInfos,
                                      &piInfosByName,&keyVarsByDataVar,
                                      &unitConverter,&bottleFlagDescr,
                                      idpOutputDir+"data/seawater/",outFn);


  /* setup the IDP parameter set for SeawaterDT - unified parameters */
  ParamSet seawaterPrmsU(SeawaterDT,&params,&seawaterDataItems,&datasetInfos,true);
  // seawaterPrms.writeDescriptions(idpOutputDir+"diagnostics/seawater/",
  //                                "_UNIFIED_PARAMETER_DESCRIPTIONS.txt");
  seawaterPrmsU.writeParamLists(idpOutputDir+"parameters/","IDP2025u_Parameters_Seawater");

  /* collate meta data and data and write to ODV spreadsheet file - unified parameters */
  outFn="GEOTRACES_IDP2025u_Seawater_Discrete_Sample_Data.txt";
  seawaterPrmsU.writeDataAsSpreadsheet(&seawaterStations,&cruisesDB,
                                       &docuByExtPrmName,&bioGeotracesInfos,
                                       &piInfosByName,&keyVarsByDataVarU,
                                       &unitConverter,&bottleFlagDescr,
                                       idpOutputDir+"data/seawater-unified/",outFn);

  return 0;
}
