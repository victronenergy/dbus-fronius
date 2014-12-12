#include <QDebug>
#include "dbus_test.h"
#include "dbus_inverter_guard.h"
#include "inverter.h"
#include "inverter_gateway.h"
#include "inverter_updater.h"

DBusTest::DBusTest(QObject *parent) :
	QObject(parent),
	mGateway(new InverterGateway(this))
{
	connect(
		mGateway, SIGNAL(inverterFound(InverterUpdater&)),
		this, SLOT(onInverterFound(InverterUpdater&)));
	// new DBusClient(this);
}

void DBusTest::onInverterFound(InverterUpdater &iu)
{
	qDebug()
		<< __FUNCTION__
		<< iu.inverter()->hostName()
		<< iu.inverter()->id();
	// At this point, an inverter has been found, but it is not yet clear what
	// its capacities are (eg. support for 3 phases). So we cannot create a
	// DBus tree yet.
	connect(&iu, SIGNAL(initialized()), this, SLOT(onInverterInitialized()));
}

void DBusTest::onInverterInitialized()
{
	qDebug() << __FUNCTION__;
	new DBusInverterGuard(static_cast<InverterUpdater *>(sender())->inverter());
}
