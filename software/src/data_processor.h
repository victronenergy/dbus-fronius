#ifndef FRONIUSDATAPROCESSOR_H
#define FRONIUSDATAPROCESSOR_H

#include <QObject>
#include "defines.h"

class Inverter;
class InverterSettings;
struct CommonInverterData;
struct ThreePhasesInverterData;

/*!
 * @brief Converts data retrieved from Fronius inverters and stores it in
 * an `Inverter` object.
 * Fronius inverter do not export values for power and total energy per phase.
 * This class computes these values from the total overall power/energy using
 * phase voltage and current as weights.
 * In case of single phased converters, all overall values will be copied to
 * the phase selected in the `InverterSettings` object passed to the
 * constructor.
 */
class DataProcessor : public QObject
{
	Q_OBJECT
public:
	DataProcessor(Inverter *inverter, InverterSettings *settings, QObject *parent = 0);

	void process(const CommonInverterData &data);

	void process(const ThreePhasesInverterData &data);

	void updateEnergySettings();

private:
	InverterPhase getPhase() const;

	void updateEnergyValue(InverterPhase phase,
						   double accumulatedEnergy, double energyDelta);

	double getEnergyValue(InverterPhase phase);

	void updateEnergySettings(InverterPhase phase);

	Inverter *mInverter;
	InverterSettings *mSettings;
	double mPreviousTotalEnergy;
};

#endif // FRONIUSDATAPROCESSOR_H
