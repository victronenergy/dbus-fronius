#include <limits>
#include "power_info.h"

static const double NaN = std::numeric_limits<double>::quiet_NaN();

PowerInfo::PowerInfo(QObject *parent) :
	QObject(parent),
	mCurrent(NaN),
	mVoltage(NaN),
	mPower(NaN),
	mTotalEnergy(NaN)
{
}

double PowerInfo::current() const
{
	return mCurrent;
}

void PowerInfo::setCurrent(double c)
{
	if (c == mCurrent)
		return;
	mCurrent = c;
	emit currentChanged();
}

double PowerInfo::voltage() const
{
	return mVoltage;
}

void PowerInfo::setVoltage(double v)
{
	if (v == mVoltage)
		return;
	mVoltage = v;
	emit voltageChanged();
}

double PowerInfo::power() const
{
	return mPower;
}

void PowerInfo::setPower(double p)
{
	if (p == mPower)
		return;
	mPower = p;
	emit powerChanged();
}

double PowerInfo::totalEnergy() const
{
	return mTotalEnergy;
}

void PowerInfo::setTotalEnergy(double e)
{
	if (mTotalEnergy == e)
		return;
	mTotalEnergy = e;
	emit totalEnergyChanged();
}

void PowerInfo::resetValues()
{
	setCurrent(NaN);
	setPower(NaN);
	setVoltage(NaN);
	setTotalEnergy(NaN);
}
