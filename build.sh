#!/bin/bash

mkdir -p build/dbus-fronius
cd build/dbus-fronius
qmake CXX=$CXX ../../software/dbus-fronius.pro && make
if [[ $? -ne 0 ]] ; then
    exit 1
fi
