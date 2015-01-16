#include <cmath>
#include <QCoreApplication>
#include <QDBusConnection>
#include <QHostAddress>
#include <QsLog.h>
#include <QStringList>
#include <velib/qt/v_busitems.h>
#include <velib/vecan/products.h>
#include "dbus_inverter_bridge.h"
#include "inverter.h"
#include "inverter_settings.h"
#include "power_info.h"
#include "version.h"

DBusInverterBridge::DBusInverterBridge(Inverter *inverter,
									   InverterSettings *settings,
									   QObject *parent) :
	DBusBridge(parent)
{
	Q_ASSERT(inverter != 0);
	connect(inverter, SIGNAL(destroyed()), this, SLOT(deleteLater()));

	QStringList spl = inverter->hostName().split('.');
	QString addr;
	for (int i=0; i<spl.length(); ++i) {
		addr.append(QString("%1").arg(spl[i], 3, QChar('0')));
	}

	mServiceName = QString("com.victronenergy.pvinverter.fronius_%1_%2").
			arg(addr).
			arg(fixServiceNameFragment(inverter->id()));

	QDBusConnection connection = VBusItems::getConnection(mServiceName);

	produce(connection, inverter, "isConnected", "/Connected");
	produce(connection, inverter, "status", "/DeviceStatus");

	addBusItems(connection, inverter->meanPowerInfo(), "/Ac");
	addBusItems(connection, inverter->l1PowerInfo(), "/Ac/L1");
	addBusItems(connection, inverter->l2PowerInfo(), "/Ac/L2");
	addBusItems(connection, inverter->l3PowerInfo(), "/Ac/L3");

	produce(connection, settings, "position", "/Position");
	produce(connection, settings, "deviceInstance", "/DeviceInstance");
	produce(connection, settings, "customName", "/CustomName");

	QString connectionString = QString("%1 - %2").
			arg(inverter->hostName()).
			arg(inverter->id());
	QString processName = QCoreApplication::arguments()[0];
	// The values of the items below will not change after creation, so we don't
	// need an update mechanism.
	produce(connection, "/Mgmt/ProcessName", processName);
	produce(connection, "/Mgmt/ProcessVersion", VERSION);
	produce(connection, "/Mgmt/Connection", connectionString);
	produce(connection, "/ProductName", tr("Fronius PV inverter"));
	produce(connection, "/ProductId", VE_PROD_ID_PV_INVERTER_FRONIUS);
	produce(connection, "/Serial", inverter->uniqueId());

	QLOG_INFO() << "Registering service" << mServiceName;
	if (!connection.registerService(mServiceName)) {
		QLOG_FATAL() << "RegisterService failed";
	}
}

DBusInverterBridge::~DBusInverterBridge()
{
	QDBusConnection connection = VBusItems::getConnection(mServiceName);
	QLOG_INFO() << "Unregistering service" << mServiceName;
	if (!connection.unregisterService(mServiceName)) {
		QLOG_FATAL() << "UnregisterService failed";
	}
}

bool DBusInverterBridge::toDBus(const QString &path, QVariant &value)
{
	if (path == "/Connected") {
		value = QVariant(value.toBool() ? 1 : 0);
	} else if (path == "/Position") {
		value = QVariant(static_cast<int>(value.value<InverterPosition>()));
	}
	if (value.type() == QVariant::Double && !std::isfinite(value.toDouble()))
		value = qVariantFromValue(QStringList());
	return true;
}

bool DBusInverterBridge::fromDBus(const QString &path, QVariant &value)
{
	if (path == "/Connected") {
		value = QVariant(value.toInt() != 0);
	} else if (path == "/Position") {
		value = QVariant(static_cast<InverterPosition>(value.toInt()));
	}
	return true;
}

void DBusInverterBridge::addBusItems(QDBusConnection &connection, PowerInfo *pi,
									const QString &path)
{
	produce(connection, pi, "current", path + "/Current", "A", 1);
	produce(connection, pi, "voltage", path + "/Voltage", "V", 0);
	produce(connection, pi, "power", path + "/Power", "W", 0);
	produce(connection, pi, "totalEnergy", path + "/Energy/Forward", "kWh", 0);
}

QString DBusInverterBridge::fixServiceNameFragment(const QString &s)
{
	return ((QString)s).remove('.').remove('_');
}
