#include <QNetworkInterface>
#include "local_ip_address_generator.h"

Subnet::Subnet(LocalIpAddressGenerator *generator, quint32 first, quint32 last, quint32 localhost):
	mGenerator(generator),
	mFirst(first),
	mCurrent(first),
	mLast(last),
	mLocalHost(localhost)
{
	next();
}

bool Subnet::hasNext() const
{
	return mCurrent <= mLast;
}

QHostAddress Subnet::next()
{
	quint32 current = mCurrent;
	++mCurrent;
	while ((mCurrent <= mLast) && (
		(mCurrent == mLocalHost) || mGenerator->exceptions().contains(QHostAddress(mCurrent)))) {
		++mCurrent;
	}
	return QHostAddress(current);
}

int Subnet::position() const
{
	return mCurrent - mFirst;
}

LocalIpAddressGenerator::LocalIpAddressGenerator():
	mPriorityOnly(false),
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
					qInfo() << "IP Address:" << address
							<< "Netmask:" << entry.netmask();
					quint32 netMask = entry.netmask().toIPv4Address();
					netMask |= QHostAddress(0xFFFFF000).toIPv4Address();

					quint32 localHost = address.toIPv4Address();
					// For link-local, scan only 169.254.0.180. This
					// is the static address used by Fronius inverters.
					if ((localHost & 0xffff0000) == 0xa9fe0000) {
						mSubnets.append(
							Subnet(this, 0xa9fe00b3, 0xa9fe00b4, localHost));
					} else {
						quint32 first = localHost & netMask;
						quint32 last = (first | ~netMask) - 1;
						mSubnets.append(
							Subnet(this, first, last, localHost));
					}
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

void LocalIpAddressGenerator::setPriorityOnly(bool p)
{
	if (mPriorityOnly == p)
		return;
	mPriorityOnly = p;
	if (p)
		reset();
}

const QSet<QHostAddress> LocalIpAddressGenerator::exceptions() const
{
	// We exclude scanning of the priorityAddresses when doing a sweep
	// since they were already scanned.
	return QSet<QHostAddress>(mPriorityAddresses.begin(), mPriorityAddresses.end());
}

void LocalIpAddressGenerator::setPriorityAddresses(
		const QList<QHostAddress> &addresses)
{
	bool atEnd = mPriorityIndex > 0 &&
			mPriorityIndex >= mPriorityAddresses.size();
	mPriorityAddresses = addresses;
	mPriorityIndex = atEnd ? mPriorityAddresses.size() : 0;
}
