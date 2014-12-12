#include "inverter.h"
#include "power_info.h"

Inverter::Inverter(const QString &hostName, int port, const QString &id,
				   const QString &uniqueId, QObject *parent) :
	QObject(parent),
	mIsConnected(0),
	mSupports3Phases(false),
	mHostName(hostName),
	mPort(port),
	mId(id),
	mUniqueId(uniqueId),
	mMeanPowerInfo(new PowerInfo(this)),
	mL1PowerInfo(new PowerInfo(this)),
	mL2PowerInfo(new PowerInfo(this)),
	mL3PowerInfo(new PowerInfo(this))
{
}

int Inverter::isConnected() const
{
	return mIsConnected;
}

void Inverter::setIsConnected(int v)
{
	if (mIsConnected == v)
		return;
	mIsConnected = v;
	emit propertyChanged("isConnected");
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
	emit propertyChanged("supports3Phases");
}

QString Inverter::id() const
{
	return mId;
}

QString Inverter::uniqueId() const
{
	return mUniqueId;
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
