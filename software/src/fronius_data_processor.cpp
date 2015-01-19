#include <cmath>
#include "fronius_data_processor.h"
#include "froniussolar_api.h"
#include "inverter.h"
#include "inverter_settings.h"
#include "power_info.h"

FroniusDataProcessor::FroniusDataProcessor(Inverter *inverter,
										   InverterSettings *settings):
	mInverter(inverter),
	mSettings(settings),
	mPreviousTotalEnergy(0)
{
	Q_ASSERT(mInverter->supports3Phases() == (mSettings->phase() == ThreePhases));
}

void FroniusDataProcessor::process(const CommonInverterData &data)
{
	Q_ASSERT(mInverter->supports3Phases() == (mSettings->phase() == ThreePhases));

	PowerInfo *pi = mInverter->meanPowerInfo();
	pi->setCurrent(data.acCurrent);
	pi->setVoltage(data.acVoltage);
	pi->setPower(data.acPower);
	// Fronius gives us energy in Wh. We need kWh here.
	pi->setTotalEnergy(data.totalEnergy / 1000);
	InverterPhase phase = mSettings->phase();
	if (phase != ThreePhases) {
		PowerInfo *li = mInverter->getPowerInfo(phase);
		li->setCurrent(pi->current());
		li->setVoltage(pi->voltage());
		li->setPower(pi->power());
		li->setTotalEnergy(pi->totalEnergy());
	}
}

void FroniusDataProcessor::process(const ThreePhasesInverterData &data)
{
	/* The com.victron.system module expects power values for each phase,
	 * but the Fronius inverter does not supply them. So we take to total
	 * power - part of the CommonInverterData - and distribute it over
	 * the phases. Voltage * Current is used as weight here.
	 *
	 * Note that if there's no current/power at all the computed power
	 * values may be NaN, and in exceptional circumstances infinity. We
	 * leave the values as computed, so we can send invalid values to the
	 * DBus later.
	 */

	Q_ASSERT(mInverter->supports3Phases());
	Q_ASSERT(mSettings->phase() == ThreePhases);

	double vi1 = data.acVoltagePhase1 * data.acCurrentPhase1;
	double vi2 = data.acVoltagePhase2 * data.acCurrentPhase2;
	double vi3 = data.acVoltagePhase3 * data.acCurrentPhase3;
	double totalVi = vi1 + vi2 + vi3;
	double powerCorrection = mInverter->meanPowerInfo()->power() / totalVi;
	double totalEnergy = mInverter->meanPowerInfo()->totalEnergy();
	double energyDelta = totalEnergy - mPreviousTotalEnergy;
	if (energyDelta < 0)
		energyDelta = 0;
	double energyCorrection = energyDelta / totalVi;
	double accumulatedEnergy =
			getEnergyValue(PhaseL1) +
			getEnergyValue(PhaseL2) +
			getEnergyValue(PhaseL3);

	PowerInfo *l1 = mInverter->l1PowerInfo();
	l1->setCurrent(data.acCurrentPhase1);
	l1->setVoltage(data.acVoltagePhase1);
	l1->setPower(vi1 * powerCorrection);
	updateEnergyValue(PhaseL1, accumulatedEnergy, vi1 * energyCorrection);

	PowerInfo *l2 = mInverter->l2PowerInfo();
	l2->setCurrent(data.acCurrentPhase2);
	l2->setVoltage(data.acVoltagePhase2);
	l2->setPower(vi2 * powerCorrection);
	updateEnergyValue(PhaseL2, accumulatedEnergy, vi2 * energyCorrection);

	PowerInfo *l3 = mInverter->l3PowerInfo();
	l3->setCurrent(data.acCurrentPhase3);
	l3->setVoltage(data.acVoltagePhase3);
	l3->setPower(vi3 * powerCorrection);
	updateEnergyValue(PhaseL3, accumulatedEnergy, vi3 * energyCorrection);

	mPreviousTotalEnergy = totalEnergy;
}

void FroniusDataProcessor::updateEnergySettings()
{
	if (!mInverter->supports3Phases())
		return;
	updateEnergySettings(PhaseL1);
	updateEnergySettings(PhaseL2);
	updateEnergySettings(PhaseL3);
}

void FroniusDataProcessor::updateEnergyValue(InverterPhase phase,
											 double accumulatedEnergy,
											 double offset)
{
	if (mPreviousTotalEnergy <= 0)
		return;
	double v0 = 0;
	if (accumulatedEnergy > 0) {
		double ep = getEnergyValue(phase);
		v0 = mPreviousTotalEnergy * ep / accumulatedEnergy;
	} else {
		v0 = mPreviousTotalEnergy / 3;
	}
	v0 += offset;
	PowerInfo *pi = mInverter->getPowerInfo(phase);
	pi->setTotalEnergy(v0);
}

double FroniusDataProcessor::getEnergyValue(InverterPhase phase)
{
	PowerInfo *pi = mInverter->getPowerInfo(phase);
	if (pi == 0)
		return 0;
	double e = pi->totalEnergy();
	return std::isnormal(e) ? e : mSettings->getEnergy(phase);
}

void FroniusDataProcessor::updateEnergySettings(InverterPhase phase)
{
	PowerInfo *pi = mInverter->getPowerInfo(phase);
	if (pi == 0)
		return;
	double e = pi->totalEnergy();
	if (std::isnormal(e))
		mSettings->setEnergy(phase, e);
}
