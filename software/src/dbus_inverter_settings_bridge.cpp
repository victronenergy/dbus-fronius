#include <QDBusConnection>
#include <QDBusVariant>
#include <velib/qt/v_busitems.h>
#include "dbus_inverter_settings_bridge.h"
#include "inverter_settings.h"

static const QString Service = "com.victronenergy.settings";
static const QString BasePath = "/Settings/Fronius/";

DBusInverterSettingsBridge::DBusInverterSettingsBridge(
	InverterSettings *settings, QObject *parent) :
	DBusBridge(parent)
{
	QDBusConnection &connection = VBusItems::getConnection();
	// Unique ID of a fronius inverter is currently a number. Using numbers as
	// name of a DBus object causes trouble: the local settings application
	// saves the settings as xml and uses object names as element names.
	// Unfortunately, the name of an xml element cannot start with a number.
	QString group = "Inverters/I" + settings->uniqueId();
	addDBusObject("Fronius", group + "/Position", 'i',
				  QDBusVariant(static_cast<int>(settings->position())));
	addDBusObject("Fronius", group + "/Phase", 'i',
				  QDBusVariant(static_cast<int>(settings->phase())));
	/// @todo EV We might need the exact device type here
	/// (eg. Fronius Galvo 3.0-1). This would force (?) us to store
	/// the type in the settings, because the inverter may be offline
	/// when we need it.
	addDBusObject("Fronius", group + "/CustomName", 's',
				  QDBusVariant(tr("Fronius PV inverter")));

	QString path = BasePath + group;
	consume(connection, Service, settings, "phase", path + "/Phase");
	consume(connection, Service, settings, "position", path + "/Position");
	consume(connection, Service, settings, "customName", path + "/CustomName");
}

bool DBusInverterSettingsBridge::toDBus(const QString &path, QVariant &v)
{
	if (path.endsWith("/Phase")) {
		v = QVariant(static_cast<int>(v.value<InverterSettings::Phase>()));
	} else if (path.endsWith("/Position")) {
		v = QVariant(static_cast<int>(v.value<InverterSettings::Position>()));
	}
	return true;
}

bool DBusInverterSettingsBridge::fromDBus(const QString &path, QVariant &v)
{
	if (path.endsWith("/Phase")) {
		v = qVariantFromValue(static_cast<InverterSettings::Phase>(v.toInt()));
	} else if (path.endsWith("/Position")) {
		v = qVariantFromValue(static_cast<InverterSettings::Position>(v.toInt()));
	}
	return true;
}
