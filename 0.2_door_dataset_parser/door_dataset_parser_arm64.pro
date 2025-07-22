#################################################################
# This QT project file produces console application project or
# make files for Windows, Mac or Linux platforms when used as
# input for qmake.
#
#   - Produce Visual Studio project file
#     qmake -tp vc door_dataset_parser_arm64.pro
#
#################################################################


SOURCES     = door_dataset_parser.cpp \
              ../common/globalFunctions.cpp \
              ../common/InfoMap.cpp \
              ../common/Data.cpp \
              ../common/Datasets.cpp \
              ../common/EventData.cpp \
              ../common/Events.cpp \
              ../common/Params.cpp \
              ../common/RMemArea.cpp\
              ../common/RRandomVar.cpp\
              ../common/Replacer.cpp \
              ../common/Stations.cpp \
              ../common/UnitConverter.cpp\
              ../common/RConfig.cpp \
              ../common/constants.cpp \
              ../common/odvDate.cpp \
              ../common/RDateTime.cpp \
              ../common/systemTools.cpp

INCLUDEPATH += ../

TEMPLATE    = app
QT          += xml
QT          -= gui

win32 {
	CONFIG		+= embed_manifest_exe
	QMAKE_LFLAGS	+= /MACHINE:ARM64 /INCREMENTAL:NO
	DEFINES		+= _CRT_SECURE_NO_WARNINGS
}
