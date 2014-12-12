#include <QNetworkInterface>
#include "local_ip_address_generator.h"

LocalIpAddressGenerator::LocalIpAddressGenerator()
{
#ifndef TARGET_ccgx
	// Workaround for testing: the generator will produce exactly one IP
	// address: the one of the testing device.
	mLocalHost = QHostAddress("127.0.0.1").toIPv4Address();
	mCurrent = QHostAddress("192.168.4.144").toIPv4Address()() - 1;
	return;
#endif
	foreach (QNetworkInterface iface, QNetworkInterface::allInterfaces()) {
		QNetworkInterface::InterfaceFlags flags = iface.flags();
		if (flags.testFlag(QNetworkInterface::IsUp) &&
			flags.testFlag(QNetworkInterface::IsRunning) &&
			!flags.testFlag(QNetworkInterface::IsLoopBack)) {
			foreach (QNetworkAddressEntry entry, iface.addressEntries()) {
				QHostAddress address = entry.ip();
				if (address != QHostAddress::Null &&
						address != QHostAddress::LocalHost &&
						address != QHostAddress::LocalHostIPv6 &&
						address != QHostAddress::Broadcast &&
						address != QHostAddress::Any &&
						address != QHostAddress::AnyIPv6 &&
						address.protocol() == QAbstractSocket::IPv4Protocol) {
					qDebug() << address << entry.netmask();
					mLocalHost = address.toIPv4Address();
					quint32 netMask = entry.netmask().toIPv4Address();
					mCurrent = mLocalHost & netMask;
					mLast = (mCurrent | ~netMask) - 1;
					if (mLast == mLocalHost)
						--mLast;
				}
			}
		}
	}
}

QHostAddress LocalIpAddressGenerator::next()
{
	++mCurrent;
	if (mCurrent == mLocalHost)
		++mCurrent;
	return QHostAddress(mCurrent);
}

bool LocalIpAddressGenerator::hasNext() const
{
	return mCurrent < mLast;
}
