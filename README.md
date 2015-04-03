dbus-fronius
============
This application reads data from Fronius photoelectric voltage inverters, and
publishes it on the D-Bus. It is designed to run on the color control CX.

The color control gui application contains several pages to change the behaviour
of this application, especially settings on how to detect the inverters.

In order to build the application for the color control you need a recent
version of QT creator and the ccgx SDK. To compile on a PC you will also need a
QT SDK (version 4.8.x), including QT D-Bus support. Note that QT for windows
does not support D-Bus, so you cannot build a windows executable.

Unit tests
==========

In order to run the unit tests, you need to install a python interpreter (v2.7
or newer).

Architecture
============

The application consists of 3 layers:
* Data acquisition layer:
	-	_FroniusSolarAPI_ implements the http+json protocol used to extract data
		from the inverters. The API is supported by Fronius Data Cards, which
		can be installed in most (all?) Fronius inverters.
	-	_InverterUpdater_ retrieves data via _FroniusSolarAPI_ and stores it in
		the data model (_Inverter_).
	-	_InverterGateway_ is reponsible for device detection.
* Data model
    -	_Settings_ Persistent inverter independent settings, such as the list
        of IP addresses where data cards have been found.
    -   _InverterSettings_ Persistent inverter settings.
    -	_Inverter_ Contains the latest measurements taken from an inverter.
* D-Bus layer
	-	_DBusInverterBridge_ Publishes realtime inverter data on the D-Bus. Each
		inverter has a dedicated service.
		Service name: com.victronenergy.pvinverter.fronius...
	-	_DBusSettingsBridge_ Publishes the persistent settings on the D-Bus.
		The application expects that the D-Bus server 'com.victronenergy.settings'
		is present (usually governed by the _localsettings_ script).
		Settings will be stored in the /Settings/Fronius subtree of that service.
	-	_DBusGatewayBridge_ Publishes non-persistent data related to device
		detection. Service name: com.victronenergy.fronius.
