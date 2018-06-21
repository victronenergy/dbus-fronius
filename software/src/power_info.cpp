#include <qnumeric.h>
#include "power_info.h"

BasicPowerInfo::BasicPowerInfo(VeQItem *root, QObject *parent) :
	VeService(root, parent),
	mPower(createItem("Power")),
	mTotalEnergy(createItem("Energy/Forward"))
{
}

double BasicPowerInfo::power() const
{
	return getDouble(mPower);
}

void BasicPowerInfo::setPower(double p)
{
	produceDouble(mPower, p, 0, "W");
}

double BasicPowerInfo::totalEnergy() const
{
	return getDouble(mTotalEnergy);
}

void BasicPowerInfo::setTotalEnergy(double e)
{
	produceDouble(mTotalEnergy, e, 2, "kWh");
}

void BasicPowerInfo::resetValues()
{
	setPower(qQNaN());
	setTotalEnergy(qQNaN());
}

PowerInfo::PowerInfo(VeQItem *root, QObject *parent) :
	BasicPowerInfo(root, parent),
	mCurrent(createItem("Current")),
	mVoltage(createItem("Voltage"))
{
}

double PowerInfo::current() const
{
	return getDouble(mCurrent);
}

void PowerInfo::setCurrent(double c)
{
	produceDouble(mCurrent, c, 1, "A");
}

double PowerInfo::voltage() const
{
	return getDouble(mVoltage);
}

void PowerInfo::setVoltage(double v)
{
	produceDouble(mVoltage, v, 0, "V");
}

void PowerInfo::resetValues()
{
	BasicPowerInfo::resetValues();
	setCurrent(qQNaN());
	setVoltage(qQNaN());
}
