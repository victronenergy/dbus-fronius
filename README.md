dbus-fronius
============
This application communicates with Fronius, ABB, SolarEdge, SMA, and possibly also
other brands of PV Inverters. The data is then made available on D-Bus, the internal
Data bus in Venus OS.

The Venus OS gui application contains several pages to change the behaviour
of this application, especially settings on how to detect the inverters on the LAN.

In order to build the application, you need a linux system,
a recent version of QT creator, the Venus OS SDK and a not publicly available
library called velib.

You can find the SDK here:

https://github.com/victronenergy/venus/wiki

The there linked documentation also contains information on how to configure QT creator to use the
cross compiler that comes with the SDK.

Next you can load the project file software/dbus-fronius.pro in QT creator and create the binary.

PV Inverter Compatibility
=============

## Fronius
All Fronius inverters supporting the solar API v1 are supported and work out
of the box, once properly commissioned. For now, this also includes really old
models like the IG plus.

The sunspec modbus TCP standard is supported. If ModbusTCP is enabled during
commissioning, then the ModbusTCP api is used instead of SolarAPI.  Power
limiting only works via ModbusTCP.

It is recommended that the SunSpec api is used whenever possible. It is faster,
and there are less problems with correctly detecting the number of phases.

Users of GEN24 models are advised to *always* turn on SunSpec. This is because
without SunSpec support, it is impossible to distinguish between a GEN24
and a Tauro.

## ABB
Monitoring and power limiting works automatically. The ModbusTCP sunspec
API is used.

## SMA
Power limiting works for newer inverters, but has to be explicitly enabled on
the GX device. Older Sunnyboy inverters were not tested, but can be monitored.
The limiting implementation is SMA specific and not Sunspec compliant, because
SMA doesn't allow cyclic writing of *WMaxLimEna*. SMA inverters need to be
configured manually to allow limiting through modbus.

## Solar Edge

All models can be monitored. Limiting is supported for some models. SolarEdge's
proprietary modbus protocol is used to do limiting, where supported.

 * The SE2200H - SE6000H range (HD-wave) was specifically tested.
 * Some models allow only one concurrent TCP connection on port 502. Additional
   connections are rejected.
 * US models currently lack Frequency Control, and are therefore not compatible.
 * The UnitId is 126.

For limiting support:
 * For SetApp (screenless) inverters: Firmware 4.8.24 or higher is required.
 * For LCD inverters: Firmware 3.25xx or higher is required.

## Other PV inverters

PV inverters that implement at least sunspec information models 1, 101 or 103,
and 120, might work as well. Limiting support is only available if information
models 123 or 704 are implemented. Limiting has to be explicitly enabled on the
GX device.

Note that as the code is now, it assumes that the sunspec registers are
available at unit ID 126. There is currently no mechanism to change this for
the user, nor auto detection. This means that for some brands it might be
necessary to change the config in the PV Inverter to that unit ID.

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

Development & toolchain
=====================

To compile and run on a (linux) PC you will also need a QT SDK (version 6.6.x), including QT D-Bus 
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
