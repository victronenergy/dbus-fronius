#include <cmath>
#include <QCoreApplication>
#include <QDBusConnection>
#include <QsLog.h>
#include <QStringList>
#include <velib/qt/v_busitems.h>
#include <velib/vecan/products.h>
#include "dbus_inverter_guard.h"
#include "inverter.h"
#include "power_info.h"
#include "version.h"

DBusInverterGuard::DBusInverterGuard(Inverter *inverter) :
	DBusGuard(inverter),
	mInverter(inverter)
{
	Q_ASSERT(inverter != 0);

	QString serviceName = QString("com.victronenergy.pvinverter.fronius_%1_%2").
			arg(fixServiceNameFragment(inverter->hostName())).
			arg(fixServiceNameFragment(inverter->id()));

	QDBusConnection connection = VBusItems::getConnection(serviceName);

	PowerInfo *mpi = inverter->meanPowerInfo();

	produce(connection, inverter, "isConnected",	"/Connected");
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
	// The values of the items below will not change after creation, so we don't
	// need an update mechanism.
	produce(connection, "/Mgmt/ProcessName", processName);
	produce(connection, "/Mgmt/ProcessVersion", VERSION);
	produce(connection, "/Mgmt/Connection", connectionString);
	produce(connection, "/Position", inverter->id());
	produce(connection, "/ProductId", VE_PROD_ID_PV_INVERTER_FRONIUS);
	produce(connection, "/ProductName", tr("Fronius PV inverter"));
	produce(connection, "/Serial", inverter->uniqueId());

	QLOG_INFO() << "Registering service" << serviceName;
	if (!connection.registerService(serviceName)) {
		QLOG_FATAL() << "RegisterService failed";
	}
}

void DBusInverterGuard::addBusItems(QDBusConnection &connection, PowerInfo *pi,
									const QString &path)
{
	produce(connection, pi, "current", path + "/Current", "A", 1);
	produce(connection, pi, "voltage", path + "/Voltage", "V", 0);
}

QString DBusInverterGuard::fixServiceNameFragment(const QString &s)
{
	return ((QString)s).remove('.').remove('_');
}
