#!/bin/bash

mkdir -p build/dbus-fronius-test
cd build/dbus-fronius-test
qmake CXX=$CXX ../../test/dbus-fronius-test.pro && make && ./dbus_fronius_test
if [[ $? -ne 0 ]] ; then
    exit 1
fi
