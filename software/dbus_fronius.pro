# Application version and revision
VERSION = 0.0.1
REVISION = $$system(git --git-dir $$PWD/.git --work-tree $$PWD describe --always --dirty --tags)

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

# Add more folders to ship with the application here
target.path = /opt/dbus_fronius
INSTALLS += target

machine=$$(MACHINE)
contains(machine,ccgx) {
	DEFINES += TARGET_ccgx
}

# Note: we need the script module to parse JSON fragments
QT += core network dbus script
QT -= gui

TARGET = dbus_fronius
CONFIG += console
CONFIG -= app_bundle

TEMPLATE = app

include(src/json/json.pri)
include(ext/qslog/QsLog.pri)

INCLUDEPATH += ext/qslog ext/velib/inc ext/velib/lib/Qvelib

SOURCES += \
	src/main.cpp \
	src/froniussolar_api.cpp \
	src/froniussolar_api_test.cpp \
	src/inverter.cpp \
	src/dbus_test.cpp \
	src/dbus_inverter_guard.cpp \
	src/dbus_settings_guard.cpp \
	src/power_info.cpp \
	src/inverter_updater.cpp \
	src/inverter_gateway.cpp \
	src/local_ip_address_generator.cpp \
	src/settings.cpp \
	ext/velib/src/qt/v_busitem.cpp \
	ext/velib/src/qt/v_busitem_adaptor.cpp \
	ext/velib/src/qt/v_busitem_proxy.cpp \
	ext/velib/src/qt/v_busitems.cpp \
	ext/velib/src/qt/v_busitem_private_prod.cpp \
	ext/velib/src/qt/v_busitem_private_cons.cpp

HEADERS += \
	src/froniussolar_api.h \
	src/froniussolar_api_test.h \
	src/inverter.h \
	src/dbus_test.h \
	src/dbus_inverter_guard.h \
	src/dbus_settings_guard.h \
	src/power_info.h \
	src/inverter_updater.h \
	src/inverter_gateway.h \
	src/local_ip_address_generator.h \
	src/settings.h \
	ext/velib/inc/velib/qt/v_busitem.h \
	ext/velib/inc/velib/qt/v_busitems.h \
	ext/velib/src/qt/v_busitem_adaptor.h \
	ext/velib/src/qt/v_busitem_private.h \
	ext/velib/src/qt/v_busitem_proxy.h \
	ext/velib/src/qt/v_busitem_private_prod.h \
	ext/velib/src/qt/v_busitem_private_cons.h

OTHER_FILES += \
	log.txt
