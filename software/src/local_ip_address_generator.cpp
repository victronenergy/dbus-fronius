#include <QNetworkInterface>
#include <QsLog.h>
#include "local_ip_address_generator.h"

LocalIpAddressGenerator::LocalIpAddressGenerator():
	mPriorityOnly(false),
	mFirst(0),
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
	mFirst(0),
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
	mFirst = 0;
	mCurrent = 0;
	mLocalHost = 0;
	mLast = 0;
	mPriorityIndex = 0;
	if (mPriorityOnly)
		return;
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
					QLOG_INFO() << "IP Address:" << address
								<< "Netmask:" << entry.netmask();
					quint32 netMask = entry.netmask().toIPv4Address();
					mLocalHost = address.toIPv4Address();
					mCurrent = mLocalHost & netMask;
					mFirst = mCurrent;
					mLast = (mCurrent | ~netMask) - 1;
					if (mLast == mLocalHost)
						--mLast;
				}
			}
		}
	}
}

int LocalIpAddressGenerator::progress() const
{
	int total = 0;
	int done = 0;
	total += mPriorityAddresses.size();
	done += mPriorityIndex;
	if (!mPriorityOnly) {
		/*!
		 * @todo EV correct for the fact that we skip mLocalHost. This will
		 * make the value slightly more accurate.
		 */
		total += mLast - mFirst;
		done += mCurrent - mFirst;
	}
	if (total == 0) {
		Q_ASSERT(done == 0);
		return 0;
	}
	return (100 * done) / total;
}

bool LocalIpAddressGenerator::priorityOnly() const
{
	return mPriorityOnly;
}

void LocalIpAddressGenerator::setPriorityOnly(bool p)
{
	if (mPriorityOnly == p)
		return;
	mPriorityOnly = p;
	if (p)
		reset();
}

const QList<QHostAddress> &LocalIpAddressGenerator::priorityAddresses() const
{
	return mPriorityAddresses;
}

void LocalIpAddressGenerator::setPriorityAddresses(
		const QList<QHostAddress> &addresses)
{
	bool atEnd = mPriorityIndex > 0 &&
			mPriorityIndex >= mPriorityAddresses.size();
	mPriorityAddresses = addresses;
	mPriorityIndex = atEnd ? mPriorityAddresses.size() : 0;
}
