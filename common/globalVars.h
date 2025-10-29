/****************************************************************************
**
** Copyright (C) 2025 Reiner Schlitzer. All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <QList>
#include <QString>

/**global variables*******************************************************/
const QChar tab=QChar('\t'),comma=QChar(',');

const QString idpName="IDP2025";
const QString idpRootDir="C:/GEOTRACES/IDP2025/";

// const QString idpName="IDP2021v2";
// const QString idpRootDir="C:/GEOTRACES/IDP2021_unified/";

const QString idpInputDir=idpRootDir+"input/";
const QString idpOutputDir=idpRootDir+"output/";
const QString idpDiagnDir=idpRootDir+"diagnostics/";
const QString idpErrorsDir=idpDiagnDir+"_errors/";
const QString idpIntermDir=idpRootDir+"intermediate/";
const QString idpDataInpDir=idpInputDir+"data/";
const QString idpDataSetInpDir=idpInputDir+"datasets/";
const QString idpPrmListInpDir=idpInputDir+"parameters/";
const QString idpDataSetIntermDir=idpIntermDir+"datasets/";
const QString idpPrmListIntermDir=idpIntermDir+"parameters/";

const QString aerosolPrmFileName="AEROSOL_parameters.txt";
const QString bioGeotracesPrmFileName="BIO_GEOTRACES_parameters.txt";
const QString dissolvedPrmFileName="DISSOLVED_TEI_parameters.txt";
const QString hydrographyPrmFileName="HYDROGRAPHY_AND_BIOGEOCHEMISTRY_parameters.txt";
const QString ligandPrmFileName="LIGAND_parameters.txt";
const QString particlePrmFileName="PARTICULATE_TEI_parameters.txt";
const QString polarPrmFileName="POLAR_parameters.txt";
const QString precipitationPrmFileName="PRECIPITATION_parameters.txt";
const QString sensorPrmFileName="SENSOR_parameters.txt";

const QString unitConversionFilePath=idpInputDir
  +"unit_conversions/unit_conversions.txt";
const QString unitConversionFilePathAerosolRain=idpInputDir
  +"unit_conversions/unit_conversions_aerosol_rain.txt";

const QString editorCmd="C:/Programs/emacs/bin/runemacs.exe";
const QString odvCmd="C:/Programs/Ocean Data View/bin_w64/odv.exe";

const QString jsHeader="/****************************************************************************\n**\n** Copyright (C) 2025 Reiner Schlitzer. All rights reserved.\n**\n** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE\n** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.\n**\n****************************************************************************/\n\n";

const QString fmtDvDef="//<DataVariable>label=\"%1\" value_type=\"%2\" qf_schema=\"SEADATANET\" significant_digits=\"%3\" is_primary_variable=\"%4\" comment=\"%5\" key_variable=\"%6\"</DataVariable>";

const QString fmtMvDef="//<MetaVariable>label=\"%1\" var_type=\"%2\" value_type=\"%3\" qf_schema=\"SEADATANET\" significant_digits=\"%4\" comment=\"%5\"</MetaVariable>";

const QString fmtOdvHead="[General]\nCollectionFormat = ODVCF6\nDataField = %1\nDataType = Profiles\nDescription = %2\nNumberOfMetaVariables = %3\nNumberOfVariables = %4\nPrimaryVariableIndex = 1\nSampleQualityFlagSchema = SEADATANET\nStationQualityFlagSchema = SEADATANET";

const QString fmtPublicationUrl="https://geotraces-portal.sedoo.fr/search?campaign=%1&param=%2";
//const QString fmtPublicationUrl="https://geotraces-biblio.sedoo.fr/search?campaign=%1&param=%2";

const QList<char> sdnQFlags=QList<char>()
  << '0' << '1' << '2' << '3' << '4' << '5' << '6' << '7' << '8' << '9' << 'A' << 'B' << 'Q';
const QList<char> mappedOdvQFlags=QList<char>()
  << '1' << '0' << '0' << '4' << '8' << '0' << '0' << '1' << '1' << '1' << '1' << '1' << '0';
const QList<char> odvQFlags=QList<char>() << '0' << '1' << '4' << '8';
const QList<char> mappedSdnQFlags=QList<char>() << '1' << '0' << '3' << '4';
