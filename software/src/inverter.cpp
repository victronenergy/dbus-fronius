#include <QsLog.h>
#include "fronius_device_info.h"
#include "inverter.h"
#include "power_info.h"

Inverter::Inverter(const QString &hostName, int port, const QString &id,
				   int deviceType, const QString &uniqueId,
				   const QString &customName, QObject *parent) :
	QObject(parent),
	mIsConnected(0),
	mHostName(hostName),
	mPort(port),
	mId(id),

	mUniqueId(uniqueId),
	mCustomName(customName),
	mDeviceInfo(FroniusDeviceInfo::find(deviceType)),
	mMeanPowerInfo(new PowerInfo(this)),
	mL1PowerInfo(new PowerInfo(this)),
	mL2PowerInfo(new PowerInfo(this)),
	mL3PowerInfo(new PowerInfo(this))
{
	if (mDeviceInfo == 0) {
		QLOG_WARN() << "Unknow inverter type:" << deviceType;
	}
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

QString Inverter::id() const
{
	return mId;
}

int Inverter::deviceType() const
{
	return mDeviceType;
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

bool Inverter::supports3Phases() const
{
	return mDeviceInfo != 0 && mDeviceInfo->supports3Phases;
}

QString Inverter::productName() const
{
	return mDeviceInfo == 0 ? QString(tr("Unknown Fronius Inverter")) :
							  mDeviceInfo->name;
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

PowerInfo *Inverter::getPowerInfo(InverterPhase phase)
{
	switch (phase) {
	case ThreePhases:
		return mMeanPowerInfo;
	case PhaseL1:
		return mL1PowerInfo;
	case PhaseL2:
		return mL2PowerInfo;
	case PhaseL3:
		return mL3PowerInfo;
	default:
		QLOG_ERROR() <<"Incorrect phase:" << phase;
		return 0;
	}
}

void Inverter::resetValues()
{
	mMeanPowerInfo->resetValues();
	mL1PowerInfo->resetValues();
	mL2PowerInfo->resetValues();
	mL3PowerInfo->resetValues();
}
