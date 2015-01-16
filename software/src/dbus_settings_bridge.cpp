#include <QDBusVariant>
#include <QsLog.h>
#include <QStringList>
#include <velib/qt/v_busitems.h>
#include "dbus_settings_bridge.h"
#include "settings.h"

static const QString Service = "com.victronenergy.settings";
static const QString PortNumberPath = "/Settings/Fronius/PortNumber";
static const QString IpAddressesPath = "/Settings/Fronius/IPAddresses";
static const QString KnownIpAddressesPath = "/Settings/Fronius/KnownIPAddresses";

DBusSettingsBridge::DBusSettingsBridge(Settings *settings, QObject *parent):
	DBusBridge(parent)
{
	Q_ASSERT(settings != 0);

	QDBusConnection &connection = VBusItems::getConnection();
	consume(connection, Service, settings, "portNumber", PortNumberPath);
	consume(connection, Service, settings, "ipAddresses", IpAddressesPath);
	consume(connection, Service, settings, "knownIpAddresses", KnownIpAddressesPath);
}

bool DBusSettingsBridge::addDBusObjects()
{
	return
		addDBusObject("Fronius", "PortNumber", 'i', QDBusVariant(80)) &&
		addDBusObject("Fronius", "IPAddresses", 's', QDBusVariant("")) &&
		addDBusObject("Fronius", "KnownIPAddresses", 's', QDBusVariant(""));
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
	}
	return true;
}
