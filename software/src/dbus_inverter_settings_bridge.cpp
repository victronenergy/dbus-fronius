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
	QString group = "Inverters/" + settings->uniqueId();
	addDBusObject("Fronius", group + "/Position", 'i', QDBusVariant(0));
	addDBusObject("Fronius", group + "/Phase", 'i', QDBusVariant(0));

	QString path = BasePath + group;
	consume(connection, Service, settings, "phase", path + "/Phase");
	consume(connection, Service, settings, "position", path + "/Position");
}

void DBusInverterSettingsBridge::toDBus(const QString &path, QVariant &v)
{
	if (path.endsWith("/Phase")) {
		v = QVariant(static_cast<int>(v.value<InverterSettings::Phase>()));
	} else if (path.endsWith("/Position")) {
		v = QVariant(static_cast<int>(v.value<InverterSettings::Position>()));
	}
}

void DBusInverterSettingsBridge::fromDBus(const QString &path, QVariant &v)
{
	if (path.endsWith("/Phase")) {
		v = qVariantFromValue(static_cast<InverterSettings::Phase>(v.toInt()));
	} else if (path.endsWith("/Position")) {
		v = qVariantFromValue(static_cast<InverterSettings::Position>(v.toInt()));
	}
}
