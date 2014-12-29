#include <QsLog.h>
#include <velib/qt/v_busitem.h>
#include <velib/qt/v_busitems.h>
#include "dbus_settings_bridge.h"
#include "settings.h"
#include "inverter_gateway.h"

static const QString Service = "com.victronenergy.settings";
static const QString AutoDetectPath = "/Settings/Fronius/AutoDetect";
static const QString IpAddressesPath = "/Settings/Fronius/IPAddresses";
static const QString KnownIpAddressesPath = "/Settings/Fronius/KnownIPAddresses";
static const QString ScanProgressPath = "/Settings/Fronius/ScanProgress";

DBusSettingsBridge::DBusSettingsBridge(Settings *settings,
	InverterGateway *gateway, QObject *parent) :
	DBusBridge(parent)
{
	// This enables us to retrieve values from QT properties via the
	// QObject::property function.
	qRegisterMetaType<QList<QHostAddress> >();
	qRegisterMetaType<QHostAddress>();

	QDBusConnection &connection = VBusItems::getConnection();
	consume(connection, Service, settings, "autoDetect", AutoDetectPath);
	consume(connection, Service, settings, "ipAddresses", IpAddressesPath);
	consume(connection, Service, settings, "knownIpAddresses", KnownIpAddressesPath);
	if (gateway != 0) {
		consume(connection, Service, gateway, "scanProgress", ScanProgressPath);
	}
}

bool DBusSettingsBridge::addDBusObjects()
{
	return
		addDBusObjects("Fronius", "AutoDetect", 'i', QDBusVariant(0)) &&
		addDBusObjects("Fronius", "IPAddresses", 's', QDBusVariant("")) &&
		addDBusObjects("Fronius", "KnownIPAddresses", 's', QDBusVariant("")) &&
		addDBusObjects("Fronius", "ScanProgress", 'i', QDBusVariant(0));
}

void DBusSettingsBridge::toDBus(const QString &path, QVariant &value)
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
}

void DBusSettingsBridge::fromDBus(const QString &path, QVariant &value)
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
}

bool DBusSettingsBridge::addDBusObjects(const QString &group, const QString &name,
									 QChar type,
									 const QDBusVariant &defaultValue)
{
	QDBusConnection &connection = VBusItems::getConnection();
	QDBusMessage m = QDBusMessage::createMethodCall(
				"com.victronenergy.settings",
				"/Settings",
				"com.victronenergy.Settings",
				"AddSetting")
		<< group
		<< name
		<< QVariant::fromValue(defaultValue)
		<< QString(type)
		<< QVariant::fromValue(QDBusVariant(0))
		<< QVariant::fromValue(QDBusVariant(0));
	QDBusMessage reply = connection.call(m);
	return reply.type() == QDBusMessage::ReplyMessage;
}