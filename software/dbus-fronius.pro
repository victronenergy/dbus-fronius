# Application version and revision
VERSION = 1.1.5

# suppress the mangling of va_arg has changed for gcc 4.4
QMAKE_CXXFLAGS += -Wno-psabi

# gcc 4.8 and newer don't like the QOMPILE_ASSERT in qt
QMAKE_CXXFLAGS += -Wno-unused-local-typedefs

# Add more folders to ship with the application here
unix {
    bindir = $$(bindir)
    DESTDIR = $$(DESTDIR)
    isEmpty(bindir) {
        bindir = /usr/local/bin
    }
    INSTALLS += target
    target.path = $${DESTDIR}$${bindir}
}

MOC_DIR=.moc
OBJECTS_DIR=.obj

# Note: we need the script module to parse JSON fragments as part of the
# fronius solar API.
QT += core network dbus script xml
QT -= gui

TARGET = dbus-fronius
CONFIG += console
CONFIG -= app_bundle
DEFINES += VERSION=\\\"$${VERSION}\\\"

TEMPLATE = app

include(src/json/json.pri)
include(ext/qslog/QsLog.pri)
include(ext/velib/src/qt/ve_qitems.pri)

VELIB_INC = ext/velib/inc/velib/qt
VELIB_SRC = ext/velib/src/qt

INCLUDEPATH += \
    ext/qslog \
    ext/velib/inc \
    src \
    src/modbus_tcp_client

SOURCES += \
    src/main.cpp \
    src/froniussolar_api.cpp \
    src/inverter.cpp \
    src/power_info.cpp \
    src/inverter_updater.cpp \
    src/inverter_gateway.cpp \
    src/local_ip_address_generator.cpp \
    src/settings.cpp \
    src/dbus_fronius.cpp \
    src/inverter_settings.cpp \
    src/fronius_data_processor.cpp \
    src/fronius_device_info.cpp \
    src/inverter_mediator.cpp \
    src/modbus_tcp_client/modbus_tcp_client.cpp \
    src/inverter_modbus_updater.cpp \
    src/ve_qitem_consumer.cpp \
    src/ve_qitem_init_monitor.cpp \
    src/ve_service.cpp \
    src/modbus_tcp_client/modbus_reply.cpp \
    src/modbus_tcp_client/modbus_client.cpp

HEADERS += \
    src/froniussolar_api.h \
    src/inverter.h \
    src/power_info.h \
    src/inverter_updater.h \
    src/inverter_gateway.h \
    src/local_ip_address_generator.h \
    src/settings.h \
    src/dbus_fronius.h \
    src/inverter_settings.h \
    src/defines.h \
    src/fronius_data_processor.h \
    src/fronius_device_info.h \
    src/inverter_mediator.h \
    src/velib/velib_config_app.h \
    src/modbus_tcp_client/modbus_tcp_client.h \
    src/inverter_modbus_updater.h \
    src/ve_qitem_consumer.h \
    src/ve_qitem_init_monitor.h \
    src/ve_service.h \
    src/modbus_tcp_client/modbus_reply.h \
    src/modbus_tcp_client/modbus_client.h

DISTFILES += \
    ../README.md
