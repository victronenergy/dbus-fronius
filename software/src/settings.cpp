#include <velib/qt/v_busitem.h>
#include "settings.h"

Settings::Settings(QObject *parent) :
	QObject(parent),
	mAutoDetect(true)
{
}

bool Settings::autoDetect() const
{
	return mAutoDetect;
}

void Settings::setAutoDetect(bool a)
{
	if (mAutoDetect == a)
		return;
	mAutoDetect = a;
	emit propertyChanged("autoDetect");
}

const QList<QHostAddress> &Settings::ipAddresses() const
{
	return mIpAddresses;
}

void Settings::setIpAddresses(const QList<QHostAddress> &addresses)
{
	mIpAddresses = addresses;
	emit propertyChanged("ipAddresses");
}

const QList<QHostAddress> &Settings::knownIpAddresses() const
{
	return mKnownIpAddresses;
}

void Settings::setKnownIpAddresses(const QList<QHostAddress> &addresses)
{
	mKnownIpAddresses = addresses;
	emit propertyChanged("knownIpAddresses");
}
