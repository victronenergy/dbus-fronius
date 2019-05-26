dbus-fronius
============
This application communicates with Fronius, ABB, SolarEdge, SMA, and possibly also
other brands of PV Inverters. The data is then made available on D-Bus, the internal
Data bus in Venus OS.

The Venus OS gui application contains several pages to change the behaviour
of this application, especially settings on how to detect the inverters on the LAN.

In order to build the application for the color control you need a linux system,
a recent version of QT creator and the CCGX SDK. You can find the SDK here:

https://www.victronenergy.com/live/open_source:ccgx:setup_development_environment

This page also contains information on how to configure QT creator to use the CCGX cross compiler.
Next you can load the project file software/dbus-fronius.pro in QT creator and create the
CCGX binary.

PV Inverter Compatibility
=============

Fronius: All Fronius inverters supporting the solar API v1. Also the sunspec modbus TCP standard is
supported. If ModbusTCP is enabled on the Fronius using the web interface, then their ModbusTCP
api is used, not the JSON. Power limiting, aka Zero feed-in, only works via ModbusTCP.

ABB: both monitoring and power limiting works. Uses the ModbusTCP sunspec API.

SMA: only monitoring works. Power limiting could be done, but requires more work, as SMA doesn’t
follow the Sunspec API for that. Abandoned.

Solar Edge: only monitoring works. Its not possible to enable power limiting without help or even
a firmware change on Solar Edge side. See details below. Abandoned.

Other PV inverters that implement the sunspec standard might work as well.

Note that as the code is now, it assumes that the sunspec registers are available at unit ID
126. There is currently no mechanism to change this for the user, nor auto detection. This means
that for some brands it might be necessary to change the config in the PV Inverter to that unit ID.

More information in the CCGX manual, section PV Inverter monitoring, as well as the PV Inverter
manuals linked from there.

Sunspec quirks & more compatibility details
--------------

Fronius:
The Fronius inverter appears to be compliant with the sunspec standard
regarding the registers needed in this project. However, there may be multiple
PV inverters sharing a single IP address.  Multiple inverters are connected to
a data manager, and not directly to the network. The data manager is connected
to the network, and acts as a gateway for modbus TCP communication. Each
inverter has its own unit ID. The unit IDs are retrieved using the solar API.

SMA sunny boy:
* The Operating State (part of the inverter model) is not supported, although
  the SunSpec standard specifies it as mandatory. This is a problem, because
  the power limiter in hub4control uses this state.
* The parameters controlling power limiting are write only (SunSpec has them
  read/write). This is a problem because it is impossible to retrieve their
  current value, for example during startup. There is a workaround possible,
  but it is not part of this release.

Solar Edge:
* The SE2200H - SE6000H range (HD-wave) was specifically tested, though others
  may work too.
* The unit id is 126.
* Only one concurrent TCP connection is allowed on port 502. Additional
  connections are rejected.
* The inverter publishes only the Common model and the Inverter model. This is
  sufficient for basic operation.
* Power limiting is not possible because the inverter does not publish model
  121 (basic settings).

Development & toolchain
=====================

To compile and run on a (linux) PC you will also need a QT SDK (version 4.8.x), including QT D-Bus 
support. Because you do not have access to the system D-Bus (unless you run as root or adjust the
D-Bus configuration) you should start the fronius application with: 'dbus-fronius --dbus session'
Note that QT for windows does not support D-Bus, so you cannot build a windows executable.

The dbus-fronius executable expects the CCGX settings manager (localsettings) to be running.
localsettings is available on github:

https://github.com/victronenergy/localsettings

The README.md of localsettings contains some information on how to run localsettings on your PC.

Unit tests
==========

In order to run the unit tests, you need to install a python interpreter (v2.7 or newer).

Architecture
============

The application consists of 3 layers:
  * Data acquisition layer:
    - `FroniusSolarAPI` implements the http+json protocol used to extract data from the inverters.
    - `ModbusTcpClient` used to communicate with SunSpec PV inverters.
    - `InverterGateway` is reponsible for device detection. This actual detection is delegated to
      one of the `AbstractDetector` classes. There is one for the Solar API (`SolarApiDetector`),
      and one for SunSpec (`SunspecDetector`). PV inverters are found by sending Solar API/
      Modbus requests out to all IP addresses in the network (the maximum number of IP addresses is
      limited). IP addresses where a PV inverter has already been detected take priority. This is
      a tedious procedure which causes a lot of network travel. However, it is necessary for auto
      detection, because the inverters do not support any protocol for efficient detection (like
      upnp).
    - `InverterMediator` Each mediator represents a PV inverter found while the service is running.
      If a inverter is detected by a `InverterGateway`, the mediator will know whether the detected
      inverter is the one he represents. It is reponsible for creating and removing `Inverter`
      objects which publishes the inverter data on the D-Bus, and for starting/stopping
      communication with the inverters.
    - `DBusFronius` Ties everything together. It creates 2 inverter `InverteGateway` objects for
      device detection. If a device is found, it will create an `InverterMediator` if there is no
      mediator yet which represents the inverter. `DBusFronius` will also publish the
      `com.victronenergy.fronius` service.
  * Data model
    - `Settings` Persistent inverter independent settings (stored on `com.victronenergy.settings`),
      such as the list of IP addresses where data cards have been found. This class takes the
      relevant data from the D-Bus using VeQItems.
    - `InverterSettings` Persistent inverter settings.
    - `Inverter` contains all values published on the D-Bus.
