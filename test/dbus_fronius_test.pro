# Application version and revision
VERSION = 0.0.1
REVISION = $$system(git --git-dir $$PWD/../.git --work-tree $$PWD describe --always --dirty --tags)

# Create a include file with VERION / REVISION
version_rule.target = $$OUT_PWD/version.h
version_rule.commands = @echo \"updating file $$revtarget.target\"; \
	printf \"/* generated file (do not edit) */\\n \
	$${LITERAL_HASH}ifndef VERSION_H\\n \
	$${LITERAL_HASH}define VERSION_H\\n \
	$${LITERAL_HASH}define VERSION \\\"$${VERSION}\\\"\\n \
	$${LITERAL_HASH}define REVISION \\\"$${REVISION}\\\"\\n \
	$${LITERAL_HASH}endif\" > $$version_rule.target
version_rule.depends = FORCE
QMAKE_DISTCLEAN += $$version_rule.target

QMAKE_EXTRA_TARGETS += version_rule
PRE_TARGETDEPS += $$OUT_PWD/version.h

# suppress the mangling of va_arg has changed for gcc 4.4
QMAKE_CXXFLAGS += -Wno-psabi

# Add more folders to ship with the application here
target.path = /opt/dbus_fronius_test
INSTALLS += target

machine=$$(MACHINE)
contains(machine,ccgx) {
	DEFINES += TARGET_ccgx
}

# suppress the mangling of va_arg has changed for gcc 4.4
QMAKE_CXXFLAGS += -Wno-psabi

QT += core dbus network script xml
QT -= gui

TARGET = dbus_fronius_test
CONFIG += console
CONFIG -= app_bundle

TEMPLATE = app

include(../software/src/json/json.pri)
include(../software/ext/velib/qt.pri)
include(../software/ext/qslog/QsLog.pri)

SRCDIR = ../software/src
EXTDIR = ../software/ext

INCLUDEPATH += \
	../software/ext/velib/inc \
	../software/ext/velib/lib/Qvelib \
	../software/ext/googletest/include \
	../software/ext/googletest \
	../software/src

HEADERS += \
	$$SRCDIR/dbus_bridge.h \
	$$SRCDIR/dbus_inverter_bridge.h \
	$$SRCDIR/dbus_settings_bridge.h \
	$$SRCDIR/froniussolar_api.h \
	$$SRCDIR/inverter.h \
	$$SRCDIR/power_info.h \
	$$SRCDIR/settings.h \
	$$SRCDIR/v_bus_node.h \
	src/dbus_client.h \
	src/dbus_settings_adaptor.h \
	src/dbus_settings.h \
	src/fronius_solar_api_test.h \
	src/test_helper.h

SOURCES += \
	$$SRCDIR/dbus_bridge.cpp \
	$$SRCDIR/dbus_inverter_bridge.cpp \
	$$SRCDIR/dbus_settings_bridge.cpp \
	$$SRCDIR/froniussolar_api.cpp \
	$$SRCDIR/inverter.cpp \
	$$SRCDIR/power_info.cpp \
	$$SRCDIR/settings.cpp \
	$$SRCDIR/v_bus_node.cpp \
	$$EXTDIR/googletest/src/gtest-all.cc \
	src/dbus_client.cpp \
	src/main.cpp \
	src/dbus_settings_adaptor.cpp \
	src/dbus_settings.cpp \
	src/dbus_inverter_bridge_test.cpp \
	src/dbus_settings_bridge_test.cpp \
	src/fronius_solar_api_test.cpp \
	src/test_helper.cpp
