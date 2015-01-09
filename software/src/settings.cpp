#include <QsLog.h>
#include <velib/qt/v_busitem.h>
#include "settings.h"

Settings::Settings(QObject *parent) :
	QObject(parent),
	mAutoDetect(true),
	mPortNumber(80)
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

const QList<InverterSettings *> &Settings::inverterSettings() const
{
	return mInverterSettings;
}

void Settings::setInverterSettings(const QList<InverterSettings *> &settings)
{
	mInverterSettings = settings;
	emit inverterSettingsChanged();
}

InverterSettings *Settings::findInverterSettings(const QString &uniqueId)
{
	foreach (InverterSettings *i, mInverterSettings) {
		if (i->uniqueId() == uniqueId)
			return i;
	}
	return 0;
}
