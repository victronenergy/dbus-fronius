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
static const QString InverterSettingsPath = "/Settings/Fronius/Inverters";

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
	consume(connection, Service, settings, "inverterSettings", InverterSettingsPath);
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
		addDBusObject("Fronius", "ScanProgress", 's', QDBusVariant("")) &&
		addDBusObject("Fronius", "Inverters", 's', QDBusVariant("[]"));
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
	} else if (path == ScanProgressPath) {
		// This is a bit ugly: it's not possible (?) to set the unit of a
		// DBus value when it is consumed.
		int progress = value.toInt();
		value = QVariant(QString("%1%").arg(progress));
	} else if (path == InverterSettingsPath) {
		QList<InverterSettings *> settings = value.value<QList<InverterSettings *> >();
		QVariantList vl;
		foreach (InverterSettings *is, settings) {
			QVariantMap m;
			m["UniqueId"] = is->uniqueId();
			m["Phase"] = static_cast<int>(is->phase());
			m["Position"] = static_cast<int>(is->position());
			vl.append(m);
		}
		QString json = JSON::instance().serialize(qVariantFromValue(vl));
		value = QVariant(json);
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
	} else if (path == InverterSettingsPath) {
		// @todo EV It's tricky to use mSettings here. Move update functionality
		// to Settings?
		QList<InverterSettings *> settings = mSettings->inverterSettings();
		QString json = value.toString();
		foreach (QVariant v, JSON::instance().parse(json).toList()) {
			QVariantMap m = v.toMap();
			QString uniqueId = m["UniqueId"].toString();
			InverterSettings *is = mSettings->findInverterSettings(uniqueId);
			if (is == 0) {
				QLOG_WARN() << "Unknown inverter id received from dbus";
				is = new InverterSettings(uniqueId, mSettings);
				settings.append(is);
			}
			is->setPhase(static_cast<InverterSettings::Phase>(m["Phase"].toInt()));
			is->setPosition(static_cast<InverterSettings::Position>(m["Position"].toInt()));
		}
		value = qVariantFromValue(settings);
	}
}
