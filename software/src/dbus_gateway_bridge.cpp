#include <QDBusConnection>
#include <QsLog.h>
#include <velib/qt/v_busitems.h>
#include "dbus_gateway_bridge.h"
#include "inverter_gateway.h"

static const QString ServiceName = "com.victronenergy.fronius";

DBusGatewayBridge::DBusGatewayBridge(InverterGateway *gateway, QObject *parent):
	DBusBridge(parent)
{
	QDBusConnection connection = VBusItems::getConnection(ServiceName);
	produce(connection, gateway, "autoDetect", "/AutoDetect");
	produce(connection, gateway, "scanProgress", "/ScanProgress", "%");

	QLOG_INFO() << "Registering service" << ServiceName;
	if (!connection.registerService(ServiceName)) {
		QLOG_FATAL() << "RegisterService failed";
	}
}

DBusGatewayBridge::~DBusGatewayBridge()
{
	QDBusConnection connection = VBusItems::getConnection(ServiceName);
	QLOG_INFO() << "Unregistering service" << ServiceName;
	if (!connection.unregisterService(ServiceName)) {
		QLOG_FATAL() << "UnregisterService failed";
	}
}

bool DBusGatewayBridge::toDBus(const QString &path, QVariant &value)
{
	// D-Bus wants an integer value for autoDetect, but InverterGateway has a
	// boolean, so we convert it here.
	if (path == "/AutoDetect")
		value = value.toBool() ? 1 : 0;
	return true;
}

bool DBusGatewayBridge::fromDBus(const QString &path, QVariant &value)
{
	if (path == "/AutoDetect")
		value = value.toInt() != 0;
	return true;
}
