#include <QMetaType>
#include <QsLog.h>
#include <velib/qt/v_busitem.h>
#include "dbus_settings_guard.h"
#include "settings.h"
#include "inverter_gateway.h"

Q_DECLARE_METATYPE(QList<QHostAddress>)
Q_DECLARE_METATYPE(QHostAddress)

static const QString Service = "com.victronenergy.settings";
static const QString AutoDetectPath = "/Settings/Fronius/AutoDetect";
static const QString IpAddressesPath = "/Settings/Fronius/IPAddresses";
static const QString KnownIpAddressesPath = "/Settings/Fronius/KnownIPAddresses";
static const QString ScanProgressPath = "/Settings/Fronius/ScanProgress";

DBusSettingsGuard::DBusSettingsGuard(Settings *settings,
	InverterGateway *gateway, QObject *parent) :
	DBusGuard(parent)
{
	QDBusConnection connection = QDBusConnection::sessionBus();
	consume(connection, Service, settings, "autoDetect", AutoDetectPath);
	consume(connection, Service, settings, "ipAddresses", IpAddressesPath);
	consume(connection, Service, settings, "knownIpAddresses", KnownIpAddressesPath);
	if (gateway != 0) {
		consume(connection, Service, gateway, "scanProgress", ScanProgressPath);
	}
}

void DBusSettingsGuard::toDBus(const QString &path, QVariant &value)
{
	QLOG_TRACE() << __FUNCTION__ << path << value;
	if (path == IpAddressesPath || path == KnownIpAddressesPath) {
		// Convert QList<QHostAddress> to QStringList, because DBus
		// support for custom types is sketchy. (How do you supply type
		// information?)
		QStringList addresses;
		foreach (QHostAddress a, value.value<QList<QHostAddress> >()) {
			addresses.append(a.toString());
		}
		value = addresses;
	}
	QLOG_TRACE() << __FUNCTION__ << path << value;
}

void DBusSettingsGuard::fromDBus(const QString &path, QVariant &value)
{
	QLOG_TRACE() << __FUNCTION__ << path << value;
	if (path == IpAddressesPath || path == KnownIpAddressesPath) {
		// Convert from DBus type back to to QList<QHostAddress>.
		// See onPropertyChanged.
		QList<QHostAddress> addresses;
		foreach (QString a, value.toStringList()) {
			addresses.append(QHostAddress(a));
		}
		value = QVariant::fromValue(addresses);
	}
	QLOG_TRACE() << __FUNCTION__ << path << value;
}
