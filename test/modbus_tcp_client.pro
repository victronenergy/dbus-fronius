# Application version and revision
VERSION = 0.1.0

# suppress the mangling of va_arg has changed for gcc 4.4
QMAKE_CXXFLAGS += -Wno-psabi

# gcc 4.8 and newer don't like the QOMPILE_ASSERT in qt
QMAKE_CXXFLAGS += -Wno-unused-local-typedefs

MOC_DIR=.moc
OBJECTS_DIR=.obj

# Add more folders to ship with the application here
target.path = /opt/dbus_fronius_test
INSTALLS += target

QT += core network
QT -= gui

TARGET = modbus_tcp_client
CONFIG += console
CONFIG -= app_bundle
DEFINES += VERSION=\\\"$${VERSION}\\\"

TEMPLATE = app

include(../software/ext/qslog/QsLog.pri)

SRCDIR = ../software/src
CLIENTDIR = $$SRCDIR/modbus_tcp_client
APPDIR = ./modbus_tcp_client
EXTDIR = ../software/ext
VELIB_INC = $$EXTDIR/velib/inc/velib/qt
VELIB_SRC = $$EXTDIR/velib/src/qt

INCLUDEPATH += \
    ../software/src \
    ../software/ext/velib/inc

HEADERS += \
    $$CLIENTDIR/crc16.h \
    $$CLIENTDIR/modbus_client.h \
    $$CLIENTDIR/modbus_tcp_client.h \
    $$CLIENTDIR/modbus_reply.h \
    $$CLIENTDIR/modbus_rtu_client.h \
    $$APPDIR/app.h \
    $$APPDIR/arguments.h

SOURCES += \
    $$EXTDIR/velib/src/plt/serial.c \
    $$EXTDIR/velib/src/plt/posix_serial.c \
    $$EXTDIR/velib/src/plt/posix_ctx.c \
    $$EXTDIR/velib/src/types/ve_variant.c \
    $$CLIENTDIR/crc16.cpp \
    $$CLIENTDIR/modbus_client.cpp \
    $$CLIENTDIR/modbus_tcp_client.cpp \
    $$CLIENTDIR/modbus_reply.cpp \
    $$CLIENTDIR/modbus_rtu_client.cpp \
    $$APPDIR/app.cpp \
    $$APPDIR/arguments.cpp \
    $$APPDIR/main.cpp
