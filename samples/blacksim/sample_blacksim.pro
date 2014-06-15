include (../../config.pri)
include (../../build.pri)

QT       += core dbus network xml

TARGET = sample_blacksim
TEMPLATE = app

CONFIG   += console
CONFIG   -= app_bundle
CONFIG   += blackmisc blacksim

DEPENDPATH += . ../../src/blackmisc ../../src/blacksim
INCLUDEPATH += . ../../src

win32:!win32-g++*: PRE_TARGETDEPS += ../../lib/blackmisc.lib ../../lib/blacksim.lib
else:              PRE_TARGETDEPS += ../../lib/libblackmisc.a ../../lib/libblacksim.a

DESTDIR = ../../bin

HEADERS += *.h
SOURCES += *.cpp

include (../../libraries.pri)
