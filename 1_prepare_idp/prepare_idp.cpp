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

  \brief Loads all inputs and performs various test for the IDP creation.

*/
{
  const QString dataDir=idpDataInpDir+"discrete/";

  QString dir,outDir,fn; QStringList sl,slP;

  /* load the bottle flag descriptions */
  QMap<char,QString> bottleFlagDescr=bottleFlagDescriptions();

  /* load the unit conversion information from file */
  UnitConverter unitConverter(idpInputDir+"unit_conversions/unit_conversions.txt");

  /* load the bioGEOTRACES information */
  InfoMap bioGeotracesInfos(idpDataInpDir+"biogeotraces/BioGEOTRACES_Omics.txt",
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


  /* load the DOoR dataset information from file */
  QStringList ignoredDatasets=fileContents(idpDataSetInpDir+"datasets_ignore.txt");
  DatasetInfos datasetInfos(idpIntermDir+"datasets/gdac_DataList_essentials.txt",
                            "PARAMETER::BARCODE",tab,&ignoredDatasets);


  /* load all data records and set the accepted status */
  DataItemsDB dataItemsDB(dataDir+"BOTTLE_DATA.csv",comma,&datasetInfos,&eventsDB);
  dataItemsDB.appendFile(dataDir+"CELL_DATA.csv",comma);
  dataItemsDB.writeDiagnostics(&cruisesDB);

  /* load the data records for all dataTypes */
  DataItemList seawaterDataItems(SeawaterDT,&dataItemsDB,&datasetInfos);
  DataItemList aerosolDataItems(AerosolsDT,&dataItemsDB,&datasetInfos);
  DataItemList precipDataItems(PrecipitationDT,&dataItemsDB,&datasetInfos);
  DataItemList cryosphDataItems(CryosphereDT,&dataItemsDB,&datasetInfos);


  dir=idpDiagnDir+"stations/"; QDir().mkpath(dir);

  /* construct the station lists for all dataTypes */
  StationList seawaterStats=
    eventsDB.collateStations(seawaterDataItems.acceptedEventNumbers.keys(),15.,5.,&eventsDB);
  seawaterStats.writeSpreadsheetFile(dir,"Seawater_Stations.txt",&eventsDB);

  StationList aerosolStats=
    eventsDB.collateStations(aerosolDataItems.acceptedEventNumbers.keys(),15.,1.,&eventsDB);
  aerosolStats.writeSpreadsheetFile(dir,"Aerosol_Stations.txt",&eventsDB);

  StationList precipStats=
    eventsDB.collateStations(precipDataItems.acceptedEventNumbers.keys(),15.,1.,&eventsDB);
  precipStats.writeSpreadsheetFile(dir,"Precipitation_Stations.txt",&eventsDB);

  StationList cryosphStats=
    eventsDB.collateStations(cryosphDataItems.acceptedEventNumbers.keys(),15.,1.,&eventsDB);
  cryosphStats.writeSpreadsheetFile(dir,"Cryosphere_Stations.txt",&eventsDB);


  dir=idpOutputDir+"datasets/"; QDir().mkpath(dir);

  /* write the cruises information file */
  appendRecords(dir+"Cruises.txt",datasetInfos.toCruisesStringList(&cruisesDB),true);

  /* create the IDP2025 contributor documents */
  InfoMap scientistInfoByName(idpIntermDir+"datasets/orcid_list.txt","NAME",tab);
  datasetInfos.writeContributingScientistsInfo(scientistInfoByName);

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
  ParamDB params(idpIntermDir+"parameters/");

  /* setup the IDP parameter sets for all dataTypes taking into
     account S&I approvals and PI permissions */
  ParamSet seawaterPrms(SeawaterDT,&params,&seawaterDataItems,&datasetInfos,false);
  seawaterPrms.writeParamLists(dir,"Seawater_Parameters");
  ParamSet seawaterPrmsU(SeawaterDT,&params,&seawaterDataItems,&datasetInfos,true);
  seawaterPrmsU.writeParamLists(idpOutputDir+"parameters/","Seawater_Parameters_unified");
  ParamSet aerosolPrms(AerosolsDT,&params,&aerosolDataItems,&datasetInfos);
  aerosolPrms.writeParamLists(dir,"Aerosol_Parameters");
  ParamSet precipPrms(PrecipitationDT,&params,&precipDataItems,&datasetInfos);
  precipPrms.writeParamLists(dir,"Precipitation_Parameters");
  ParamSet cryosphPrms(CryosphereDT,&params,&cryosphDataItems,&datasetInfos);
  cryosphPrms.writeParamLists(dir,"Cryosphere_Parameters");


  /* validate units in the data items against units of parameters */
  seawaterDataItems.validateUnits(&seawaterPrms);
  aerosolDataItems.validateUnits(&precipPrms);
  precipDataItems.validateUnits(&aerosolPrms);
  cryosphDataItems.validateUnits(&cryosphPrms);

  return 0;
}
