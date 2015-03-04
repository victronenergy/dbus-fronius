#ifndef DEFINES_H
#define DEFINES_H

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

/// This value is used to indicate that the correct device instance has not
/// been set yet.
const int InvalidDeviceInstance = -1;
const int MinDeviceInstance = 20;
const int MaxDeviceInstance = 79;

#endif // DEFINES_H
