#include <qnumeric.h>
#include <QCoreApplication>
#include <QStringList>
#include "fronius_device_info.h"
#include "inverter.h"
#include "power_info.h"
#include "logging.h"

Inverter::Inverter(VeQItem *root, const DeviceInfo &deviceInfo, int deviceInstance,
				   QObject *parent) :
	VeService(root, parent),
	mDeviceInfo(deviceInfo),
	mErrorCode(createItem("ErrorCode")),
	mStatusCode(createItem("StatusCode")),
	mPowerLimit(createItem("Ac/PowerLimit")),
	mPosition(createItem("Position")),
	mDeviceInstance(createItem("DeviceInstance")),
	mCustomName(createItem("CustomName")),
	mProductName(createItem("ProductName")),
	mConnection(createItem("Mgmt/Connection")),
	mMeanPowerInfo(new BasicPowerInfo(root->itemGetOrCreate("Ac", false), this)),
	mL1PowerInfo(new PowerInfo(root->itemGetOrCreate("Ac/L1", false), this)),
	mL2PowerInfo(new PowerInfo(root->itemGetOrCreate("Ac/L2", false), this)),
	mL3PowerInfo(new PowerInfo(root->itemGetOrCreate("Ac/L3", false), this))
{
	produceValue(createItem("Connected"), 1);
	produceValue(createItem("Mgmt/ProcessName"), QCoreApplication::arguments()[0]);
	produceValue(createItem("Mgmt/ProcessVersion"), QCoreApplication::applicationVersion());
	produceValue(createItem("ProductName"), deviceInfo.productName);
	produceValue(createItem("ProductId"), deviceInfo.productId,
		QString::number(deviceInfo.productId, 16));
	produceValue(createItem("Serial"), deviceInfo.serialNumber.isEmpty() ? deviceInfo.uniqueId : deviceInfo.serialNumber);
	produceValue(mDeviceInstance, deviceInstance);
	produceDouble(createItem("Ac/MaxPower"), deviceInfo.maxPower, 0, "W");
	produceValue(createItem("FirmwareVersion"), deviceInfo.firmwareVersion.isEmpty() ?
		QVariant() : deviceInfo.firmwareVersion);
	produceValue(createItem("DataManagerVersion"), deviceInfo.dataManagerVersion.isEmpty() ?
		QVariant() : deviceInfo.dataManagerVersion);
	produceValue(createItem("Ac/NumberOfPhases"), deviceInfo.phaseCount);
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
		case 11:
			text = "Running (MPPT)";
			break;
		case 12:
			text = "Running (Throttled)";
			break;
		default:
			text = QString::number(code);
			break;
		}
	}
	produceValue(mStatusCode, code, text);
}

void Inverter::invalidateStatusCode()
{
	produceValue(mStatusCode, QVariant(), "");
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
	QVariant n = mCustomName->getValue();
	if (n.isValid() && n.toString() == name)
		return;
	produceValue(mCustomName, name);
	emit customNameChanged();
}

QString Inverter::hostName() const
{
	return mDeviceInfo.hostName;
}

int Inverter::networkId() const
{
	return mDeviceInfo.networkId;
}

void Inverter::setHostName(const QString &h)
{
	if (mDeviceInfo.hostName== h)
		return;
	mDeviceInfo.hostName= h;
	updateConnectionItem();
	emit hostNameChanged();
}

int Inverter::port() const
{
	return mDeviceInfo.port;
}

void Inverter::setPort(int p)
{
	if (mDeviceInfo.port == p)
		return;
	mDeviceInfo.port = p;
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

BasicPowerInfo *Inverter::meanPowerInfo()
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
	Q_ASSERT(phase != MultiPhase);
	switch (phase) {
	case PhaseL1:
		return mL1PowerInfo;
	case PhaseL2:
		return mL2PowerInfo;
	case PhaseL3:
		return mL3PowerInfo;
	default:
		qCritical() << "Incorrect phase:" << phase;
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

QString Inverter::location() const
{
	return QString("%1@%2:%3").
		arg(mDeviceInfo.uniqueId).
		arg(mDeviceInfo.hostName).
		arg(mDeviceInfo.networkId);
}

void Inverter::updateConnectionItem()
{
	produceValue(mConnection, QString("%1 - %2 (%3)").
		arg(mDeviceInfo.hostName).arg(mDeviceInfo.networkId).
		arg(mDeviceInfo.retrievalMode == ProtocolFroniusSolarApi ? "solarapi" : "sunspec"));
}

ThrottledInverter::ThrottledInverter(VeQItem *root, const DeviceInfo &deviceInfo,
					int deviceInstance, QObject *parent) :
	Inverter(root, deviceInfo, deviceInstance, parent)
{
	// If it has sunspec model 123, powerLimitScale will be non-zero.
	// Enable the power limiter and initialise it to maxPower.
	if (deviceInfo.powerLimitScale)
		setPowerLimit(deviceInfo.maxPower);
}
