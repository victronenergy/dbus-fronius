#include <QNetworkInterface>
#include <QsLog.h>
#include "local_ip_address_generator.h"

Subnet::Subnet(quint32 first, quint32 last, quint32 localhost):
	mFirst(first),
	mCurrent(first),
	mLast(last),
	mLocalHost(localhost)
{
}

bool Subnet::hasNext() const {
	return mCurrent < mLast;
}

QHostAddress Subnet::next() {
	++mCurrent;
	if (mCurrent == mLocalHost)
		++mCurrent;
	return QHostAddress(mCurrent);
}

LocalIpAddressGenerator::LocalIpAddressGenerator():
	mPriorityOnly(false),
	mNetMaskLimit(0u),
	mPriorityIndex(0),
	mSubnetIndex(0)
{
	reset();
}

LocalIpAddressGenerator::LocalIpAddressGenerator(
		const QList<QHostAddress> &priorityAddresses, bool priorityOnly):
	mPriorityOnly(priorityOnly),
	mPriorityAddresses(priorityAddresses),
	mNetMaskLimit(0u),
	mPriorityIndex(0),
	mSubnetIndex(0)
{
	reset();
}

QHostAddress LocalIpAddressGenerator::next()
{
	// First return the priority addresses
	if (mPriorityIndex < mPriorityAddresses.size()) {
		QHostAddress a = mPriorityAddresses[mPriorityIndex];
		++mPriorityIndex;
		return a;
	}

	// Then traverse the subnets
	while (mSubnetIndex < mSubnets.size()) {
		if (mSubnets[mSubnetIndex].hasNext())
			return mSubnets[mSubnetIndex].next();
		++mSubnetIndex;
	}

	Q_ASSERT(false);
	return QHostAddress((quint32)0); // We should never reach this.
}

bool LocalIpAddressGenerator::hasNext() const
{
	int idx = mSubnetIndex;
	if (mPriorityIndex < mPriorityAddresses.size())
		return true;
	while (idx < mSubnets.size()) {
		if (mSubnets[idx].hasNext())
			return true;
		++idx;
	}
	return false;
}

void LocalIpAddressGenerator::reset()
{
	mPriorityIndex = 0;
	mSubnetIndex = 0;
	mSubnets.clear();
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
					netMask |= mNetMaskLimit.toIPv4Address();

					quint32 localHost = address.toIPv4Address();
					quint32 first = localHost & netMask;
					quint32 last = (first | ~netMask) - 1;
					if (last == localHost) --last;
					mSubnets.append(
						Subnet(first, last, localHost));
				}
			}
		}
	}
}

int LocalIpAddressGenerator::progress(int activeCount) const
{
	int total = 0;
	int done = 0;
	total += mPriorityAddresses.size();
	done += mPriorityIndex;
	if (!mPriorityOnly) {
		/*!
		 * @todo EV correct for the fact that we skip localhost. This will
		 * make the value slightly more accurate.
		 */
		foreach (Subnet s, mSubnets) {
			total += s.size();
			done += s.position();
		}
	}
	if (total == 0) {
		Q_ASSERT(done == 0);
		return 0;
	}
	return (100 * qMax(0, done - activeCount)) / total;
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

QHostAddress LocalIpAddressGenerator::netMaskLimit() const
{
	return mNetMaskLimit;
}

void LocalIpAddressGenerator::setNetMaskLimit(const QHostAddress &limit)
{
	mNetMaskLimit = limit;
}
