#################################################################
# This QT project file produces console application project or
# make files for Windows, Mac or Linux platforms when used as
# input for qmake.
#
#   - Produce Makefile for use in: make all
#     qmake build_for_linux-x64.pro
#
#################################################################


SOURCES       = door_parameter_parser.cpp
TARGET        = door_parameter_parser

INCLUDEPATH  += ../

TEMPLATE      = app
QT           += xml
QT           -= gui

QMAKE_CXXFLAGS          += -fno-exceptions -std=gnu++11
QMAKE_CXXFLAGS_WARN_OFF  = -Wunused -Wredundant-decls -Wcomment -Wformat
QMAKE_CXXFLAGS_WARN_OFF	+= -Wuninitialized -Winit-self
QMAKE_CXXFLAGS_WARN_OFF += -Wreturn-type -Wno-write-strings
QMAKE_LFLAGS_RELEASE    += -Wl,-s
