#include "inverter.h"
#include "power_info.h"

Inverter::Inverter(const QString &hostName, int port, const QString &id,
				   const QString &uniqueId, const QString &customName, QObject *parent) :
	QObject(parent),
	mIsConnected(0),
	mSupports3Phases(false),
	mHostName(hostName),
	mPort(port),
	mId(id),
	mUniqueId(uniqueId),
	mCustomName(customName),
	mMeanPowerInfo(new PowerInfo(this)),
	mL1PowerInfo(new PowerInfo(this)),
	mL2PowerInfo(new PowerInfo(this)),
	mL3PowerInfo(new PowerInfo(this))
{
}

bool Inverter::isConnected() const
{
	return mIsConnected;
}

void Inverter::setIsConnected(bool v)
{
	if (mIsConnected == v)
		return;
	mIsConnected = v;
	emit isConnectedChanged();
}

QString Inverter::status() const
{
	return mStatus;
}

void Inverter::setStatus(const QString &c)
{
	if (mStatus == c)
		return;
	mStatus = c;
	emit statusChanged();
}

bool Inverter::supports3Phases() const
{
	return mSupports3Phases;
}

void Inverter::setSupports3Phases(bool p)
{
	if (mSupports3Phases == p)
		return;
	mSupports3Phases = p;
	emit supports3PhasesChanged();
}

QString Inverter::id() const
{
	return mId;
}

QString Inverter::uniqueId() const
{
	return mUniqueId;
}

QString Inverter::customName() const
{
	return mCustomName;
}

QString Inverter::hostName() const
{
	return mHostName;
}

int Inverter::port() const
{
	return mPort;
}

PowerInfo *Inverter::meanPowerInfo()
{
	return mMeanPowerInfo;
}

PowerInfo *Inverter::l1PowerInfo()
{
	return mL1PowerInfo;
}

PowerInfo *Inverter::l2PowerInfo()
{
	return mL2PowerInfo;
}

PowerInfo *Inverter::l3PowerInfo()
{
	return mL3PowerInfo;
}

void Inverter::resetValues()
{
	mMeanPowerInfo->resetValues();
	mL1PowerInfo->resetValues();
	mL2PowerInfo->resetValues();
	mL3PowerInfo->resetValues();
}
