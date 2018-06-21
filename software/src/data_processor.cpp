#include <cmath>
#include "data_processor.h"
#include "froniussolar_api.h"
#include "inverter.h"
#include "inverter_settings.h"
#include "power_info.h"

DataProcessor::DataProcessor(Inverter *inverter, InverterSettings *settings):
	mInverter(inverter),
	mSettings(settings),
	mPreviousTotalEnergy(-1)
{
}

void DataProcessor::process(const CommonInverterData &data)
{
	BasicPowerInfo *pi = mInverter->meanPowerInfo();
	pi->setPower(data.acPower);
	// Fronius gives us energy in Wh. We need kWh here.
	pi->setTotalEnergy(data.totalEnergy / 1000);
	InverterPhase phase = getPhase();
	if (phase != MultiPhase) {
		PowerInfo *li = mInverter->getPowerInfo(phase);
		li->setCurrent(data.acCurrent);
		li->setVoltage(data.acVoltage);
		li->setPower(pi->power());
		li->setTotalEnergy(pi->totalEnergy());
	}
}

void DataProcessor::process(const ThreePhasesInverterData &data)
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

	Q_ASSERT(getPhase() == MultiPhase);

	const DeviceInfo &deviceInfo = mInverter->deviceInfo();

	double vi1 = data.acVoltagePhase1 * data.acCurrentPhase1;
	double vi2 = data.acVoltagePhase2 * data.acCurrentPhase2;
	double vi3 = data.acVoltagePhase3 * data.acCurrentPhase3;
	double totalEnergy = mInverter->meanPowerInfo()->totalEnergy();
	double energyDelta = totalEnergy - mPreviousTotalEnergy;
	if (energyDelta < 0)
		energyDelta = 0;
	double totalVi = vi1 + vi2;
	double accumulatedEnergy =
			getEnergyValue(PhaseL1) +
			getEnergyValue(PhaseL2);
	if (deviceInfo.phaseCount > 2) {
		accumulatedEnergy += getEnergyValue(PhaseL3);
		totalVi += vi3;
	}
	double powerCorrection = 0;
	double energyCorrection = 0;
	if (totalVi > 0) {
		powerCorrection = mInverter->meanPowerInfo()->power() / totalVi;
		energyCorrection = energyDelta / totalVi;
	}

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

	if (deviceInfo.phaseCount > 2) {
		PowerInfo *l3 = mInverter->l3PowerInfo();
		l3->setCurrent(data.acCurrentPhase3);
		l3->setVoltage(data.acVoltagePhase3);
		l3->setPower(vi3 * powerCorrection);
		updateEnergyValue(PhaseL3, accumulatedEnergy, vi3 * energyCorrection);
	}

	mPreviousTotalEnergy = totalEnergy;
}

void DataProcessor::updateEnergySettings()
{
	updateEnergySettings(PhaseL1);
	updateEnergySettings(PhaseL2);
	updateEnergySettings(PhaseL3);
}

InverterPhase DataProcessor::getPhase() const
{
	return mInverter->deviceInfo().phaseCount > 1 ? MultiPhase : mSettings->phase();
}

void DataProcessor::updateEnergyValue(InverterPhase phase, double accumulatedEnergy,
									  double energyDelta)
{
	if (mPreviousTotalEnergy < 0)
		return;
	double phaseEnergy = 0;
	if (accumulatedEnergy > 0) {
		double prevPhaseEnergy = getEnergyValue(phase);
		// p0 is calculated as the weighted fraction of mPreviousTotalEnergy.
		// The weight is ep / accumulatedEnergy.
		// We do this instead of simply incrementing 'phaseEnergy' with
		// 'energyDelta' to prevent that the sum of the phase energy value
		// drifts away from the toal energy value.
		phaseEnergy = mPreviousTotalEnergy * prevPhaseEnergy / accumulatedEnergy;
	} else {
		phaseEnergy = mPreviousTotalEnergy / mInverter->deviceInfo().phaseCount;
	}
	phaseEnergy += energyDelta;
	PowerInfo *pi = mInverter->getPowerInfo(phase);
	pi->setTotalEnergy(phaseEnergy);
}

double DataProcessor::getEnergyValue(InverterPhase phase)
{
	PowerInfo *pi = mInverter->getPowerInfo(phase);
	if (pi == 0)
		return 0;
	double e = pi->totalEnergy();
	return std::isnormal(e) ? e : mSettings->getEnergy(phase);
}

void DataProcessor::updateEnergySettings(InverterPhase phase)
{
	PowerInfo *pi = mInverter->getPowerInfo(phase);
	if (pi == 0)
		return;
	double e = pi->totalEnergy();
	if (std::isnormal(e))
		mSettings->setEnergy(phase, e);
}
