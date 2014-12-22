# Add more folders to ship with the application here
target.path = /opt/dbus_fronius_test
INSTALLS += target

machine=$$(MACHINE)
contains(machine,ccgx) {
	DEFINES += TARGET_ccgx
}

# suppress the mangling of va_arg has changed for gcc 4.4
QMAKE_CXXFLAGS += -Wno-psabi

QT += core dbus network xml
QT -= gui

TARGET = dbus_fronius_test
CONFIG += console qtestlib
CONFIG -= app_bundle

TEMPLATE = app

include(../software/ext/velib/qt.pri)
include(../software/ext/qslog/QsLog.pri)

SRCDIR = ../software/src

INCLUDEPATH += \
	../software/ext/velib/inc \
	../software/src

HEADERS += \
	$$SRCDIR/dbus_bridge.h \
	$$SRCDIR/dbus_settings_bridge.h \
	$$SRCDIR/v_bus_node.h \
	$$SRCDIR/settings.h \
	src/dbus_client.h \
	src/dbus_settings_adaptor.h \
	src/dbus_settings.h \
	src/dbus_settings_bridge_test.h

SOURCES += \
	$$SRCDIR/dbus_bridge.cpp \
	$$SRCDIR/dbus_settings_bridge.cpp \
	$$SRCDIR/settings.cpp \
	$$SRCDIR/v_bus_node.cpp \
	src/dbus_client.cpp \
	src/main.cpp \
	src/dbus_settings_adaptor.cpp \
	src/dbus_settings.cpp \
	src/dbus_settings_bridge_test.cpp
