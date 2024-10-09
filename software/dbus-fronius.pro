# Application version and revision
VERSION = 1.7.3

# suppress the mangling of va_arg has changed for gcc 4.4
QMAKE_CXXFLAGS += -Wno-psabi

# gcc 4.8 and newer don't like the QOMPILE_ASSERT in qt
QMAKE_CXXFLAGS += -Wno-unused-local-typedefs

# Silence a heap of moaning about internal QT4 definitions
equals(QT_MAJOR_VERSION, 4): QMAKE_CXXFLAGS += -Wno-deprecated-copy -Wno-class-memaccess

!lessThan(QT_VERSION, 5) {
    QMAKE_CXXFLAGS += "-Wsuggest-override"
    CONFIG(debug, debug|release) {
        QMAKE_CXXFLAGS += "-Werror=suggest-override"
    }
}

equals(QT_MAJOR_VERSION, 6): QMAKE_CXXFLAGS += -std=c++17

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

QT += core network dbus xml
QT -= gui

TARGET = dbus-fronius
CONFIG += console
CONFIG -= app_bundle
DEFINES += VERSION=\\\"$${VERSION}\\\"

TEMPLATE = app

include(ext/veutil/veutil.pri)

# Fronius SolarAPI still uses QHttp, and porting to QT5 is not possible
# because QNetworkAccessManager is a CPU hog.
equals(QT_MAJOR_VERSION, 5): include(src/qhttp/qhttp.pri)
equals(QT_MAJOR_VERSION, 6): include(src/qhttp/qhttp.pri)

# QT4 needs external json library
equals(QT_MAJOR_VERSION, 4): include(src/json/json.pri)

INCLUDEPATH += \
    ext/qslog \
    src \
    src/modbus_tcp_client

SOURCES += \
    src/logging.cpp \
    src/main.cpp \
    src/froniussolar_api.cpp \
    src/inverter.cpp \
    src/fronius_inverter.cpp \
    src/power_info.cpp \
    src/inverter_gateway.cpp \
    src/local_ip_address_generator.cpp \
    src/settings.cpp \
    src/dbus_fronius.cpp \
    src/inverter_settings.cpp \
    src/fronius_device_info.cpp \
    src/inverter_mediator.cpp \
    src/modbus_tcp_client/modbus_tcp_client.cpp \
    src/ve_qitem_consumer.cpp \
    src/ve_qitem_init_monitor.cpp \
    src/ve_service.cpp \
    src/abstract_detector.cpp \
    src/solar_api_detector.cpp \
    src/sunspec_detector.cpp \
    src/fronius_udp_detector.cpp \
    src/modbus_tcp_client/modbus_reply.cpp \
    src/modbus_tcp_client/modbus_client.cpp \
    src/sunspec_tools.cpp \
    src/gateway_interface.cpp \
    src/sunspec_updater.cpp \
    src/solar_api_updater.cpp \
    src/data_processor.cpp \
    src/solaredge_limiter.cpp

HEADERS += \
    src/compat.h \
    src/logging.h \
    src/froniussolar_api.h \
    src/inverter.h \
    src/fronius_inverter.h \
    src/power_info.h \
    src/inverter_gateway.h \
    src/local_ip_address_generator.h \
    src/settings.h \
    src/dbus_fronius.h \
    src/inverter_settings.h \
    src/defines.h \
    src/fronius_device_info.h \
    src/inverter_mediator.h \
    src/modbus_tcp_client/modbus_tcp_client.h \
    src/ve_qitem_consumer.h \
    src/ve_qitem_init_monitor.h \
    src/ve_service.h \
    src/abstract_detector.h \
    src/solar_api_detector.h \
    src/sunspec_detector.h \
    src/fronius_udp_detector.h \
    src/modbus_tcp_client/modbus_reply.h \
    src/modbus_tcp_client/modbus_client.h \
    src/sunspec_tools.h \
    src/gateway_interface.h \
    src/sunspec_updater.h \
    src/solar_api_updater.h \
    src/data_processor.h \
    src/solaredge_limiter.h

DISTFILES += \
    ../README.md
