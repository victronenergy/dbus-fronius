#include <QsLog.h>
#include "dbus_gateway_bridge.h"
#include "inverter_gateway.h"

static const QString ServiceName = "pub/com.victronenergy.fronius";

DBusGatewayBridge::DBusGatewayBridge(InverterGateway *gateway, QObject *parent):
	DBusBridge(ServiceName, true, parent)
{
	produce(gateway, "autoDetect", "/AutoDetect");
	produce(gateway, "scanProgress", "/ScanProgress", "%");
	registerService();
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
