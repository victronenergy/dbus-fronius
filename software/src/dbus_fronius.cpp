#include <QsLog.h>
#include "dbus_fronius.h"
#include "dbus_inverter_bridge.h"
#include "dbus_settings_bridge.h"
#include "inverter.h"
#include "inverter_gateway.h"
#include "inverter_updater.h"
#include "settings.h"

DBusFronius::DBusFronius(QObject *parent) :
	QObject(parent),
	mSettings(new Settings(this)),
	mGateway(new InverterGateway(mSettings, this)),
	mSettingsBridge(new DBusSettingsBridge(mSettings, mGateway, this))
{
	connect(mGateway, SIGNAL(inverterFound(InverterUpdater *)),
			this, SLOT(onInverterFound(InverterUpdater *)));
	connect(mSettingsBridge, SIGNAL(initialized()),
			this, SLOT(onSettingsInitialized()));
}

void DBusFronius::onInverterFound(InverterUpdater *iu)
{
	QLOG_INFO() << "New inverter:" << iu->inverter()->hostName()
				<< iu->inverter()->id();
	// At this point, an inverter has been found, but it is not yet clear what
	// its capacities are (eg. support for 3 phases). So we cannot create a
	// DBus tree yet.
	connect(iu, SIGNAL(initialized()), this, SLOT(onInverterInitialized()));
}

void DBusFronius::onInverterInitialized()
{
	Inverter *inverter = static_cast<InverterUpdater *>(sender())->inverter();
	// DBusInverterBridge will set inverter as its parent, so we have no
	// memory leak here.
	new DBusInverterBridge(inverter, this);
}

void DBusFronius::onSettingsInitialized()
{
	mGateway->startDetection();
}
