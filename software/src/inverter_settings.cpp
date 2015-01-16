#include <QsLog.h>
#include "inverter_settings.h"

InverterSettings::InverterSettings(const QString &uniqueId, QObject *parent) :
	QObject(parent),
	mUniqueId(uniqueId),
	mPhase(PhaseL1),
	mPosition(Input1),
	mL1Energy(0),
	mL2Energy(0),
	mL3Energy(0)
{
}

QString InverterSettings::uniqueId() const
{
	return mUniqueId;
}

InverterPhase InverterSettings::phase() const
{
	return mPhase;
}

void InverterSettings::setPhase(InverterPhase phase)
{
	if (mPhase == phase)
		return;
	mPhase = phase;
	emit phaseChanged();
}

InverterPosition InverterSettings::position() const
{
	return mPosition;
}

void InverterSettings::setPosition(InverterPosition position)
{
	if (mPosition == position)
		return;
	mPosition = position;
	emit positionChanged();
	emit deviceInstanceChanged();
}

int InverterSettings::deviceInstance() const
{
	return mPosition + 20;
}

QString InverterSettings::customName() const
{
	return mCustomName;
}

void InverterSettings::setCustomName(const QString &n)
{
	if (mCustomName == n)
		return;
	mCustomName	= n;
	emit customNameChanged();
}

double InverterSettings::l1Energy() const
{
	return mL1Energy;
}

void InverterSettings::setL1Energy(double e)
{
	if (mL1Energy == e)
		return;
	mL1Energy = e;
	emit l1EnergyChanged();
}

double InverterSettings::l2Energy() const
{
	return mL2Energy;
}

void InverterSettings::setL2Energy(double e)
{
	if (mL2Energy == e)
		return;
	mL2Energy = e;
	emit l2EnergyChanged();
}

double InverterSettings::l3Energy() const
{
	return mL3Energy;
}

void InverterSettings::setL3Energy(double e)
{
	if (mL3Energy == e)
		return;
	mL3Energy = e;
	emit l3EnergyChanged();
}

double InverterSettings::getEnergy(InverterPhase phase) const
{
	switch (phase) {
	case PhaseL1:
		return mL1Energy;
	case PhaseL2:
		return mL2Energy;
	case PhaseL3:
		return mL3Energy;
	default:
		QLOG_ERROR() <<"Incorrect phase:" << phase;
		return 0;
	}
}

void InverterSettings::setEnergy(InverterPhase phase, double value)
{
	switch (phase) {
	case PhaseL1:
		setL1Energy(value);
		break;
	case PhaseL2:
		setL2Energy(value);
		break;
	case PhaseL3:
		setL3Energy(value);
		break;
	default:
		QLOG_ERROR() <<"Incorrect phase:" << phase;
	}
}
