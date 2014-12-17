#include <QsLog.h>
#include "dbus_test.h"
#include "dbus_inverter_guard.h"
#include "dbus_settings_guard.h"
#include "inverter.h"
#include "inverter_gateway.h"
#include "inverter_updater.h"
#include "settings.h"

DBusTest::DBusTest(QObject *parent) :
	QObject(parent),
	mSettings(new Settings(this)),
	mGateway(new InverterGateway(mSettings, this)),
	mSettingsGuard(new DBusSettingsGuard(mSettings, mGateway, this))
{
	connect(
		mGateway, SIGNAL(inverterFound(InverterUpdater&)),
		this, SLOT(onInverterFound(InverterUpdater&)));
}

void DBusTest::onInverterFound(InverterUpdater &iu)
{
	QLOG_INFO() << "New inverter:" << iu.inverter()->hostName()
				<< iu.inverter()->id();
	// At this point, an inverter has been found, but it is not yet clear what
	// its capacities are (eg. support for 3 phases). So we cannot create a
	// DBus tree yet.
	connect(&iu, SIGNAL(initialized()), this, SLOT(onInverterInitialized()));
}

void DBusTest::onInverterInitialized()
{
	Inverter *inverter = static_cast<InverterUpdater *>(sender())->inverter();
	// DBusInverterGuard will set inverter as its parent, so we have no
	// memory leak here.
	new DBusInverterGuard(inverter);
}
