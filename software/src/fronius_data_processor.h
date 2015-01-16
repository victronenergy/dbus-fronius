#ifndef FRONIUSDATAPROCESSOR_H
#define FRONIUSDATAPROCESSOR_H

#include "defines.h"

class Inverter;
class InverterSettings;
struct CommonInverterData;
struct ThreePhasesInverterData;

/*!
 * @brief Converts data retrieved from Fronius inverters and stores it in
 * an `Inverter` object.
 */
class FroniusDataProcessor
{
public:
	FroniusDataProcessor(Inverter *inverter, InverterSettings *settings);

	void process(const CommonInverterData &data, bool setPhaseData);

	void process(const ThreePhasesInverterData &data);

	void updateEnergySettings();

private:
	void updateEnergyValue(InverterPhase phase,
						   double accumulatedEnergy, double offset);

	double getEnergyValue(InverterPhase phase);

	void updateEnergySettings(InverterPhase phase);

	Inverter *mInverter;
	InverterSettings *mSettings;
	double mPreviousTotalEnergy;
};

#endif // FRONIUSDATAPROCESSOR_H
