#include <Qt>
#include <velib/qt/ve_qitem.hpp>
#include "defines.h"
#include "settings.h"

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
#include <QRegularExpression>
#else
#include <QRegExp>
#define QRegularExpression QRegExp
#endif

#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
#define SkipEmptyParts Qt::SkipEmptyParts
#else
#define SkipEmptyParts QString::SkipEmptyParts
#endif


Settings::Settings(VeQItem *root, QObject *parent) :
	VeQItemConsumer(root, parent),
	mPortNumber(connectItem("PortNumber", 80, SIGNAL(portNumberChanged()), false)),
	mIpAddresses(connectItem("IPAddresses", "", SIGNAL(ipAddressesChanged()), false)),
	mKnownIpAddresses(connectItem("KnownIPAddresses", "", 0, false)),
	mInverterIds(connectItem("InverterIds", "", SLOT(onInverterdIdsChanged()), false)),
	mAutoScan(connectItem("AutoScan", 1, 0)),
	mIdBySerial(connectItem("IdentifyBySerialNumber", 0, 0))
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

bool Settings::idBySerial() const
{
	return mIdBySerial->getValue().toBool();
}

QStringList Settings::inverterIds() const
{
	return mInverterIdCache;
}

int Settings::registerInverter(const QString &uniqueId)
{
	QString settingsId = createInverterId(uniqueId);
	int i = getDeviceInstance(settingsId, "pvinverter", MinDeviceInstance);

	if (!mInverterIdCache.contains(settingsId)) {
		mInverterIdCache.append(settingsId);
		mInverterIds->setValue(mInverterIdCache.join(","));
	}
	return i;
}

QString Settings::createInverterId(const QString &deviceSerial)
{
	// DBus paths may only contain [A-Z][a-z][0-9]_, therefore we must
	// limit serial numbers to those.
	return QString("I%1").arg(deviceSerial).replace(
		QRegularExpression("[^A-Za-z0-9_]"), "_");
}

void Settings::onInverterdIdsChanged()
{
	if (!mInverterIdCache.isEmpty())
		return;
	mInverterIdCache = mInverterIds->getValue().toString().split(',', SkipEmptyParts);
}

QList<QHostAddress> Settings::toAdressList(const QString &s) const
{
	QStringList addresses = s.split(',', SkipEmptyParts);
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
