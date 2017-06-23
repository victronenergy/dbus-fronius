#include <qnumeric.h>
#include "power_info.h"

PowerInfo::PowerInfo(VeQItem *root, QObject *parent) :
	VeService(root, parent),
	mCurrent(createItem("Current")),
	mVoltage(createItem("Voltage")),
	mPower(createItem("Power")),
	mTotalEnergy(createItem("Energy/Forward"))
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

double PowerInfo::power() const
{
	return getDouble(mPower);
}

void PowerInfo::setPower(double p)
{
	produceDouble(mPower, p, 0, "W");
}

double PowerInfo::totalEnergy() const
{
	return getDouble(mTotalEnergy);
}

void PowerInfo::setTotalEnergy(double e)
{
	produceDouble(mTotalEnergy, e, 2, "kWh");
}

void PowerInfo::resetValues()
{
	setCurrent(qQNaN());
	setPower(qQNaN());
	setVoltage(qQNaN());
	setTotalEnergy(qQNaN());
}
