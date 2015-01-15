dbus-fronius
============
This application reads data from Fronius photoelectric voltage inverters, and
publishes it on the D-Bus. It is designed to run on the color control CX.

The color control gui application contains several pages to change the behaviour
of this application, especially settings on how to detect the inverters.

In order to build the application for the color control you need a recent
version of QT creator and the ccgx SDK. To compile on a PC you will also need a
QT SDK (version 4.8.x), including QT D-Bus support. The latter means that you
cannot compile the application under windows.

Unit tests
==========

In order to run the unit tests, you need to install a python interpreter (v2.7
or newer).
