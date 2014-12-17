#include "power_info.h"

PowerInfo::PowerInfo(QObject *parent) :
	QObject(parent),
	mCurrent(0),
	mVoltage(0),
	mPower(0)
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
