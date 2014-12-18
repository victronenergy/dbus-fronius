#include <QsLog.h>
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
	emit autoDetectChanged();
}

const QList<QHostAddress> &Settings::ipAddresses() const
{
	return mIpAddresses;
}

void Settings::setIpAddresses(const QList<QHostAddress> &addresses)
{
	if (mIpAddresses == addresses)
		return;
	mIpAddresses = addresses;
	emit ipAddressesChanged();
}

const QList<QHostAddress> &Settings::knownIpAddresses() const
{
	return mKnownIpAddresses;
}

void Settings::setKnownIpAddresses(const QList<QHostAddress> &addresses)
{
	if (mKnownIpAddresses == addresses)
		return;
	mKnownIpAddresses = addresses;
	emit knownIpAddressesChanged();
}
