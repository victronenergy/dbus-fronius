#include <QsLog.h>
#include <velib/qt/v_busitem.h>
#include <velib/qt/v_busitems.h>
#include "dbus_settings_bridge.h"
#include "json/json.h"
#include "inverter_settings.h"
#include "inverter_gateway.h"
#include "settings.h"

static const QString Service = "com.victronenergy.settings";
static const QString AutoDetectPath = "/Settings/Fronius/AutoDetect";
static const QString PortNumberPath = "/Settings/Fronius/PortNumber";
static const QString IpAddressesPath = "/Settings/Fronius/IPAddresses";
static const QString KnownIpAddressesPath = "/Settings/Fronius/KnownIPAddresses";
static const QString ScanProgressPath = "/Settings/Fronius/ScanProgress";

DBusSettingsBridge::DBusSettingsBridge(Settings *settings,
	InverterGateway *gateway, QObject *parent) :
	DBusBridge(parent),
	mSettings(settings)
{
	Q_ASSERT(settings != 0);

	QDBusConnection &connection = VBusItems::getConnection();
	consume(connection, Service, settings, "autoDetect", AutoDetectPath);
	consume(connection, Service, settings, "portNumber", PortNumberPath);
	consume(connection, Service, settings, "ipAddresses", IpAddressesPath);
	consume(connection, Service, settings, "knownIpAddresses", KnownIpAddressesPath);
	if (gateway != 0) {
		consume(connection, Service, gateway, "scanProgress", ScanProgressPath);
	}
}

bool DBusSettingsBridge::addDBusObjects()
{
	return
		addDBusObject("Fronius", "AutoDetect", 'i', QDBusVariant(0)) &&
		addDBusObject("Fronius", "PortNumber", 'i', QDBusVariant(80)) &&
		addDBusObject("Fronius", "IPAddresses", 's', QDBusVariant("")) &&
		addDBusObject("Fronius", "KnownIPAddresses", 's', QDBusVariant("")) &&
		addDBusObject("Fronius", "ScanProgress", 's', QDBusVariant(""));
}

bool DBusSettingsBridge::toDBus(const QString &path, QVariant &value)
{
	if (path == IpAddressesPath || path == KnownIpAddressesPath) {
		// Convert QList<QHostAddress> to QStringList, because DBus
		// support for custom types is sketchy. (How do you supply type
		// information?)
		QString addresses;
		foreach (QHostAddress a, value.value<QList<QHostAddress> >()) {
			if (!addresses.isEmpty())
				addresses.append(',');
			addresses.append(a.toString());
		}
		value = addresses;
	} else if (path == ScanProgressPath) {
		// This is a bit ugly: it's not possible (?) to set the unit of a
		// DBus value when it is consumed.
		int progress = value.toInt();
		value = QVariant(QString("%1%").arg(progress));
	}
	return true;
}

bool DBusSettingsBridge::fromDBus(const QString &path, QVariant &value)
{
	if (path == IpAddressesPath || path == KnownIpAddressesPath) {
		// Convert from DBus type back to to QList<QHostAddress>.
		// See onPropertyChanged.
		QList<QHostAddress> addresses;
		foreach (QString a, value.toString().split(',', QString::SkipEmptyParts)) {
			addresses.append(QHostAddress(a));
		}
		value = QVariant::fromValue(addresses);
	} else if (path == ScanProgressPath) {
		return false;
	}
	return true;
}
