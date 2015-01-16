#ifndef DEFINES_H
#define DEFINES_H

enum InverterPhase {
	/*!
	 * Inverter produces 3 phased power
	 */
	ThreePhases = 0,
	PhaseL1 = 1,
	PhaseL2 = 2,
	PhaseL3 = 3
};

enum InverterPosition {
	Input1 = 0,
	Output = 1,
	Input2 = 2
};

#endif // DEFINES_H

