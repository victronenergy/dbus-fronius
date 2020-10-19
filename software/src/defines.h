#ifndef DEFINES_H
#define DEFINES_H

#include <QString>

enum InverterPhase {
	/*!
	 * Inverter produces 3 phased power
	 */
	MultiPhase = 0,
	PhaseL1 = 1,
	PhaseL2 = 2,
	PhaseL3 = 3
};

enum InverterPosition {
	Input1 = 0,
	Output = 1,
	Input2 = 2
};

enum ProtocolType {
	ProtocolFroniusSolarApi,
	ProtocolSunSpecFloat,
	ProtocolSunSpecIntSf
};

struct DeviceInfo
{
	DeviceInfo():
		networkId(0),
		port(0),
		deviceType(0),
		phaseCount(0),
		productId(0),
		retrievalMode(ProtocolFroniusSolarApi),
		inverterModelOffset(0),
		namePlateModelOffset(0),
		immediateControlOffset(0),
		powerLimitScale(0),
		maxPower(0),
		storageCapacity(0)
	{}

	QString hostName;
	QString uniqueId;
	QString productName;
	QString firmwareVersion;
	QString serialNumber;
	int networkId;
	int port; // Fronius solar API only
	int deviceType; // Fronius solar API only
	int phaseCount;
	int productId;
	ProtocolType retrievalMode;
	// Sunspec only
	quint16 inverterModelOffset;
	quint16 namePlateModelOffset;
	quint16 immediateControlOffset;
	double powerLimitScale;
	double maxPower;
	double storageCapacity; // SMA SunnyIsland will report a storage capacity
};

/// This value is used to indicate that the correct device instance has not
/// been set yet.
const int InvalidDeviceInstance = -1;
const int MinDeviceInstance = 20;
const int MaxDeviceInstance = 79;

#endif // DEFINES_H
