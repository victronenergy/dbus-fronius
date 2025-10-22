#include <Qt>
#include <QStringList>
#include <QRegularExpression>
#include <veutil/qt/ve_qitem.hpp>
#include "defines.h"
#include "settings.h"

Settings::Settings(VeQItem *root, QObject *parent) :
	VeQItemConsumer(root, parent),
	mPortNumber(connectItem("PortNumber", 80, SIGNAL(portNumberChanged()), false)),
	mIpAddresses(connectItem("IPAddresses", "", SIGNAL(ipAddressesChanged()), false)),
	mKnownIpAddresses(connectItem("KnownIPAddresses", "", 0, false)),
	mAutoScan(connectItem("AutoScan", 1, 0)),
	mIdBySerial(connectItem("IdentifyBySerialNumber", 0, 0)),
	mModbusSlaveAddress(connectItem("ModbusSlaveAddress", 126, 0, 247))
{
}

int Settings::portNumber() const
{
	return mPortNumber->getValue().toInt();
}

int Settings::modbusSlaveAddress() const
{
	return mModbusSlaveAddress->getValue().toInt();
}

QList<QHostAddress> Settings::ipAddresses() const
{
	return toAdressList(mIpAddresses->getValue().toString());
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

int Settings::registerInverter(const QString &uniqueId)
{
	QString settingsId = createInverterId(uniqueId);
	return getDeviceInstance(settingsId, "pvinverter", MinDeviceInstance);
}

QString Settings::createInverterId(const QString &deviceSerial)
{
	// DBus paths may only contain [A-Z][a-z][0-9]_, therefore we must
	// limit serial numbers to those.
	return QString("I%1").arg(deviceSerial).replace(
		QRegularExpression("[^A-Za-z0-9_]"), "_");
}

QList<QHostAddress> Settings::toAdressList(const QString &s) const
{
	QStringList addresses = s.split(',', Qt::SkipEmptyParts);
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
