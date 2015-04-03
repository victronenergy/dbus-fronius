#include <QsLog.h>
#include <QStringList>
#include "dbus_settings_bridge.h"
#include "settings.h"

static const QString Service = "com.victronenergy.settings";
static const QString PortNumberPath = "/Settings/Fronius/PortNumber";
static const QString IpAddressesPath = "/Settings/Fronius/IPAddresses";
static const QString KnownIpAddressesPath = "/Settings/Fronius/KnownIPAddresses";
static const QString InverterIdsPath = "/Settings/Fronius/InverterIds";

DBusSettingsBridge::DBusSettingsBridge(Settings *settings, QObject *parent):
	DBusBridge(parent)
{
	Q_ASSERT(settings != 0);

	consume(Service, settings, "portNumber", QVariant(80), PortNumberPath);
	consume(Service, settings, "ipAddresses", QVariant(""), IpAddressesPath);
	consume(Service, settings, "knownIpAddresses", QVariant(""), KnownIpAddressesPath);
	consume(Service, settings, "inverterIds", QVariant(""), InverterIdsPath);
}

bool DBusSettingsBridge::toDBus(const QString &path, QVariant &value)
{
	if (path == IpAddressesPath || path == KnownIpAddressesPath) {
		// Convert QList<QHostAddress> to QStringList, because D-Bus
		// support for custom types is sketchy. Furthermore, the localsettings
		// service only supports standard types (string, int, double).
		QString addresses;
		foreach (QHostAddress a, value.value<QList<QHostAddress> >()) {
			if (!addresses.isEmpty())
				addresses.append(',');
			addresses.append(a.toString());
		}
		value = addresses;
	} else if (path == InverterIdsPath) {
		value = value.value<QStringList>().join(",");
	}
	return true;
}

bool DBusSettingsBridge::fromDBus(const QString &path, QVariant &value)
{
	if (path == IpAddressesPath || path == KnownIpAddressesPath) {
		// Convert from DBus type back to to QList<QHostAddress>.
		QList<QHostAddress> addresses;
		foreach (QString a, value.toString().split(',', QString::SkipEmptyParts)) {
			addresses.append(QHostAddress(a));
		}
		value = QVariant::fromValue(addresses);
	} else if (path == InverterIdsPath) {
		value = value.toString().split(',', QString::SkipEmptyParts);
	}
	return true;
}
