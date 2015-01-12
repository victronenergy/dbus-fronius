#include <QsLog.h>
#include "dbus_fronius.h"
#include "dbus_inverter_bridge.h"
#include "dbus_inverter_settings_bridge.h"
#include "dbus_settings_bridge.h"
#include "inverter.h"
#include "inverter_gateway.h"
#include "inverter_settings.h"
#include "inverter_updater.h"
#include "settings.h"

DBusFronius::DBusFronius(QObject *parent) :
	QObject(parent),
	mSettings(new Settings(this)),
	mGateway(new InverterGateway(mSettings, this)),
	mSettingsBridge(new DBusSettingsBridge(mSettings, mGateway, this))
{
	// This enables us to retrieve values from QT properties via the
	// QObject::property function.
	qRegisterMetaType<QList<QHostAddress> >();
	qRegisterMetaType<QHostAddress>();
	qRegisterMetaType<InverterSettings::Position>("Position");
	qRegisterMetaType<InverterSettings::Phase>("Phase");

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
	// its capabilities are (eg. support for 3 phases). So we cannot create a
	// DBus tree yet.
	connect(iu, SIGNAL(initialized()), this, SLOT(onInverterInitialized()));
}

void DBusFronius::onInverterInitialized()
{
	InverterUpdater *ui = static_cast<InverterUpdater *>(sender());
	Inverter *inverter = ui->inverter();
	InverterSettings *settings = ui->settings();
	// DBusInverterBridge will set inverter as its parent, so we have no
	// memory leak here.
	new DBusInverterBridge(inverter, settings, inverter);
	new DBusInverterSettingsBridge(settings, settings);
}

void DBusFronius::onSettingsInitialized()
{
	mGateway->startDetection();
}
