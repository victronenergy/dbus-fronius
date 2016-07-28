# Application version and revision
VERSION = 0.0.1

# suppress the mangling of va_arg has changed for gcc 4.4
QMAKE_CXXFLAGS += -Wno-psabi

# gcc 4.8 and newer don't like the QOMPILE_ASSERT in qt
QMAKE_CXXFLAGS += -Wno-unused-local-typedefs

# Add more folders to ship with the application here
target.path = /opt/dbus_fronius_test
INSTALLS += target

QT += core network dbus script xml
QT -= gui

TARGET = dbus_fronius_test
CONFIG += console
CONFIG -= app_bundle
DEFINES += VERSION=\\\"$${VERSION}\\\" PRJ_DIR=\\\"$$PWD\\\"

TEMPLATE = app

SRCDIR = ../software/src
EXTDIR = ../software/ext
VELIB_INC = $$EXTDIR/velib/inc/velib/qt
VELIB_SRC = $$EXTDIR/velib/src/qt

include($$EXTDIR/qslog/QsLog.pri)
include($$SRCDIR/json/json.pri)
include($$EXTDIR/velib/src/qt/ve_qitems.pri)

INCLUDEPATH += \
    $$EXTDIR/velib/inc \
    $$EXTDIR/googletest/googletest/include \
    $$EXTDIR/googletest/googletest \
    $$EXTDIR/qthttp/src/qhttp \
    $$SRCDIR

HEADERS += \
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
#    src/dbus_observer.h \
#    src/dbus_service_observer.h \
#    src/dbus_settings_adaptor.h \
#    src/dbus_settings.h \
    src/fronius_solar_api_test.h \
    src/test_helper.h \
    src/dbus_inverter_bridge_test.h \
#    src/dbus_settings_bridge_test.h \
#    src/dbus_inverter_settings_bridge_test.h \
    src/dbus_gateway_bridge_test.h \
    src/fronius_data_processor_test.h

SOURCES += \
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
    $$EXTDIR/googletest/googletest/src/gtest-all.cc \
    src/main.cpp \
#    src/dbus_observer.cpp \
#    src/dbus_service_observer.cpp \
#    src/dbus_settings_adaptor.cpp \
#    src/dbus_settings.cpp \
    src/dbus_inverter_bridge_test.cpp \
#    src/dbus_settings_bridge_test.cpp \
    src/fronius_solar_api_test.cpp \
    src/test_helper.cpp \
#    src/dbus_inverter_settings_bridge_test.cpp \
    src/dbus_gateway_bridge_test.cpp \
    src/fronius_data_processor_test.cpp \

OTHER_FILES += \
    src/fronius_sim/app.py \
    src/fronius_sim/fronius_sim.py
