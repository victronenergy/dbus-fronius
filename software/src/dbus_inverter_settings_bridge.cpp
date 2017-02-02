#include "dbus_inverter_settings_bridge.h"
#include "defines.h"
#include "settings.h"
#include "inverter_settings.h"

static const QString Service = "sub/com.victronenergy.settings";
static const QString BasePath = "/Settings/Fronius/Inverters/";

DBusInverterSettingsBridge::DBusInverterSettingsBridge(
	InverterSettings *settings, QObject *parent) :
	DBusBridge(Service, false, parent)
{
	QString path = BasePath + Settings::createInverterId(settings->deviceType(),
														 settings->uniqueId());
	consume(settings, "phase",
			QVariant(static_cast<int>(settings->phase())), path + "/Phase", false);
	consume(settings, "position",
			QVariant(static_cast<int>(settings->position())), path + "/Position", false);
	consume(settings, "customName", QVariant(""), path + "/CustomName", false);
	consume(settings, "isActive", 1, path + "/IsActive", false);
	consume(settings, "l1Energy", 0.0, 0.0, 1e6, path + "/L1Energy", true);
	consume(settings, "l2Energy", 0.0, 0.0, 1e6, path + "/L2Energy", true);
	consume(settings, "l3Energy", 0.0, 0.0, 1e6, path + "/L3Energy", true);
}

bool DBusInverterSettingsBridge::toDBus(const QString &path, QVariant &v)
{
	if (path.endsWith("/Phase")) {
		v = QVariant(static_cast<int>(v.value<InverterPhase>()));
	} else if (path.endsWith("/Position")) {
		v = QVariant(static_cast<int>(v.value<InverterPosition>()));
	}
	return true;
}

bool DBusInverterSettingsBridge::fromDBus(const QString &path, QVariant &v)
{
	if (path.endsWith("/Phase")) {
		v = qVariantFromValue(static_cast<InverterPhase>(v.toInt()));
	} else if (path.endsWith("/Position")) {
		v = qVariantFromValue(static_cast<InverterPosition>(v.toInt()));
	}
	return true;
}
