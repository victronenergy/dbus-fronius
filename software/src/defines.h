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

enum LimiterSupport {
	LimiterDisabled = 0,
	LimiterEnabled = 1,
	LimiterForcedDisabled = 2,
	LimiterForcedEnabled = 3
};

enum ProtocolType {
	ProtocolFroniusSolarApi,
	ProtocolSunSpecFloat,
	ProtocolSunSpecIntSf,
	ProtocolSunSpec2018 // 700 series models
};

struct DeviceInfo
{
	DeviceInfo():
		modbusPort(502),
		networkId(0),
		port(0),
		deviceType(0),
		phaseCount(0),
		productId(0),
		retrievalMode(ProtocolFroniusSolarApi),
		inverterModelOffset(0),
		inverterModel(0),
		immediateControlOffset(0),
		immediateControlModel(0),
		trackerModelOffset(0),
		numberOfTrackers(0),
		powerLimitScale(0),
		trackerVoltageScale(0),
		trackerPowerScale(0),
		maxPower(0),
		storageCapacity(0)
	{}

	QString hostName;
	QString uniqueId;
	QString productName;
	QString dataManagerVersion;
	QString firmwareVersion;
	QString serialNumber;
	int modbusPort;
	int networkId;
	int port; // Fronius solar API only
	int deviceType; // Fronius solar API only
	int phaseCount;
	int productId;
	ProtocolType retrievalMode;
	// Sunspec only
	quint16 inverterModelOffset;
	quint16 inverterModel;
	quint16 immediateControlOffset;
	quint16 immediateControlModel; // What model to use for control, 123/704
	quint16 trackerModelOffset;
	int numberOfTrackers;
	double powerLimitScale;
	double trackerVoltageScale;
	double trackerPowerScale;
	double maxPower;
	double storageCapacity; // SMA SunnyIsland will report a storage capacity
};

/// This value is used to indicate that the correct device instance has not
/// been set yet.
const int MinDeviceInstance = 20;

#endif // DEFINES_H
