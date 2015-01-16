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
target.path = /opt/color-control/dbus-fronius
INSTALLS += target

prefix = $$[QT_INSTALL_PREFIX]
contains(prefix, ".*bpp3.*") {
	message(Target is ccgx)
	DEFINES += TARGET_ccgx
}

# Note: we need the script module to parse JSON fragments as part of the
# fronius solar API.
QT += core network dbus script
QT -= gui

TARGET = dbus-fronius
CONFIG += console
CONFIG -= app_bundle

TEMPLATE = app

include(src/json/json.pri)
include(ext/qslog/QsLog.pri)
include(ext/velib/qt.pri)

INCLUDEPATH += \
	ext/qslog \
	ext/velib/inc \
	ext/velib/lib/Qvelib

SOURCES += \
	src/main.cpp \
	src/froniussolar_api.cpp \
	src/inverter.cpp \
	src/power_info.cpp \
	src/inverter_updater.cpp \
	src/inverter_gateway.cpp \
	src/local_ip_address_generator.cpp \
	src/settings.cpp \
	src/v_bus_node.cpp \
	src/dbus_fronius.cpp \
	src/dbus_bridge.cpp \
	src/dbus_inverter_bridge.cpp \
	src/dbus_settings_bridge.cpp \
	src/inverter_settings.cpp \
	src/dbus_inverter_settings_bridge.cpp \
	src/dbus_gateway_bridge.cpp \
	src/fronius_data_processor.cpp

HEADERS += \
	src/froniussolar_api.h \
	src/inverter.h \
	src/power_info.h \
	src/inverter_updater.h \
	src/inverter_gateway.h \
	src/local_ip_address_generator.h \
	src/settings.h \
	src/v_bus_node.h \
	src/dbus_fronius.h \
	src/dbus_bridge.h \
	src/dbus_inverter_bridge.h \
	src/dbus_settings_bridge.h \
	src/inverter_settings.h \
	src/dbus_inverter_settings_bridge.h \
	src/dbus_gateway_bridge.h \
	src/defines.h \
	src/fronius_data_processor.h
