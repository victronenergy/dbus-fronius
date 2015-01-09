#include "QsLog.h"
#include "inverter_settings.h"

InverterSettings::InverterSettings(const QString &uniqueId, QObject *parent) :
	QObject(parent),
	mUniqueId(uniqueId),
	mPhase(L1),
	mPosition(Input1)
{
}

InverterSettings::~InverterSettings()
{
}

QString InverterSettings::uniqueId() const
{
	return mUniqueId;
}

InverterSettings::Phase InverterSettings::phase() const
{
	return mPhase;
}

void InverterSettings::setPhase(InverterSettings::Phase phase)
{
	if (mPhase == phase)
		return;
	mPhase = phase;
	emit phaseChanged();
}

InverterSettings::Position InverterSettings::position() const
{
	return mPosition;
}

void InverterSettings::setPosition(InverterSettings::Position position)
{
	if (mPosition == position)
		return;
	mPosition = position;
	emit positionChanged();
}
