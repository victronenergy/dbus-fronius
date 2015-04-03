#include "dbus_inverter_settings_bridge.h"
#include "defines.h"
#include "settings.h"
#include "inverter_settings.h"

static const QString Service = "com.victronenergy.settings";
static const QString BasePath = "/Settings/Fronius/Inverters/";

DBusInverterSettingsBridge::DBusInverterSettingsBridge(
	InverterSettings *settings, QObject *parent) :
	DBusBridge(parent)
{
	QString path = BasePath + Settings::createInverterId(settings->deviceType(),
														 settings->uniqueId());
	consume(Service, settings, "phase",
			QVariant(static_cast<int>(settings->phase())), path + "/Phase");
	consume(Service, settings, "position",
			QVariant(static_cast<int>(settings->position())), path + "/Position");
	consume(Service, settings, "deviceInstance",
			QVariant(InvalidDeviceInstance), path + "/DeviceInstance");
	consume(Service, settings, "customName",
			QVariant(""), path + "/CustomName");
	consume(Service, settings, "l1Energy",
			0.0, 0.0, 1e6, path + "/L1Energy");
	consume(Service, settings, "l2Energy",
			0.0, 0.0, 1e6, path + "/L2Energy");
	consume(Service, settings, "l3Energy",
			0.0, 0.0, 1e6, path + "/L3Energy");
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
