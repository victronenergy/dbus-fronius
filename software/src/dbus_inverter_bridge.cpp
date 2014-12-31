#include <QCoreApplication>
#include <QDBusConnection>
#include <QHostAddress>
#include <QsLog.h>
#include <QStringList>
#include <velib/qt/v_busitems.h>
#include <velib/vecan/products.h>
#include "dbus_inverter_bridge.h"
#include "inverter.h"
#include "power_info.h"
#include "version.h"

DBusInverterBridge::DBusInverterBridge(Inverter *inverter, QObject *parent) :
	DBusBridge(parent)
{
	Q_ASSERT(inverter != 0);

	QStringList spl = inverter->hostName().split('.');
	QString addr;
	for (int i=0; i<spl.length(); ++i) {
		addr.append(QString("%1").arg(spl[i], 3, QChar('0')));
	}

	QString serviceName = QString("com.victronenergy.pvinverter.fronius_%1_%2").
			arg(addr).
			arg(fixServiceNameFragment(inverter->id()));

	QDBusConnection connection = VBusItems::getConnection(serviceName);

	PowerInfo *mpi = inverter->meanPowerInfo();

	produce(connection, inverter, "isConnected", "/Connected");
	produce(connection, mpi, "current", "/Ac/Current", "A", 1);
	produce(connection, mpi, "voltage", "/Ac/Voltage", "V", 0);
	produce(connection, mpi, "power", "/Ac/Power", "W", 0);
	if (inverter->supports3Phases()) {
		addBusItems(connection, inverter->l1PowerInfo(), "/Ac/L1");
		addBusItems(connection, inverter->l2PowerInfo(), "/Ac/L2");
		addBusItems(connection, inverter->l3PowerInfo(), "/Ac/L3");
	}

	QString connectionString = QString("%1 - %2").
			arg(inverter->hostName()).
			arg(inverter->id());
	QString processName = QCoreApplication::arguments()[0];
	QString customName = inverter->customName();
	QString productName = QString("%1 - %2").
						  arg(tr("Fronius PV inverter")).
						  arg(customName.isEmpty() ?
								  inverter->uniqueId() : customName);
	// The values of the items below will not change after creation, so we don't
	// need an update mechanism.
	produce(connection, "/Mgmt/ProcessName", processName);
	produce(connection, "/Mgmt/ProcessVersion", VERSION);
	produce(connection, "/Mgmt/Connection", connectionString);
	produce(connection, "/Position", inverter->id());
	produce(connection, "/ProductId", VE_PROD_ID_PV_INVERTER_FRONIUS);
	produce(connection, "/ProductName", productName);
	produce(connection, "/Serial", inverter->uniqueId());

	QLOG_INFO() << "Registering service" << serviceName;
	if (!connection.registerService(serviceName)) {
		QLOG_FATAL() << "RegisterService failed";
	}
}

void DBusInverterBridge::toDBus(const QString &path, QVariant &value)
{
	if (path == "/Connected")
		value = QVariant(value.toBool() ? 1 : 0);
}

void DBusInverterBridge::fromDBus(const QString &path, QVariant &value)
{
	if (path == "/Connected")
		value = QVariant(value.toInt() != 0);
}

void DBusInverterBridge::addBusItems(QDBusConnection &connection, PowerInfo *pi,
									const QString &path)
{
	produce(connection, pi, "current", path + "/Current", "A", 1);
	produce(connection, pi, "voltage", path + "/Voltage", "V", 0);
}

QString DBusInverterBridge::fixServiceNameFragment(const QString &s)
{
	return ((QString)s).remove('.').remove('_');
}
