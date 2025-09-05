#################################################################
# This QT project file produces console application project or
# make files for Windows, Mac or Linux platforms when used as
# input for qmake.
#
#   - Produce Makefile for use in: make all
#     qmake build_for_linux-x64.pro
#
#################################################################


SOURCES       = prepare_idp.cpp \
                ../common/globalFunctions.cpp \
                ../common/Cruises.cpp \
                ../common/Data.cpp \
                ../common/Datasets.cpp \
                ../common/EventData.cpp \
                ../common/Events.cpp \
                ../common/InfoMap.cpp \
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
TARGET        = prepare_idp

INCLUDEPATH  += ../

TEMPLATE      = app
QT           += xml
QT           -= gui

QMAKE_CXXFLAGS          += -fno-exceptions -std=gnu++11
QMAKE_CXXFLAGS_WARN_OFF  = -Wunused -Wredundant-decls -Wcomment -Wformat
QMAKE_CXXFLAGS_WARN_OFF	+= -Wuninitialized -Winit-self
QMAKE_CXXFLAGS_WARN_OFF += -Wreturn-type -Wno-write-strings
QMAKE_LFLAGS_RELEASE    += -Wl,-s
