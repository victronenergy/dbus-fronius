#include <qnumeric.h>
#include <QCoreApplication>
#include <QsLog.h>
#include <QStringList>
#include <velib/vecan/products.h>
#include "fronius_device_info.h"
#include "inverter.h"
#include "power_info.h"

Inverter::Inverter(VeQItem *root, const DeviceInfo &deviceInfo, int deviceInstance,
				   QObject *parent) :
	VeService(root, parent),
	mHostName(deviceInfo.hostName),
	mUniqueId(deviceInfo.uniqueId),
	mId(deviceInfo.networkId),
	mPort(deviceInfo.port),
	mPhaseCount(deviceInfo.phaseCount),
	mErrorCode(createItem("ErrorCode")),
	mStatusCode(createItem("StatusCode")),
	mPowerLimit(createItem("Ac/PowerLimit")),
	mMaxPower(createItem("Ac/MaxPower")),
	mPosition(createItem("Position")),
	mDeviceInstance(createItem("DeviceInstance")),
	mCustomName(createItem("CustomName")),
	mProductName(createItem("ProductName")),
	mConnection(createItem("Mgmt/Connection")),
	mMeanPowerInfo(new PowerInfo(root->itemGetOrCreate("Ac", false), this)),
	mL1PowerInfo(new PowerInfo(root->itemGetOrCreate("Ac/L1", false), this)),
	mL2PowerInfo(new PowerInfo(root->itemGetOrCreate("Ac/L2", false), this)),
	mL3PowerInfo(new PowerInfo(root->itemGetOrCreate("Ac/L3", false), this))
{
	produceValue(createItem("Connected"), 1);
	produceValue(createItem("Mgmt/ProcessName"), QCoreApplication::arguments()[0]);
	produceValue(createItem("Mgmt/ProcessVersion"), QCoreApplication::applicationVersion());
	produceValue(createItem("ProductName"), deviceInfo.productName);
	produceValue(createItem("ProductId"), VE_PROD_ID_PV_INVERTER_FRONIUS,
				 QString::number(VE_PROD_ID_PV_INVERTER_FRONIUS, 16));
	// produceValue(createItem("FroniusDeviceType"), deviceInfo.deviceType);
	produceValue(createItem("Serial"), deviceInfo.uniqueId);
	produceValue(mDeviceInstance, deviceInstance);
	updateConnectionItem();
	root->produceValue(QVariant(), VeQItem::Synchronized);
}

int Inverter::errorCode() const
{
	return mErrorCode->getValue().toInt();
}

void Inverter::setErrorCode(int code)
{
	produceValue(mErrorCode, code);
}

int Inverter::statusCode() const
{
	return mStatusCode->getValue().toInt();
}

void Inverter::setStatusCode(int code)
{
	QString text;
	if (code >= 0 && code < 7) {
		text = QString("Startup %1/6").arg(code);
	} else {
		switch (code) {
		case 7:
			text = "Running";
			break;
		case 8:
			text = "Standby";
			break;
		case 9:
			text = "Boot loading";
			break;
		case 10:
			text = "Error";
			break;
		default:
			text = QString::number(code);
			break;
		}
	}
	produceValue(mStatusCode, code, text);
}

int Inverter::id() const
{
	return mId;
}

QString Inverter::uniqueId() const
{
	return mUniqueId;
}

QString Inverter::productName() const
{
	return mProductName->getValue().toString();
}

QString Inverter::customName() const
{
	return mCustomName->getValue().toString();
}

void Inverter::setCustomName(const QString &name)
{
	if (customName() == name)
		return;
	produceValue(mCustomName, name);
	emit customNameChanged();
}

QString Inverter::hostName() const
{
	return mHostName;
}

void Inverter::setHostName(const QString &h)
{
	if (mHostName== h)
		return;
	mHostName = h;
	updateConnectionItem();
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

InverterPosition Inverter::position() const
{
	return static_cast<InverterPosition>(mPosition->getValue().toInt());
}

void Inverter::setPosition(InverterPosition p)
{
	QString text;
	switch (p) {
	case Input1:
		text = "Input 1";
		break;
	case Output:
		text = "Output";
		break;
	case Input2:
		text = "Input 2";
		break;
	default:
		QString::number(p);
		break;
	}
	produceValue(mPosition, static_cast<int>(p), text);
}

int Inverter::phaseCount() const
{
	return mPhaseCount;
}

int Inverter::deviceInstance() const
{
	return mDeviceInstance->getValue().toInt();
}

void Inverter::setDeviceInstance(int instance)
{
	// Q_ASSERT(root()->getState() == VeQItem::Offline || root()->getState() == VeQItem::Idle);
	produceValue(mDeviceInstance, instance);
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

double Inverter::powerLimit() const
{
	return getDouble(mPowerLimit);
}

void Inverter::setPowerLimit(double p)
{
	produceDouble(mPowerLimit, p, 0, "W");
}

double Inverter::maxPower() const
{
	return getDouble(mMaxPower);
}

void Inverter::setMaxPower(double p)
{
	produceDouble(mMaxPower, p, 0, "W");
}

int Inverter::handleSetValue(VeQItem *item, const QVariant &variant)
{
	if (item == mPowerLimit) {
		emit powerLimitRequested(variant.toDouble());
		return 0;
	}
	if (item == mCustomName) {
		setCustomName(variant.toString());
		return 0;
	}
	return VeService::handleSetValue(item, variant);
}

void Inverter::updateConnectionItem()
{
	produceValue(mConnection, QString("%1 - %2").arg(mHostName).arg(mId));
}
