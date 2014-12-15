#include <QNetworkInterface>
#include "local_ip_address_generator.h"

LocalIpAddressGenerator::LocalIpAddressGenerator():
	mPriorityOnly(false),
	mCurrent(0),
	mLast(0),
	mLocalHost(0),
	mPriorityIndex(0)
{
	reset();
}

LocalIpAddressGenerator::LocalIpAddressGenerator(
		const QList<QHostAddress> &priorityAddresses, bool priorityOnly):
	mPriorityOnly(priorityOnly),
	mCurrent(0),
	mLast(0),
	mLocalHost(0),
	mPriorityAddresses(priorityAddresses),
	mPriorityIndex(0)
{
	reset();
}

QHostAddress LocalIpAddressGenerator::next()
{
	if (mPriorityIndex < mPriorityAddresses.size()) {
		QHostAddress a = mPriorityAddresses[mPriorityIndex];
		++mPriorityIndex;
		return a;
	}
	++mCurrent;
	if (mCurrent == mLocalHost)
		++mCurrent;
	return QHostAddress(mCurrent);
}

bool LocalIpAddressGenerator::hasNext() const
{
	if (mPriorityIndex < mPriorityAddresses.size())
		return true;
	return mCurrent < mLast;
}

void LocalIpAddressGenerator::reset()
{
	mCurrent = 0;
	mLocalHost = 0;
	mLast = 0;
	mPriorityIndex = 0;
	if (mPriorityOnly) {
		return;
	}
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

bool LocalIpAddressGenerator::priorityOnly() const
{
	return mPriorityOnly;
}

void LocalIpAddressGenerator::setPriorityOnly(bool p)
{
	qDebug() << __FUNCTION__ << p;
	mPriorityOnly = p;
	reset();
}

const QList<QHostAddress> &LocalIpAddressGenerator::priorityAddresses() const
{
	return mPriorityAddresses;
}

void LocalIpAddressGenerator::setPriorityAddresses(
		const QList<QHostAddress> &addresses)
{
	qDebug() << __FUNCTION__ << addresses;
	mPriorityAddresses = addresses;
	reset();
}
