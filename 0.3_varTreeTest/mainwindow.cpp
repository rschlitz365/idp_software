/****************************************************************************
**
** treeWidget test
**
****************************************************************************/

#include "mainwindow.h"

#include <QApplication>
#include <QFile>
#include <QLabel>
#include <QTextStream>

#include "common/globalVars.h"
#include "common/globalFunctions.h"
#include "common/Cruises.h"
#include "common/Data.h"
#include "common/Events.h"
#include "common/InfoMap.h"
#include "common/Params.h"

#define UCC_CIRCLED_i      QChar(0x24D8) /* circled i */

/**************************************************************************/
MainWindow::MainWindow(QWidget *parent,char *prmType)
  : QMainWindow(parent)
/**************************************************************************/
{
  const QString dataDir=idpDataDir+"DISCRETE_DATA/";
  const QString outDir=idpOutputDir+"parameter_lists/";

  setupUi(this);

  /* select a dataType */
  IdpDataType dataType=(IdpDataType) (prmType[0]-48);
  QString dataTypeName=ParamSet::dataTypeNameFromType(dataType);

  /* load the DOoR dataset information */
  QStringList ignoredDatasets=fileContents(idpDataSetDir+"datasets_ignore.txt");
  DatasetInfos datasetInfos(idpDataSetDir+"gdac_DataList_essentials.txt",
                            "PARAMETER::BARCODE",tab,&ignoredDatasets);

  /* load the cruise information from file */
  CruisesDB cruisesDB(dataDir+"CRUISES.csv","CRUISE",comma);

  /* load the event information from file */
  EventsDB eventsDB(dataDir+"EVENTS.csv","BODC_EVENT_NUMBER",comma);
  eventsDB.insertFile(idpInputDir+"data/_corrections/EVENTS_corrected.csv",
                      "BODC_EVENT_NUMBER",comma);
  eventsDB.autoCorrectStationLabels();

  /* load all data records and set the accepted status */
  DataItemsDB dataItemsDB(dataDir+"BOTTLE_DATA.csv",comma,&datasetInfos,&eventsDB);
  dataItemsDB.appendFile(dataDir+"CELL_DATA.csv",comma);
  dataItemsDB.writeDiagnostics(&cruisesDB);

  /* load the data records for all dataTypes */
  DataItemList seawaterDataItems(SeawaterDT,&dataItemsDB,&datasetInfos);
  DataItemList aerosolDataItems(AerosolsDT,&dataItemsDB,&datasetInfos);
  DataItemList precipitationDataItems(PrecipitationDT,&dataItemsDB,&datasetInfos);
  DataItemList cryosphereDataItems(CryosphereDT,&dataItemsDB,&datasetInfos);

  /* load all parameter definitions */
  ParamDB params(idpParameterListDir);

  /* setup the IDP parameter sets for all dataTypes taking into
     account S&I approvals and PI permissions */
  ParamSet seawaterPrms(SeawaterDT,&params,&seawaterDataItems,&datasetInfos);
  seawaterPrms.writeParamLists(outDir,"IDP2025_Parameters_Seawater");
  ParamSet aerosolPrms(AerosolsDT,&params,&aerosolDataItems,&datasetInfos);
  aerosolPrms.writeParamLists(outDir,"IDP2025_Parameters_Aerosols");
  ParamSet precipitationPrms(PrecipitationDT,&params,&precipitationDataItems,&datasetInfos);
  precipitationPrms.writeParamLists(outDir,"IDP2025_Parameters_Precipitation");
  ParamSet cryospherePrms(CryosphereDT,&params,&cryosphereDataItems,&datasetInfos);
  cryospherePrms.writeParamLists(outDir,"IDP2025_Parameters_Cryosphere");

  DataItemList dataSet(dataType,&dataItemsDB,&datasetInfos);
  ParamSet prmSet(dataType,&params,&dataSet,&datasetInfos);

  ParamGroup prmGroup; Param prm; QString groupName,groupLabel;
  ParamGroupList *prmGroups=prmSet.paramGroupListPtr();
  int i,j,groupCount=prmGroups->size(),prmCount,groupID=0;
  QStringList valSl=QStringList() << "" << "13.27" << "1" << QString(UCC_CIRCLED_i);

  /* fill the treewidget */
  QTreeWidgetItem *twi,*twiv;
  treeWidget->clear(); treeWidget->setColumnCount(4);
  treeWidget->setIndentation(16);
  for (i=0; i<groupCount; ++i)
    {
      prmGroup=prmGroups->at(i); prmCount=prmGroup.prmLst.size();
      groupName=prmGroup.category+" - "+prmGroup.sampler;
      groupLabel=QString("%1 %2").arg(++groupID,3,10,QChar('0')).arg(groupName);
      if (groupName.isEmpty())
        {
          for (j=0; j<prmCount; ++j)
            {
              prm=prmGroup.prmLst.at(j);
              valSl[0]=QString("%1: %2").arg(prm.id).arg(prm.fullLabel());
              twiv=new QTreeWidgetItem((QTreeWidget*) NULL,valSl,0);
              treeWidget->addTopLevelItem(twiv);
            }
        }
      else
        {
          twi=new QTreeWidgetItem((QTreeWidget*) NULL,QStringList(),0);
          treeWidget->addTopLevelItem(twi);
          QLabel *label=new QLabel(QString("<b>%1</b>").arg(groupLabel));
          treeWidget->setItemWidget(twi,0,label);

          for (j=0; j<prmCount; ++j)
            {
              prm=prmGroup.prmLst.at(j);
              valSl[0]=QString("%1: %2").arg(prm.id).arg(prm.fullLabel());
              twiv=new QTreeWidgetItem(twi,valSl,0);
            }
        }
    }
}



/**************************************************************************/
int main(int argc,char *argv[])
/**************************************************************************/
/*!

  \brief Usage: <app name> 1 (or 2, 3, ...)

*/
{
  QApplication app(argc,argv);
  MainWindow window(NULL,argv[1]);
  window.show();
  return app.exec();
}
