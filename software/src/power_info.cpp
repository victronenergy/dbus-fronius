#include <QDebug>
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
	emit propertyChanged("current");
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
	emit propertyChanged("voltage");
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
	emit propertyChanged("power");
}
