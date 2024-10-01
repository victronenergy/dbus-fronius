# Application version and revision
VERSION = 0.0.1

# suppress the mangling of va_arg has changed for gcc 4.4
QMAKE_CXXFLAGS += -Wno-psabi

# gcc 4.8 and newer don't like the QOMPILE_ASSERT in qt
QMAKE_CXXFLAGS += -Wno-unused-local-typedefs

equals(QT_MAJOR_VERSION, 6): QMAKE_CXXFLAGS += -std=c++17

# Add more folders to ship with the application here
target.path = /opt/dbus_fronius_test
INSTALLS += target

QT += core network
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

include($$EXTDIR/veutil/veutil.pri)
include($$SRCDIR/qhttp/qhttp.pri)

INCLUDEPATH += \
    $$EXTDIR/velib/inc \
    $$EXTDIR/googletest/include \
    $$EXTDIR/googletest \
    $$EXTDIR/qthttp/src/qhttp \
    $$SRCDIR

HEADERS += \
    $$SRCDIR/froniussolar_api.h \
    $$SRCDIR/inverter.h \
    $$SRCDIR/power_info.h \
    $$SRCDIR/inverter_settings.h \
    $$SRCDIR/data_processor.h \
    $$SRCDIR/fronius_device_info.h \
    $$SRCDIR/ve_qitem_consumer.h \
    $$SRCDIR/ve_service.h \
    src/fronius_solar_api_test.h \
    src/test_helper.h \
    src/dbus_inverter_bridge_test.h \
    src/data_processor_test.h

SOURCES += \
    $$SRCDIR/froniussolar_api.cpp \
    $$SRCDIR/inverter.cpp \
    $$SRCDIR/power_info.cpp \
    $$SRCDIR/inverter_settings.cpp \
    $$SRCDIR/data_processor.cpp \
    $$SRCDIR/fronius_device_info.cpp \
    $$SRCDIR/ve_qitem_consumer.cpp \
    $$SRCDIR/ve_service.cpp \
    $$EXTDIR/googletest/src/gtest-all.cc \
    src/main.cpp \
    src/dbus_inverter_bridge_test.cpp \
    src/fronius_solar_api_test.cpp \
    src/test_helper.cpp \
    src/data_processor_test.cpp

OTHER_FILES += \
    src/fronius_sim/app.py \
    src/fronius_sim/fronius_sim.py
