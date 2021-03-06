load(common_pre)

QT       += core dbus network

TARGET = samplefsd
TEMPLATE = app

CONFIG   += console
CONFIG   += blackmisc blackcore
CONFIG  -= app_bundle

DEPENDPATH += . $$SourceRoot/src
INCLUDEPATH += . $$SourceRoot/src

SOURCES += *.cpp

LIBS *= -lvatsimauth

DESTDIR = $$DestRoot/bin

target.path = $$PREFIX/bin
INSTALLS += target

load(common_post)
