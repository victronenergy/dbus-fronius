#include <qnumeric.h>
#include <QCoreApplication>
#include <QHostAddress>
#include <QsLog.h>
#include <QStringList>
#include <velib/vecan/products.h>
#include "dbus_inverter_bridge.h"
#include "inverter.h"
#include "inverter_settings.h"
#include "power_info.h"

DBusInverterBridge::DBusInverterBridge(Inverter *inverter, InverterSettings *settings,
									   QObject *parent) :
	DBusBridge(
		QString("pub/com.victronenergy.pvinverter.fronius_%1_%2").
			arg(inverter->deviceType()).
			arg(fixServiceNameFragment(inverter->uniqueId())),
		true,
		parent),
	mInverter(inverter)
{
	produce(inverter, "isConnected", "/Connected");
	produce(inverter, "errorCode", "/ErrorCode");
	produce(inverter, "statusCode", "/StatusCode");
	produce(inverter, "powerLimit", "/Ac/PowerLimit", "W", 0, true);
	produce(inverter, "minPowerLimit", "/Ac/MinPowerLimit", "W", 0);
	produce(inverter, "maxPower", "/Ac/MaxPower", "W", 0);

	addBusItems(inverter->meanPowerInfo(), "/Ac");
	addBusItems(inverter->l1PowerInfo(), "/Ac/L1");
	addBusItems(inverter->l2PowerInfo(), "/Ac/L2");
	addBusItems(inverter->l3PowerInfo(), "/Ac/L3");

	produce(settings, "position", "/Position");
	produce(inverter, "deviceInstance", "/DeviceInstance");
	produce(settings, "customName", "/CustomName");
	produce(inverter, "hostName", "/Mgmt/Connection");

	QString processName = QCoreApplication::arguments()[0];
	// The values of the items below will not change after creation, so we don't
	// need an update mechanism.
	produce("/Mgmt/ProcessName", processName);
	produce("/Mgmt/ProcessVersion", VERSION);
	produce("/ProductName", inverter->productName());
	produce("/ProductId", VE_PROD_ID_PV_INVERTER_FRONIUS);
	produce("/FroniusDeviceType", inverter->deviceType());
	produce("/Serial", inverter->uniqueId());

	registerService();
}

bool DBusInverterBridge::toDBus(const QString &path, QVariant &value)
{
	if (path == "/Connected") {
		value = QVariant(value.toBool() ? 1 : 0);
	} else if (path == "/Position") {
		value = QVariant(static_cast<int>(value.value<InverterPosition>()));
	} else if (path == "/Mgmt/Connection") {
		value = QString("%1 - %2").arg(value.toString()).arg(mInverter->id());
	} else if (path == "/CustomName") {
		QString name = value.toString();
		if (name.isEmpty())
			value = mInverter->productName();
	}
	if (value.type() == QVariant::Double && !qIsFinite(value.toDouble()))
		value = QVariant();
	return true;
}

bool DBusInverterBridge::fromDBus(const QString &path, QVariant &value)
{
	if (path == "/CustomName") {
		QString name = value.toString();
		if (name == mInverter->productName())
			value = "";
		return true;
	} else if (path == "/Ac/PowerLimit") {
		mInverter->setRequestedPowerLimit(value.isValid() ? value.toDouble() : qQNaN());
	}
	return false;
}

void DBusInverterBridge::addBusItems(PowerInfo *pi, const QString &path)
{
	produce(pi, "current", path + "/Current", "A", 1);
	produce(pi, "voltage", path + "/Voltage", "V", 0);
	produce(pi, "power", path + "/Power", "W", 0);
	produce(pi, "totalEnergy", path + "/Energy/Forward", "kWh", 2);
}

QString DBusInverterBridge::fixServiceNameFragment(const QString &s)
{
	QString tmp = s;
	return tmp.remove('.').remove('_');
}
