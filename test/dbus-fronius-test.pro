# Application version and revision
VERSION = 0.0.1

# suppress the mangling of va_arg has changed for gcc 4.4
QMAKE_CXXFLAGS += -Wno-psabi

# gcc 4.8 and newer don't like the QOMPILE_ASSERT in qt
QMAKE_CXXFLAGS += -Wno-unused-local-typedefs

# Add more folders to ship with the application here
target.path = /opt/dbus_fronius_test
INSTALLS += target

QT += core network script
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
    $$EXTDIR/googletest/include \
    $$EXTDIR/googletest \
    $$EXTDIR/qthttp/src/qhttp \
    $$SRCDIR

HEADERS += \
    $$SRCDIR/froniussolar_api.h \
    $$SRCDIR/gateway_interface.h \
    $$SRCDIR/inverter.h \
    $$SRCDIR/local_ip_address_generator.h \
    $$SRCDIR/power_info.h \
    $$SRCDIR/settings.h \
    $$SRCDIR/inverter_gateway.h \
    $$SRCDIR/inverter_settings.h \
    $$SRCDIR/fronius_data_processor.h \
    $$SRCDIR/fronius_device_info.h \
    $$SRCDIR/ve_qitem_consumer.h \
    $$SRCDIR/ve_service.h \
    src/fronius_solar_api_test.h \
    src/test_helper.h \
    src/dbus_inverter_bridge_test.h \
    src/fronius_data_processor_test.h

SOURCES += \
    $$SRCDIR/froniussolar_api.cpp \
    $$SRCDIR/gateway_interface.cpp \
    $$SRCDIR/inverter.cpp \
    $$SRCDIR/local_ip_address_generator.cpp \
    $$SRCDIR/power_info.cpp \
    $$SRCDIR/settings.cpp \
    $$SRCDIR/inverter_settings.cpp \
    $$SRCDIR/inverter_gateway.cpp \
    $$SRCDIR/fronius_data_processor.cpp \
    $$SRCDIR/fronius_device_info.cpp \
    $$SRCDIR/ve_qitem_consumer.cpp \
    $$SRCDIR/ve_service.cpp \
    $$EXTDIR/googletest/src/gtest-all.cc \
    src/main.cpp \
    src/dbus_inverter_bridge_test.cpp \
    src/fronius_solar_api_test.cpp \
    src/test_helper.cpp \
    src/fronius_data_processor_test.cpp \

OTHER_FILES += \
    src/fronius_sim/app.py \
    src/fronius_sim/fronius_sim.py
