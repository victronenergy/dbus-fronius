#include <QsLog.h>
#include <QRegExp>
#include <velib/qt/ve_qitem.hpp>
#include "defines.h"
#include "settings.h"

Settings::Settings(VeQItem *root, QObject *parent) :
	VeQItemConsumer(root, parent),
	mPortNumber(connectItem("PortNumber", 80, SIGNAL(portNumberChanged()), false)),
	mIpAddresses(connectItem("IPAddresses", "", SIGNAL(ipAddressesChanged()), false)),
	mKnownIpAddresses(connectItem("KnownIPAddresses", "", SIGNAL(knownIpAddressesChanged()), false)),
	mInverterIds(connectItem("InverterIds", "", SLOT(onInverterdIdsChanged()), false)),
	mAutoScan(connectItem("AutoScan", 1, 0))
{
}

int Settings::portNumber() const
{
	return mPortNumber->getValue().toInt();
}

QList<QHostAddress> Settings::ipAddresses() const
{
	return toAdressList(mIpAddresses->getValue().toString());
}

void Settings::setIpAddresses(const QList<QHostAddress> &addresses)
{
	mIpAddresses->setValue(fromAddressList(addresses));
}

QList<QHostAddress> Settings::knownIpAddresses() const
{
	return toAdressList(mKnownIpAddresses->getValue().toString());
}

void Settings::setKnownIpAddresses(const QList<QHostAddress> &addresses)
{
	mKnownIpAddresses->setValue(fromAddressList(addresses));
}


bool Settings::autoScan() const
{
	return mAutoScan->getValue().toBool();
}

QStringList Settings::inverterIds() const
{
	return mInverterIdCache;
}

void Settings::registerInverter(const QString &uniqueId)
{
	QString settingsId = createInverterId(uniqueId);
	if (mInverterIdCache.contains(settingsId))
		return;
	mInverterIdCache.append(settingsId);
	mInverterIds->setValue(mInverterIdCache.join(","));
	emit inverterIdsChanged();
}

int Settings::getDeviceInstance(const QString &uniqueId) const
{
	QString settingsId = createInverterId(uniqueId);
	int i = mInverterIdCache.indexOf(settingsId);
	if (i == -1)
		return -1;
	return MinDeviceInstance + i % (MaxDeviceInstance - MinDeviceInstance + 1);
}

QString Settings::createInverterId(const QString &deviceSerial)
{
	// DBus paths may only contain [A-Z][a-z][0-9]_, therefore we must
	// limit serial numbers to those.
	return QString("I%1").arg(deviceSerial).replace(
		QRegExp("[^A-Za-z0-9_]"), "_");
}

void Settings::onInverterdIdsChanged()
{
	if (!mInverterIdCache.isEmpty())
		return;
	mInverterIdCache = mInverterIds->getValue().toString().split(',', QString::SkipEmptyParts);
	if (!mInverterIdCache.isEmpty())
		emit inverterIdsChanged();
}

QList<QHostAddress> Settings::toAdressList(const QString &s) const
{
	QStringList addresses = s.split(',', QString::SkipEmptyParts);
	QList<QHostAddress> result;
	foreach (QString address, addresses)
		result.append(QHostAddress(address));
	return result;
}

QString Settings::fromAddressList(const QList<QHostAddress> &a)
{
	QString result;
	foreach(QHostAddress address, a) {
		if (!result.isEmpty())
			result.append(',');
		result.append(address.toString());
	}
	return result;
}
