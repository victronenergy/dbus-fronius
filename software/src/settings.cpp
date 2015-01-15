#include <QsLog.h>
#include <velib/qt/v_busitem.h>
#include "settings.h"

Settings::Settings(QObject *parent) :
	QObject(parent),
	mPortNumber(80)
{
}

int Settings::portNumber() const
{
	return mPortNumber;
}

void Settings::setPortNumber(int p)
{
	if (mPortNumber == p)
		return;
	mPortNumber = p;
	emit portNumberChanged();
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
