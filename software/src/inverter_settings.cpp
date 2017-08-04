#include <QsLog.h>
#include <velib/qt/ve_qitem.hpp>
#include "defines.h"
#include "inverter_settings.h"
#include "settings.h"

InverterSettings::InverterSettings(VeQItem *root, QObject *parent) :
	VeQItemConsumer(root, parent),
	mPhase(connectItem("Phase", PhaseL1, SIGNAL(phaseChanged()))),
	mPosition(connectItem("Position", Input1, SIGNAL(positionChanged()))),
	mCustomName(connectItem("CustomName", "", SIGNAL(customNameChanged()), false)),
	mIsActive(connectItem("IsActive", 1, SIGNAL(isActiveChanged()))),
	mL1Energy(connectItem("L1Energy", 0.0, 0.0, 1e6, SIGNAL(l1EnergyChanged()), true)),
	mL2Energy(connectItem("L2Energy", 0.0, 0.0, 1e6, SIGNAL(l2EnergyChanged()), true)),
	mL3Energy(connectItem("L3Energy", 0.0, 0.0, 1e6, SIGNAL(l3EnergyChanged()), true))
{
}

InverterPhase InverterSettings::phase() const
{
	return static_cast<InverterPhase>(mPhase->getValue().toInt());
}

void InverterSettings::setPhase(InverterPhase phase)
{
	mPhase->setValue(static_cast<int>(phase));
}

InverterPosition InverterSettings::position() const
{
	return static_cast<InverterPosition>(mPosition->getValue().toInt());
}

QString InverterSettings::customName() const
{
	return mCustomName->getValue().toString();
}

void InverterSettings::setCustomName(const QString &n)
{
	if (customName() == n)
		return;
	mCustomName->setValue(n);
}

bool InverterSettings::isActive() const
{
	return mIsActive->getValue().toBool();
}

double InverterSettings::l1Energy() const
{
	return getDouble(mL1Energy);
}

void InverterSettings::setL1Energy(double e)
{
	mL1Energy->setValue(e);
}

double InverterSettings::l2Energy() const
{
	return getDouble(mL2Energy);
}

void InverterSettings::setL2Energy(double e)
{
	mL2Energy->setValue(e);
}

double InverterSettings::l3Energy() const
{
	return getDouble(mL3Energy);
}

void InverterSettings::setL3Energy(double e)
{
	mL3Energy->setValue(e);
}

double InverterSettings::getEnergy(InverterPhase phase) const
{
	switch (phase) {
	case PhaseL1:
		return l1Energy();
	case PhaseL2:
		return l2Energy();
	case PhaseL3:
		return l3Energy();
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
		break;
	}
}
