QT += core dbus xml
QT -= gui

TARGET = test_tool
CONFIG += console
CONFIG -= app_bundle

TEMPLATE = app

SRCDIR = ../software/src

include(../software/ext/qslog/QsLog.pri)
include(../software/ext/velib/qt.pri)

INCLUDEPATH += \
	../software/ext/qslog \
	../software/ext/velib/inc \
	../software/src

HEADERS += \
	src/dbus_observer.h \
	src/dbus_settings.h \
	src/dbus_settings_adaptor.h \
	src/dbus_service_observer.h

SOURCES += \
	src/test_tool.cpp \
	src/dbus_observer.cpp \
	src/dbus_settings.cpp \
	src/dbus_settings_adaptor.cpp \
	src/dbus_service_observer.cpp
