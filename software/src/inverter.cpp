#include <qnumeric.h>
#include <QsLog.h>
#include "fronius_device_info.h"
#include "inverter.h"
#include "power_info.h"

Inverter::Inverter(const QString &hostName, int port, const QString &id,
				   int deviceType, const QString &uniqueId,
				   const QString &customName, QObject *parent) :
	QObject(parent),
	mIsConnected(0),
	mErrorCode(0),
	mStatusCode(0),
	mHostName(hostName),
	mPort(port),
	mId(id),
	mDeviceType(deviceType),
	mUniqueId(uniqueId),
	mCustomName(customName),
	mDeviceInstance(InvalidDeviceInstance),
	mDeviceInfo(FroniusDeviceInfo::find(deviceType)),
	mMeanPowerInfo(new PowerInfo(this)),
	mL1PowerInfo(new PowerInfo(this)),
	mL2PowerInfo(new PowerInfo(this)),
	mL3PowerInfo(new PowerInfo(this)),
	mPowerLimit(qQNaN()),
	mMaxPower(qQNaN())
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

int Inverter::errorCode() const
{
	return mErrorCode;
}

void Inverter::setErrorCode(int code)
{
	if (mErrorCode == code)
		return;
	mErrorCode = code;
	emit errorCodeChanged();
}

int Inverter::statusCode() const
{
	return mStatusCode;
}

void Inverter::setStatusCode(int code)
{
	if (mStatusCode == code)
		return;
	mStatusCode = code;
	emit statusCodeChanged();
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

void Inverter::setHostName(const QString &h)
{
	if (mHostName == h)
		return;
	mHostName = h;
	emit hostNameChanged();
}

int Inverter::port() const
{
	return mPort;
}

void Inverter::setPort(int p)
{
	if (mPort == p)
		return;
	mPort = p;
	emit portChanged();
}

int Inverter::phaseCount() const
{
	return mDeviceInfo == 0 ? 1 : mDeviceInfo->phaseCount;
}

QString Inverter::productName() const
{
	return mDeviceInfo == 0 ? QString(tr("Unknown Fronius Inverter")) :
							  mDeviceInfo->name;
}

int Inverter::deviceInstance() const
{
	return mDeviceInstance;
}

void Inverter::setDeviceInstance(int instance)
{
	if (mDeviceInstance == instance)
		return;
	mDeviceInstance = instance;
	emit deviceInstanceChanged();
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
	case MultiPhase:
		return mMeanPowerInfo;
	case PhaseL1:
		return mL1PowerInfo;
	case PhaseL2:
		return mL2PowerInfo;
	case PhaseL3:
		return mL3PowerInfo;
	default:
		QLOG_ERROR() << "Incorrect phase:" << phase;
		return 0;
	}
}

void Inverter::setPowerLimit(double p)
{
//	if (mPowerLimit == p)
//		return;
	mPowerLimit = p;
	emit powerLimitChanged();
}

void Inverter::setRequestedPowerLimit(double p)
{
	emit powerLimitRequested(p);
}

void Inverter::setMaxPower(double p)
{
	if (mMaxPower == p)
		return;
	mMaxPower = p;
	emit maxPowerChanged();
}

void Inverter::resetValues()
{
	mMeanPowerInfo->resetValues();
	mL1PowerInfo->resetValues();
	mL2PowerInfo->resetValues();
	mL3PowerInfo->resetValues();
}
