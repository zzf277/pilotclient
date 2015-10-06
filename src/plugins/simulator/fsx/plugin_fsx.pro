load(common_pre)

QT       += core dbus gui network concurrent xml

TARGET = simulator_fsx
TEMPLATE = lib

CONFIG += plugin shared
CONFIG += blackmisc blackcore

LIBS +=  -lsimulator_fscommon -lSimConnect -lFSUIPC_User

# required for FSUIPC
win32:!win32-g++*: QMAKE_LFLAGS += /NODEFAULTLIB:LIBC.lib

DEPENDPATH += . $$SourceRoot/src
INCLUDEPATH += . $$SourceRoot/src

LIBS += -ldxguid -lole32

SOURCES += *.cpp
HEADERS += *.h

DESTDIR = $$DestRoot/bin/plugins/simulator

load(common_post)
