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

# gcc 4.8 and newer don't like the QOMPILE_ASSERT in qt
QMAKE_CXXFLAGS += -Wno-unused-local-typedefs

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
include(../software/ext/qslog/QsLog.pri)

SRCDIR = ../software/src
EXTDIR = ../software/ext/
VELIB_INC = ../software/ext/velib/inc/velib/qt
VELIB_SRC = ../software/ext/velib/src/qt

INCLUDEPATH += \
    ../software/ext/velib/inc \
    ../software/ext/velib/lib/Qvelib \
    ../software/ext/googletest/googletest/include \
    ../software/ext/googletest/googletest \
    ../software/src

HEADERS += \
    $$VELIB_SRC/v_busitem_adaptor.h \
    $$VELIB_SRC/v_busitem_private_cons.h \
    $$VELIB_SRC/v_busitem_private_prod.h \
    $$VELIB_SRC/v_busitem_private.h \
    $$VELIB_SRC/v_busitem_proxy.h \
    $$VELIB_INC/v_busitem.h \
    $$VELIB_INC/v_busitems.h \
    $$SRCDIR/dbus_bridge.h \
    $$SRCDIR/dbus_gateway_bridge.h \
    $$SRCDIR/dbus_inverter_bridge.h \
    $$SRCDIR/dbus_settings_bridge.h \
    $$SRCDIR/froniussolar_api.h \
    $$SRCDIR/inverter.h \
    $$SRCDIR/local_ip_address_generator.h \
    $$SRCDIR/power_info.h \
    $$SRCDIR/settings.h \
    $$SRCDIR/inverter_gateway.h \
    $$SRCDIR/inverter_updater.h \
    $$SRCDIR/inverter_settings.h \
    $$SRCDIR/dbus_inverter_settings_bridge.h \
    $$SRCDIR/fronius_data_processor.h \
    $$SRCDIR/fronius_device_info.h \
    $$SRCDIR/v_bus_node.h \
    src/dbus_observer.h \
    src/dbus_service_observer.h \
    src/dbus_settings_adaptor.h \
    src/dbus_settings.h \
    src/fronius_solar_api_test.h \
    src/test_helper.h \
    src/dbus_inverter_bridge_test.h \
    src/dbus_settings_bridge_test.h \
    src/dbus_inverter_settings_bridge_test.h \
    src/dbus_gateway_bridge_test.h \
    src/fronius_data_processor_test.h

SOURCES += \
    $$VELIB_SRC/v_busitem.cpp \
    $$VELIB_SRC/v_busitems.cpp \
    $$VELIB_SRC/v_busitem_adaptor.cpp \
    $$VELIB_SRC/v_busitem_private_cons.cpp \
    $$VELIB_SRC/v_busitem_private_prod.cpp \
    $$VELIB_SRC/v_busitem_proxy.cpp \
    $$SRCDIR/dbus_bridge.cpp \
    $$SRCDIR/dbus_gateway_bridge.cpp \
    $$SRCDIR/dbus_inverter_bridge.cpp \
    $$SRCDIR/dbus_inverter_settings_bridge.cpp \
    $$SRCDIR/dbus_settings_bridge.cpp \
    $$SRCDIR/froniussolar_api.cpp \
    $$SRCDIR/inverter.cpp \
    $$SRCDIR/inverter_updater.cpp \
    $$SRCDIR/local_ip_address_generator.cpp \
    $$SRCDIR/power_info.cpp \
    $$SRCDIR/settings.cpp \
    $$SRCDIR/inverter_settings.cpp \
    $$SRCDIR/inverter_gateway.cpp \
    $$SRCDIR/fronius_data_processor.cpp \
    $$SRCDIR/fronius_device_info.cpp \
    $$SRCDIR/v_bus_node.cpp \
    $$EXTDIR/googletest/googletest/src/gtest-all.cc \
    src/main.cpp \
    src/dbus_observer.cpp \
    src/dbus_service_observer.cpp \
    src/dbus_settings_adaptor.cpp \
    src/dbus_settings.cpp \
    src/dbus_inverter_bridge_test.cpp \
    src/dbus_settings_bridge_test.cpp \
    src/fronius_solar_api_test.cpp \
    src/test_helper.cpp \
    src/dbus_inverter_settings_bridge_test.cpp \
    src/dbus_gateway_bridge_test.cpp \
    src/fronius_data_processor_test.cpp \

OTHER_FILES += \
    src/fronius_sim/app.py \
    src/fronius_sim/fronius_sim.py
