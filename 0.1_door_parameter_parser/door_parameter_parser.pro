#################################################################
# This QT project file produces console application project or
# make files for Windows, Mac or Linux platforms when used as
# input for qmake.
#
#   - Produce Visual Studio project file
#     qmake -tp vc door_parameter_parser.pro
#
#################################################################


SOURCES     = door_parameter_parser.cpp

INCLUDEPATH += ../

TEMPLATE    = app
QT          += xml
QT          -= gui

win32 {
	CONFIG		+= embed_manifest_exe
	QMAKE_LFLAGS	+= /MACHINE:X64 /INCREMENTAL:NO
	DEFINES		+= _CRT_SECURE_NO_WARNINGS
}
