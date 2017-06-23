#include <QsLog.h>
#include "dbus_fronius.h"
#include "defines.h"
#include "inverter_gateway.h"
#include "inverter_mediator.h"
#include "settings.h"
#include "ve_qitem_init_monitor.h"

DBusFronius::DBusFronius(QObject *parent) :
	QObject(parent),
	mSettings(new Settings(VeQItems::getRoot()->itemGetOrCreate("sub/com.victronenergy.settings/Settings/Fronius", false), this)),
	mGateway(new InverterGateway(mSettings, VeQItems::getRoot()->itemGetOrCreate("pub/com.victronenergy.fronius"), this))
{
	connect(mGateway, SIGNAL(inverterFound(DeviceInfo)), this, SLOT(onInverterFound(DeviceInfo)));
	VeQItemInitMonitor::monitor(mSettings->root(), this, SLOT(onSettingsInitialized()));
}

void DBusFronius::onSettingsInitialized()
{
	mGateway->startDetection();
}

void DBusFronius::onInverterFound(DeviceInfo deviceInfo)
{
	foreach (InverterMediator *m, mMediators) {
		if (m->processNewInverter(deviceInfo))
			return;
	}
	InverterMediator *m = new InverterMediator(deviceInfo, mGateway, mSettings, this);
	mMediators.append(m);
}
