#include <veutil/qt/ve_qitem.hpp>
#include "defines.h"
#include "inverter_settings.h"
#include "settings.h"
#include "logging.h"

InverterSettings::InverterSettings(VeQItem *root, QObject *parent) :
	VeQItemConsumer(root, parent),
	mPhase(connectItem("Phase", PhaseL1, SIGNAL(phaseChanged()))),
	mPhaseCount(connectItem("PhaseCount", 1, 0)),
	mPosition(connectItem("Position", Input1, SIGNAL(positionChanged()))),
	mCustomName(connectItem("CustomName", "", SIGNAL(customNameChanged()), false)),
	mIsActive(connectItem("IsActive", 1, SIGNAL(isActiveChanged()))),
	mL1Energy(connectItem("L1Energy", 0.0, 0.0, 1e6, SIGNAL(l1EnergyChanged()), true)),
	mL2Energy(connectItem("L2Energy", 0.0, 0.0, 1e6, SIGNAL(l2EnergyChanged()), true)),
	mL3Energy(connectItem("L3Energy", 0.0, 0.0, 1e6, SIGNAL(l3EnergyChanged()), true)),
	mSerialNumber(connectItem("SerialNumber", "", 0, false)),
	mLimiterSupported(connectItem("LimiterSupported", 0, 0)),
	mEnableLimiter(connectItem("EnableLimiter", 0, SIGNAL(enableLimiterChanged())))
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

void InverterSettings::setPhaseCount(int phaseCount)
{
	mPhaseCount->setValue(phaseCount);
}

InverterPosition InverterSettings::position() const
{
	return static_cast<InverterPosition>(mPosition->getValue().toInt());
}

void InverterSettings::setPosition(InverterPosition p)
{
	if (position() == p)
		return;
	mPosition->setValue(static_cast<int>(p));
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
		qCritical() <<"Incorrect phase:" << phase;
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
		qCritical() << "Incorrect phase:" << phase;
		break;
	}
}

void InverterSettings::setSerialNumber(const QString &s)
{
	mSerialNumber->setValue(s);
}

void InverterSettings::setLimiterSupported(LimiterSupport v)
{
	mLimiterSupported->setValue(v);
}

bool InverterSettings::enableLimiter() const
{
	return mEnableLimiter->getValue().toBool();
}
